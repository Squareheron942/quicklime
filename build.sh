MAKE=make

COMPILE_COMMANDS="$(pwd)/compile_commands.json"
[ -f "$COMPILE_COMMANDS" ] && rm "$COMPILE_COMMANDS"

$MAKE -j

VERBOSE=1 $MAKE --always-make --dry-run | grep -wE 'arm-none-eabi-g\+\+' | grep -v 'true add' | grep -w '\-c' | jq -nR '[inputs|{directory:"'$(pwd)'", command:., file:(match("(?<=[^-]-c )(?:[^ \\\\]|\\\\.)*").string), output: match("(?<=-o )(?:[^ \\\\]|\\\\.)*").string}]' >> "$COMPILE_COMMANDS"

#sed -z 's/\n\]\n\[/,/g' -i "$COMPILE_COMMANDS"