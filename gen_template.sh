#!/bin/sh

set -e

src="$1"
if [ -z "$src" ] || [ ! -f "$src" ]; then
    echo "gen_template.sh: missing source file" >&2
    exit 1
fi

printf '#ifndef CONFIG_TEMPLATE_H\n'
printf '#define CONFIG_TEMPLATE_H\n\n'
printf '#include <stddef.h>\n\n'
printf 'static const char CONFIG_TEMPLATE[] =\n'

awk '{
    gsub(/\\/, "\\\\");
    gsub(/"/, "\\\"");
    printf "    \"%s\\n\"\n", $0;
}' "$src"

printf ';\n\n'
printf 'static const size_t CONFIG_TEMPLATE_LEN = sizeof(CONFIG_TEMPLATE) - 1;\n\n'
printf '#endif\n'
