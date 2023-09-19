probe: src/probe.c victim
	echo "compiling probe.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/probe.c -o probe
	
victim: src/victim.c
	echo "compiling victim.c"
	cc -Wall -Wextra -fdiagnostics-color=always -g src/victim.c -o victim

.PHONY: clean
clean:
	rm -f probe victim 
