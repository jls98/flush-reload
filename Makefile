hello:
	echo "Hello"

probe: src/probe.c
	cc probe.c -o probe