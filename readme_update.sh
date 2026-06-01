#!/usr/bin/env sh
# Update README.md size placeholders and version badge.
# Reads key=value size pairs from stdin (pipe from parser_size.sh),
# then updates the version from clib.json if jq is available.
#
# Usage:
#   ./parser_size.sh | ./readme_update.sh   # full update
#   ./readme_update.sh < sizes.txt          # from a saved report

set -e

while IFS='=' read -r key value; do
    case $key in
        text)  sed -i "s|<dotTextSize>.*</dotTextSize>|<dotTextSize>${value}</dotTextSize>|" README.md ;;
        data)  sed -i "s|<dotDataSize>.*</dotDataSize>|<dotDataSize>${value}</dotDataSize>|" README.md ;;
        bss)   sed -i "s|<dotBSSSize>.*</dotBSSSize>|<dotBSSSize>${value}</dotBSSSize>|" README.md ;;
        flash) sed -i "s|<flashSizeUsage>.*</flashSizeUsage>|<flashSizeUsage>${value}</flashSizeUsage>|" README.md ;;
    esac
done

if command -v jq > /dev/null 2>&1 && [ -f clib.json ]; then
    version=$(jq -r '.version' clib.json)
    sed -i "s|<version>.*</version>|<version>${version}</version>|" README.md
    sed -i "s|<versionBadge>.*</versionBadge>|<versionBadge>![Version ${version}](https://img.shields.io/badge/version-${version}-blue.svg)</versionBadge>|" README.md
fi

echo "README.md updated"
