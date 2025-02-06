project: assembler.o preAssembler.o mainAssembler.o helpingFunctions.o
	gcc -ansi -Wall -pedantic assembler.c -o project
assembler.o: assembler.c preAssembler.c mainAssembler.c data.h
	gcc -ansi -Wall -pedantic assembler.c -o assembler.o
preAssembler.o: data.h
	gcc -c -ansi -Wall -pedantic preAssembler.c -o preAssembler.o
mainAssembler.o: helpingFunctions.c data.h
	gcc -c -ansi -Wall -pedantic mainAssembler.c -o mainAssembler.o
helpingFunctions.o: helpingFunctions.c data.h
	gcc -c -ansi -Wall -pedantic helpingFunctions.c -o helpingFunctions.o	
