#!/usr/bin/env bash

. "./lib/nolifebar/functions"

# ANSI color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Variables to track test results
all_tests_passed=true
test_count=0
failed_tests=0

# Function to mark a test as failed
mark_test_failed() {
    all_tests_passed=false
    ((failed_tests++))
}

# Function to increment the test count
increment_test_count() {
    ((test_count++))
}

# Test function
test_apply_dictionary_value_format() {
    increment_test_count

    # Declare target, source and format dictionaries
    declare -A target_kv=()
    declare -A source_kv=(
        ["apple"]="5"
        ["carrot"]="2"
        ["banana"]="7"
    )

    declare -A format_kv=(
        ["apple"]="Number of apples: %d"
        ["carrot"]="Number of carrots: %d"
    )

    # Apply the formatting
    apply_dictionary_value_format target_kv source_kv format_kv

    # Expected output
    local expected_apple="Number of apples: 5"
    local expected_carrot="Number of carrots: 2"
    local expected_banana="7"

    # Assertions
    if [[ "${target_kv["apple"]}" == "$expected_apple" ]] && \
       [[ "${target_kv["carrot"]}" == "$expected_carrot" ]] && \
       [[ "${target_kv["banana"]}" == "$expected_banana" ]]; then
        echo -e "${FUNCNAME[0]} - ${GREEN}Test passed.${NC}"
    else
        echo -e "${FUNCNAME[0]} - ${RED}Test failed.${NC}"
        echo "Expected: apple=\"$expected_apple\", carrot=\"$expected_carrot\", banana=\"$expected_banana\""
        echo "Got: apple=\"${target_kv["apple"]}\", carrot=\"${target_kv["carrot"]}\", banana=\"${target_kv["banana"]}\""
        mark_test_failed
    fi
}

test_update_threshold() {
    increment_test_count

    # Declare the result, data, and threshold dictionaries
    declare -A result_kv=()
    declare -A data_kv=(
        ["sensor1"]="51"
        ["sensor2"]="49"
        ["sensor3"]="anc"
        ["sensor4"]="10"
    )
    declare -A threshold_kv=(
        ["sensor1"]="50"
        ["sensor2"]="50"
        ["sensor3"]="75"
    )

    # Apply the update_threshold function
    update_threshold result_kv data_kv threshold_kv

    # Expected output
    local expected_sensor1="ON"
    local expected_sensor2="OFF"
    local expected_sensor3="OFF"

    # Assertions
    if [[ "${result_kv["sensor1"]}" == "$expected_sensor1" ]] && \
        [[ "${result_kv["sensor2"]}" == "$expected_sensor2" ]] && \
        [[ "${result_kv["sensor3"]}" == "$expected_sensor3" ]]; then
        echo -e "${FUNCNAME[0]} - ${GREEN}Test passed.${NC}"
    else
        echo -e "${FUNCNAME[0]} - ${RED}Test failed.${NC}"
        echo "Expected: sensor1=\"$expected_sensor1\", sensor2=\"$expected_sensor2\", sensor3=\"$expected_sensor3\""
        echo "Got: sensor1=\"${result_kv["sensor1"]}\", sensor2=\"${result_kv["sensor2"]}\", sensor3=\"${result_kv["sensor3"]}\""
        mark_test_failed
    fi
}

# Test function for is_array
test_is_array() {
    increment_test_count

    local -a array_var=(1 2 3)
    local string_var="not an array"

    if is_array "array_var"; then
        echo -e "is_array (array): ${GREEN}Passed${NC}"
    else
        echo -e "is_array (array): ${RED}Failed${NC}"
        mark_test_failed
    fi

    increment_test_count

    if ! is_array "string_var"; then
        echo -e "is_array (string): ${GREEN}Passed${NC}"
    else
        echo -e "is_array (string): ${RED}Failed${NC}"
        mark_test_failed
    fi
}

# Test function for is_array_not_empty
test_is_array_not_empty() {
    increment_test_count

    local -a non_empty_array=(1 2 3)
    local -a empty_array=()
    local string_var="not an array"

    # Test with a non-empty array
    if is_array_not_empty "non_empty_array"; then
        echo -e "is_array_not_empty (non-empty array): ${GREEN}Passed${NC}"
    else
        echo -e "is_array_not_empty (non-empty array): ${RED}Failed${NC}"
        mark_test_failed
    fi

    increment_test_count

    # Test with an empty array
    if ! is_array_not_empty "empty_array"; then
        echo -e "is_array_not_empty (empty array): ${GREEN}Passed${NC}"
    else
        echo -e "is_array_not_empty (empty array): ${RED}Failed${NC}"
        mark_test_failed
    fi

    increment_test_count

    # Test with a non-array variable
    if is_array_not_empty "string_var"; then
        echo -e "is_array_not_empty (string): ${GREEN}Passed${NC}"
    else
        echo -e "is_array_not_empty (string): ${RED}Failed${NC}"
        mark_test_failed
    fi
}

