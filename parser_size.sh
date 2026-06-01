#!/usr/bin/env sh
# Compile the parser with embedded flags and print its section sizes.
# Output: key=value lines on stdout, suitable for piping into readme_update.sh
#
# Usage:
#   ./parser_size.sh                        # print sizes
#   ./parser_size.sh | ./readme_update.sh   # update README.md

set -e

CC="${CC:-cc}"
CFLAGS="${CFLAGS} -std=c99 -Wall -pedantic"

tmpobj=$(mktemp /tmp/minimal_multipart_parser_XXXXXX.o)
trap 'rm -f "$tmpobj"' EXIT

$CC $CFLAGS -c -g0 -Os minimal_multipart_parser.c -o "$tmpobj"

size "$tmpobj" | awk 'NR==2 {
    text=$1; data=$2; bss=$3
    printf "text=%d\ndata=%d\nbss=%d\nflash=%d\n", text, data, bss, text+data
}'
