#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

//Space Simulator
int IDnum = 0;

//declaration of body type
typedef struct {
	int ID;
	char* type;
	int mass;
	int radius;
	int posx, posy, posz;
	int vx, vy, vz;
} Body;
//Function Declarations
void init_body(Body *b, char* type, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz);
void create_ID(Body *b);

void init_body(Body *b, char* type, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz) {
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

//Mathematical constants
float gravity = .0000000000667408;
int hubble = 500;	//Units km/s/Mpc





int main(int argc, char** argv) {
	//declare ranks
	MPI_Init( &argc, &argv);
	MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
	MPI_Comm_rank( MPI_COMM_WORLD, &my_mpi_rank);
	upper = argv[1] / mpi_commsize;

	Bodies = calloc(3,sizeof(Body));

	//how are bodies distributed randomly?
	//Generate random ints in the range of the universe bounds as x,y,z coordinates
	Body star;
	Body planet;
	Body stroid;
	init_body(&star, "Star", 100, 10, (rand() % (upper + 1)), (rand() % (upper + 1)) + lower, (rand() % (upper + 1)) + lower, 1, 0, 0);
	init_body(&star, "Planet", 10, 10, (rand() % (upper + 1)), (rand() % (upper + 1)) + lower, (rand() % (upper + 1)) + lower, 1, 0, 0);
	init_body(&stroid, "Asteroid", 1, 10, (rand() % (upper + 1)), (rand() % (upper + 1)) + lower, (rand() % (upper + 1)) + lower, 1, 0, 0);
	Bodies[bodies_index] = star;
	bodies_index++;
	Bodies[bodies_index] = planet;
	bodies_index++;
	Bodies[bodies_index] = stroid;
	bodies_index++;

	printf("ID: %d, Type: %s, Mass: %d\n", ((&star)->ID), ((&star)->type), ((&star)->mass));
	printf("ID: %d, Type: %s, Mass: %d\n", ((&stroid)->ID), ((&stroid)->type), ((&stroid)->mass));
		//declare number, location, and velocity of objects

	//for each tick
		//calculate


	//After calculating, pass objects between ranks

}