# Test function for create_log_scale
test_create_log_scale() {
    increment_test_count

    local -a scale_points
    create_log_scale scale_points 10 1000 5

    local expected_scale=(10 32 100 316 1000)
    if [[ "${scale_points[*]}" == "${expected_scale[*]}" ]]; then
        echo -e "create_log_scale: ${GREEN}Passed${NC}"
    else
        echo -e "create_log_scale: ${RED}Failed${NC}"
        echo "Expected: ${expected_scale[*]}"
        echo "Got: ${scale_points[*]}"
        mark_test_failed
    fi
}

# Test function for find_scale_point
test_find_scale_point() {
    increment_test_count

    local -a scale_points=(1 10 100 1000)
    local result

    find_scale_point result scale_points 15
    if [[ "$result" -eq 2 ]]; then
        echo -e "find_scale_point (15): ${GREEN}Passed${NC}"
    else
        echo -e "find_scale_point (15): ${RED}Failed${NC}"
        echo "Expected: 2, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    find_scale_point result scale_points 100
    if [[ "$result" -eq 3 ]]; then
        echo -e "find_scale_point (100): ${GREEN}Passed${NC}"
    else
        echo -e "find_scale_point (100): ${RED}Failed${NC}"
        echo "Expected: 3, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    find_scale_point result scale_points 1000
    if [[ "$result" -eq 4 ]]; then
        echo -e "find_scale_point (1000): ${GREEN}Passed${NC}"
    else
        echo -e "find_scale_point (1000): ${RED}Failed${NC}"
        echo "Expected: 3, Got: $result"
        mark_test_failed
    fi

    find_scale_point result scale_points 1001
    if [[ "$result" -eq 4 ]]; then
        echo -e "find_scale_point (1000): ${GREEN}Passed${NC}"
    else
        echo -e "find_scale_point (1000): ${RED}Failed${NC}"
        echo "Expected: 3, Got: $result"
        mark_test_failed
    fi
}

# Test function for ceil_division
test_ceil_division() {
    increment_test_count

    local result

    ceil_division result 5 2
    if [[ "$result" -eq 3 ]]; then
        echo -e "ceil_division (5 / 2): ${GREEN}Passed${NC}"
    else
        echo -e "ceil_division (5 / 2): ${RED}Failed${NC}"
        echo "Expected: 3, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    ceil_division result 4 2
    if [[ "$result" -eq 2 ]]; then
        echo -e "ceil_division (4 / 2): ${GREEN}Passed${NC}"
    else
        echo -e "ceil_division (4 / 2): ${RED}Failed${NC}"
        echo "Expected: 2, Got: $result"
        mark_test_failed
    fi
}

# Test function for bytes_to_gigabytes_rounded
test_bytes_to_gigabytes_rounded() {
    increment_test_count

    local result

    # Test 1: Standard input
    bytes_to_gigabytes_rounded result 1234567890
    if [[ "$result" == "1.1" ]]; then
        echo -e "bytes_to_gigabytes_rounded (1234567890) == 1.1: ${GREEN}Passed${NC}"
    else
        echo -e "bytes_to_gigabytes_rounded (1234567890): ${RED}Failed${NC}"
        echo "Expected: 1.1, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    # Test 2: Exact 1 GB
    bytes_to_gigabytes_rounded result $((1024 * 1024 * 1024))
    if [[ "$result" == "1.0" ]]; then
        echo -e "bytes_to_gigabytes_rounded (1024^3) == 1.0: ${GREEN}Passed${NC}"
    else
        echo -e "bytes_to_gigabytes_rounded (1024^3): ${RED}Failed${NC}"
        echo "Expected: 1.0, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    # Test 3: Less than 1 GB
    bytes_to_gigabytes_rounded result $((512 * 1024 * 1024))
    if [[ "$result" == "0.5" ]]; then
        echo -e "bytes_to_gigabytes_rounded (512MB) == 0.5: ${GREEN}Passed${NC}"
    else
        echo -e "bytes_to_gigabytes_rounded (512MB): ${RED}Failed${NC}"
        echo "Expected: 0.5, Got: $result"
        mark_test_failed
    fi

    increment_test_count

    # Test 4: Rounding test
    bytes_to_gigabytes_rounded result 1610612736  # ~1.5 GB
    if [[ "$result" == "1.5" ]]; then
        echo -e "bytes_to_gigabytes_rounded (1610612736) == 1.5: ${GREEN}Passed${NC}"
    else
        echo -e "bytes_to_gigabytes_rounded (1610612736): ${RED}Failed${NC}"
        echo "Expected: 1.5, Got: $result"
        mark_test_failed
    fi

    bytes_to_gigabytes_rounded result 0
    if [[ "$result" == "0" ]]; then
        echo -e "bytes_to_gigabytes_rounded (0) == 0: ${GREEN}Passed${NC}"
    else
        echo -e "bytes_to_gigabytes_rounded (0): ${RED}Failed${NC}"
        echo "Expected: 0, Got: $result"
        mark_test_failed
    fi
}

# Run the tests
test_apply_dictionary_value_format
test_update_threshold
test_is_array
test_is_array_not_empty
test_create_log_scale
test_find_scale_point
test_ceil_division
test_bytes_to_gigabytes_rounded

# Print summary and exit with code 1 if any test failed
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
