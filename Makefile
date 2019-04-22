# build an executable named main.exe from spaceSim.c with statically linked libraries

main: spaceSim.c
	mpicc -o main.exe spaceSim.c -lm

bluegene: spaceSim.c
	mpixlc -o main.exe spaceSim.c -lm 

clean: 
	$(RM) main.exe
