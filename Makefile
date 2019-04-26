# build an executable named main.exe from spaceSim.c with statically linked libraries

main: spaceSim.c
	mpicc -o main.exe spaceSim.c -lm

debug: spaceSim.c
	mpicc -o main.exe spaceSim.c -lm -D DEBUG

msg: spaceSim.c
	mpicc -o main.exe spaceSim.c -lm -D DEBUG_MSG

bluegene: spaceSim.c
	mpixlc -o main.exe spaceSim.c -lm 

clean: 
	$(RM) main.exe