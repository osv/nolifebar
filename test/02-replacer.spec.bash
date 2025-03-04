#!/usr/bin/env bash

# set -e

REPLACER_BIN=./bin/nolifebar-replacer

# Define color codes for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Global counters
test_count=0
failed_tests=0
all_tests_passed=true

mark_test_failed() {
    all_tests_passed=false
    ((failed_tests++))
}

increment_test_count() {
    ((test_count++))
}

# Test 1: Basic replacement test
test_basic_replacement() {
    rep_file=$(mktemp)
    echo "apple: orange" > "$rep_file"

    result=$(echo "apple" | "$REPLACER_BIN" "$rep_file")
    expected="orange"
    if [ "$result" == "$expected" ]; then
        echo -e "test_basic_replacement: ${GREEN}Passed${NC}"
    else
        echo -e "test_basic_replacement: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Test 2: Multiple replacement rules
test_multiple_replacements() {
    rep_file=$(mktemp)
    cat > "$rep_file" <<EOF
cat: dog
bird: eagle
EOF

    result=$(echo "cat bird" | "$REPLACER_BIN" "$rep_file")
    expected="dog eagle"
    if [ "$result" == "$expected" ]; then
        echo -e "test_multiple_replacements: ${GREEN}Passed${NC}"
    else
        echo -e "test_multiple_replacements: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Test 3: Duplicate substitution rules (edge case)
# The replacements file declares "hello" twice.
# The program processes replacements in order, so the first rule should apply.
test_duplicate_replacement() {
    rep_file=$(mktemp)
    cat > "$rep_file" <<EOF
hello: hi
hello: hey
EOF

    result=$(echo "say hello to hello" | "$REPLACER_BIN" "$rep_file")
    expected="say hi to hi"
    if [ "$result" == "$expected" ]; then
        echo -e "test_duplicate_replacement: ${GREEN}Passed${NC}"
    else
        echo -e "test_duplicate_replacement: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Test 4: When no rule matches, output remains unchanged.
test_no_replacement() {
    rep_file=$(mktemp)
    echo "foo: bar" > "$rep_file"

    result=$(echo "hello" | "$REPLACER_BIN" "$rep_file")
    expected="hello"
    if [ "$result" == "$expected" ]; then
        echo -e "test_no_replacement: ${GREEN}Passed${NC}"
    else
        echo -e "test_no_replacement: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Test 5: Replacement that requires shifting buffer content.
test_shift_replacement() {
    rep_file=$(mktemp)
    echo "<long>: verylong" > "$rep_file"

    result=$(echo "this is a <long> text" | "$REPLACER_BIN" "$rep_file")
    expected="this is a verylong text"
    if [ "$result" == "$expected" ]; then
        echo -e "test_shift_replacement: ${GREEN}Passed${NC}"
    else
        echo -e "test_shift_replacement: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Test 6: Simulate inotify file change (reload replacements) using coproc with output redirection
test_reload_replacements() {
    tmp_rep=$(mktemp)
    tmp_out=$(mktemp)

    set_replace_text() {
        echo "$1" > "$tmp_rep"
    }
    # Initial replacement rule: "test" -> "initial"
    set_replace_text "test: initial"

    # Start the coprocess. Its stdin remains available via REPLACER_PROC[1].
    # Redirect the coprocess's stdout and stderr to tmp_out.
    coproc REPLACER_PROC { "$REPLACER_BIN" "$tmp_rep"; } > "$tmp_out" 2>&1

    # Allow time for the process to start.
    sleep 0.5

    # Send first input. It should process "test" using the initial rule.
    echo "test" >&"${REPLACER_PROC[1]}"
    sleep 0.1  # Wait for the input to be processed

    # Modify the replacement file to update the rule to "test" -> "updated"
    set_replace_text "test: updated"
    sleep 0.1  # Allow time for inotify to trigger and reprocess the last line

    # Close the coprocess input to signal EOF so the program exits gracefully.
    exec {REPLACER_PROC[1]}>&-

    # Wait for the coprocess to finish.
    wait "${REPLACER_PROC_PID}" 2>/dev/null

    # Capture the output from the temporary file.
    result=$(grep -v '^$' "$tmp_out")

    # Expected output:
    #   First, the input "test" is processed with the initial rule: "initial"
    #   Then, after file modification, the last line is reprocessed to "updated"
    expected="initial
updated"
    if [ "$result" == "$expected" ]; then
        echo -e "test_reload_replacements: ${GREEN}Passed${NC}"
    else
        echo -e "test_reload_replacements: ${RED}Failed${NC}"
        echo "Expected:"
        echo "$expected"
        echo "Got:"
        echo "$result"
        mark_test_failed
    fi

    increment_test_count
    rm "$tmp_rep" "$tmp_out"
}

# Test 6: Multiple nested replacement rules
test_multiple_nested_replacements() {
    rep_file=$(mktemp)
    cat > "$rep_file" <<EOF
<cat>      : dog
<bird>     : eagle
<predator> : <bird>
<creature> : <predator>
EOF
    result=$(echo "cat, <predator> <bird> <creature>" | "$REPLACER_BIN" "$rep_file")
    expected="cat, eagle eagle eagle"
    if [ "$result" == "$expected" ]; then
        echo -e "test_multiple_replacements: ${GREEN}Passed${NC}"
    else
        echo -e "test_multiple_replacements: ${RED}Failed${NC}"
        echo "Expected: '$expected', Got: '$result'"
        mark_test_failed
    fi
    increment_test_count
    rm "$rep_file"
}

# Run all tests
test_basic_replacement
test_multiple_replacements
test_duplicate_replacement
test_no_replacement
test_shift_replacement
test_reload_replacements
test_multiple_nested_replacements

echo
if [ "$all_tests_passed" = false ]; then
    echo -e "${RED}One or more tests failed.${NC}"
    echo -e "${RED}Failed: $failed_tests/${test_count} tests.${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed.${NC}"
    echo -e "${GREEN}Passed: $test_count/$test_count tests.${NC}"
    exit 0
fi
