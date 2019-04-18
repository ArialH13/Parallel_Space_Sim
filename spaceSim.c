#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

//Space Simulator

/* Body Class Structure */

int IDnum = 0;

typedef struct {
	int ID;
	char* type;
	int mass;
	int radius;
	int posx, posy, posz;
	int vx, vy, vz;
} Body;

//Function Declarations
void init_body(Body *b, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz);
void create_ID(Body *b);

void init_body(Body *b, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz) {
	char* type = NULL;
	if(mass<100){
		type = "Asteroid";
	}else if(mass<500){
		type = "Planet";
	}else{
		type = "Star";
	}
	b-> type = type;
	create_ID(b);
	b-> mass = mass;
	b-> radius = radius;
	b-> posx = posx;
	b-> posy = posy;
	b-> posz = posz;
	b-> vx = vx;
	b-> vy = vy;
	b-> vz = vz;
}

void create_ID(Body *b) {
	b-> ID = IDnum;
	IDnum++;
}

//MPI variables
int mpi_myrank;
int mpi_commsize;

//array of bodies for each rank
Body* bodies;
int bodies_index = 0;

//universe bounds x, y, z
int boundx, boundy, boundz;
//tick time length
int ticks;
int tickTimeStep = 1;
int minBodies = 10;
int maxBodies = 15;
int maxMass = 1000;
int minMass = 10;
int universeSize = 10000; //universe is currently a cube
int maxAbsVelocity = 100;
//Mathematical constants
float gravity = .0000000000667408;
int hubble = 500;	//Units km/s/Mpc



int main(int argc, char** argv) {
	//declare ranks
	MPI_Init( &argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
	MPI_Comm_rank( MPI_COMM_WORLD, &my_mpi_rank);

	Bodies = calloc(3,sizeof(Body));

	//how are bodies distributed randomly?
	srand(time(NULL));   // Initialization, should only be called once.
	int r = rand()%(maxBodies-minBodies)+minBodies;      // Returns a pseudo-random integer between minBodies and maxBodies
	bodies = calloc(r, sizeof(Body));
		//randomly generate overall number of objects
	for(int i = 0; i < r; i++) {
		//randomly generate variables of bodies, with control over the bounds of velocity, universe size, and mass
		//randomly generate size of objects
		int randMass = rand()%(maxMass-minMass)+minMass;
		//randomly generate position
		int randPosX = rand()%universeSize;
		int randPosY = rand()%universeSize;
		int randPosZ = rand()%universeSize;
		//randomly generate velocity
		int randVelX = rand()%maxAbsVelocity;
		int randVelY = rand()%maxAbsVelocity;
		int randVelZ = rand()%maxAbsVelocity;
		init_body(&bodies[i], randMass, 10, randPosX, randPosY, randPosZ, randVelX, randVelY, randVelZ); //example of init_body
		printf("ID: %d, Type: %s, Mass: %d\n", ((&bodies[i])->ID), ((&bodies[i])->type), ((&bodies[i])->mass));
		printf("Position: X-%d Y-%d Z-%d\n", ((&bodies[i])->posx),((&bodies[i])->posy),((&bodies[i])->posz));
		printf("Velocity: X-%d Y-%d Z-%d\n", ((&bodies[i])->vx),((&bodies[i])->vy),((&bodies[i])->vz));
	}
}

//tick for loop
	//calculate
	//for planets
		//for planets
			//add acceleration effect


}
