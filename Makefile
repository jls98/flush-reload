hello:
	echo "Hello"

probe: src/probe.c
	cc src/probe.c -o probe