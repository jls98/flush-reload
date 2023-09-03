probe: src/probe.c victim
	echo "compiling probe.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/probe.c -o probe
	
testexec: src/testexec.c 
	echo "compiling testexec.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/testexec.c -o testexec

victim: src/victim.c testexec
	echo "compiling victim.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/victim.c -o victim

.PHONY: clean
clean:
	rm -f probe testexec
