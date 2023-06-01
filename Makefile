probe: src/probe.c
	echo "compiling probe.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/probe.c -o probe
	
testexec: src/testexec.c 
	echo "compiling testexec.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/testexec.c -o testexec
