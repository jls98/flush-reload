probe: src/probe.c
	echo "compiling probe.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/probe.c -o probe