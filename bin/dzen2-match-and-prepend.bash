#!/usr/bin/env bash

print_help() {
    echo "Usage: $0 COMMANDS..."
    echo
    echo "COMMANDS should be in the format MATCHTEXT=DELAY:PREFIX[,DELAY:PREFIX]..."
    echo "For example:"
    echo "  echo HELLO | ./bin/dzen2-match-and-prepend.bash \"HELLO=0.2:^bg(red)^fg(black), 0.2:^bg(black)^fg(red), 0.2:^bg(red)^fg(black),0.2:^bg(black)^fg(red)\" | dzen2 -p"
    echo
    echo "  echo HELLO | $0 \"HELLO=0.3:foo, 0.5:bar, 0.3:baz\" \"WORLD=0.2:one, 0.2:two\""
    echo
    echo "This script reads lines from stdin. If a line matches MATCHTEXT,"
    echo "it prints the corresponding PREFIX followed by the line after the specified DELAY."
    echo "If a new line is read from stdin, the current processing is cancelled and the new line is processed."
}

# Check if no arguments are provided and print help
if [ $# -eq 0 ]; then
    print_help
    exit 1
fi

# Parse commands from arguments
declare -A commands
for arg in "$@"; do
    match_text=$(echo "$arg" | cut -d'=' -f1)
    pairs=$(echo "$arg" | cut -d'=' -f2)
    commands["$match_text"]="$pairs"
done

# Function to process input line
process_input() {
    local line="$1"
    local pairs=${commands["$line"]}

    if [[ -n "$pairs" ]]; then
        IFS=',' read -ra pair_array <<< "$pairs"
        for pair in "${pair_array[@]}"; do
            delay=$(echo "$pair" | cut -d':' -f1 | tr -d ' ')
            prefix=$(echo "$pair" | cut -d':' -f2)
            echo "${prefix}${line}"
            sleep "$delay"
        done
    else
        echo "$line"
    fi
}

# Main loop to read lines from stdin and handle background processing
while IFS= read -r line || [ -n "$line" ]; do
    # Kill the previous background process if it exists
    if [[ -n "$bg_pid" ]]; then
        kill "$bg_pid" 2>/dev/null
        wait "$bg_pid" 2>/dev/null
    fi

    # Process the input line in the background
    process_input "$line" &
    bg_pid=$!
done

# Wait for the last background process to finish
wait "$bg_pid"
