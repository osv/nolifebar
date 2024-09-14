#include <pulse/pulseaudio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define ERROR_PREFIX "[nolifebar-pa-status] "
#define LOG_ERROR(fmt, ...) fprintf(stderr, ERROR_PREFIX fmt, ##__VA_ARGS__)

static pa_mainloop *pa_ml;
static pa_mainloop_api *pa_mlapi;
static pa_context *pa_ctx;
static char default_sink_name[256] = {0};  // Buffer to store the default sink name
static char *sink_name_to_monitor = NULL;  // Sink name from command line

void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
    if (eol > 0) {
        return;
    }

    // If monitoring a specific sink name and it's not the current one, skip
    if (sink_name_to_monitor
        && strcmp(sink_name_to_monitor, "default") != 0
        && strcmp(i->name, sink_name_to_monitor) != 0) {
        return;
    }

    // If monitoring the default sink and this sink is not the current default, skip
    if (sink_name_to_monitor
        && strcmp(sink_name_to_monitor, "default") == 0
        && strcmp(i->name, default_sink_name) != 0) {
        return;
    }

    int volume_percent = round((i->volume.values[0] * 100.0) / PA_VOLUME_NORM);
    printf("Volume: %d Muted: %s Sink: %s\n",
           volume_percent,
           i->mute ? "yes" : "no",
           i->name);
    fflush(stdout);
}

void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata) {
    // If monitoring the default sink, check if the default sink has changed
    if (sink_name_to_monitor && strcmp(sink_name_to_monitor, "default") == 0) {
        if (strcmp(i->default_sink_name, default_sink_name) != 0) {
            strncpy(default_sink_name, i->default_sink_name, sizeof(default_sink_name) - 1);
            LOG_ERROR("Default sink changed to: %s\n", default_sink_name);

            // Get the initial status of the new default sink
            pa_operation *o = pa_context_get_sink_info_by_name(c, default_sink_name, sink_info_callback, NULL);
            if (o) {
                pa_operation_unref(o);
            }
        }
    }
}

void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
    if ((type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) == PA_SUBSCRIPTION_EVENT_SERVER) {
        LOG_ERROR("Server event received, checking for default sink change...\n");
        pa_operation *o = pa_context_get_server_info(c, server_info_callback, NULL);
        if (o) {
            pa_operation_unref(o);
        }
    } else if ((type & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_CHANGE) {
        // Handle sink changes
        pa_operation *o = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, NULL);
        if (o) {
            pa_operation_unref(o);
        }
    }
}

void context_state_callback(pa_context *c, void *userdata) {
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_READY:
            LOG_ERROR("Connection to PulseAudio established.\n");
            // If monitoring the default sink, get the current server info (including default sink)
            if (sink_name_to_monitor && strcmp(sink_name_to_monitor, "default") == 0) {
                pa_operation *o = pa_context_get_server_info(c, server_info_callback, NULL);
                if (o) {
                    pa_operation_unref(o);
                }
            } else if (sink_name_to_monitor) {
                // If monitoring a specific sink name, fetch its info
                pa_operation *o = pa_context_get_sink_info_by_name(c, sink_name_to_monitor, sink_info_callback, NULL);
                if (o) {
                    pa_operation_unref(o);
                }
            } else {
                // If monitoring all sinks, get information about all sinks
                pa_operation *o = pa_context_get_sink_info_list(c, sink_info_callback, NULL);
                if (o) {
                    pa_operation_unref(o);
                }
            }
            // Subscribe to both sink and server changes (for default sink changes)
            pa_context_set_subscribe_callback(c, subscribe_callback, NULL);
            pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SERVER, NULL, NULL);
            break;
        case PA_CONTEXT_FAILED:
        case PA_CONTEXT_TERMINATED:
            pa_mainloop_quit(pa_ml, 1);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    // Check if a sink name is provided as a command-line argument
    if (argc > 1) {
        sink_name_to_monitor = argv[1];
        LOG_ERROR("Monitoring sink: %s\n", sink_name_to_monitor);
    } else {
        LOG_ERROR("Monitoring all sinks.\n");
    }

    pa_ml = pa_mainloop_new();
    pa_mlapi = pa_mainloop_get_api(pa_ml);
    pa_ctx = pa_context_new(pa_mlapi, "NolifeBar sink monitor");

    // Set the context state callback
    pa_context_set_state_callback(pa_ctx, context_state_callback, NULL);

    // Connect to the PulseAudio server
    if (pa_context_connect(pa_ctx, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0) {
        LOG_ERROR("Failed to connect to PulseAudio server.\n");
        return 1;
    }

    // Run the main loop
    if (pa_mainloop_run(pa_ml, NULL) < 0) {
        LOG_ERROR("Failed to run PulseAudio mainloop.\n");
        return 1;
    }

    // Cleanup
    pa_context_disconnect(pa_ctx);
    pa_context_unref(pa_ctx);
    pa_mainloop_free(pa_ml);

    return 0;
}
