#!/usr/bin/env bash
# Quick test that the interface works as expected
# Not comprehensive as that is suppose to be done by test.c

input="-----------------------------9051914041544843365972754266\r\n"\
"Content-Disposition: form-data; name=\"text\"\r\n"\
"\r\n"\
"text default\r\n"\
"-----------------------------9051914041544843365972754266--\r\n"

expected_output="text default"

output=$(echo -e "$input" | ./multipart_extract)

if [[ "$output" == "$expected_output" ]]; then
  echo "multipart_extract test PASSED"
  exit 0
else
  echo "multipart_extract test FAILED"
  exit 1
fi
