hello:
	echo "Hello"

// probe
probe: src/probe.c
	cc probe.c -o probe