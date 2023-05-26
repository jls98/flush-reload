probe: src/probe.c hello
	echo "probe"
	cc src/probe.c -o probe
	
hello:
	echo "Hello"