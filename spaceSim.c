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
	int type;
	int mass;
	int radius;
	int posx, posy, posz;
	int vx, vy, vz;
} Body;

/* Type mapping (used to simplify our struct for easier MPI calls)
0 = Asteroid
1 = Planet
2 = Star
*/
char *types[3] = {"Asteroid", "Planet", "Star"};

//Function Declarations
void init_body(Body *b, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz);
void create_ID(Body *b);

void init_body(Body *b, int mass, int radius, int posx, int posy, int posz, int vx, int vy, int vz) {
	int type = -1;
	if(mass<100){
		type = 0;
	}else if(mass<500){
		type = 1;
	}else{
		type = 2;
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
int tickTimeStep = 1; // day
int minbodies = 10;
int maxbodies = 15;
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
	MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);

	int ticks = atoi(argv[1]);
	bodies = calloc(3,sizeof(Body));


	//how are bodies distributed randomly?
	srand(time(NULL));   // Initialization, should only be called once.
	int num_bodies = rand()%(maxbodies-minbodies)+minbodies;      // Returns a pseudo-random integer between minbodies and maxbodies
	bodies = calloc(num_bodies, sizeof(Body));
		//randomly generate overall number of objects
	for(int i = 0; i < num_bodies; i++) {
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
		printf("ID: %d, Type: %s, Mass: %d\n", ((&bodies[i])->ID), types[((&bodies[i])->type)], ((&bodies[i])->mass));
		printf("Position: (%d, %d, %d)\n", ((&bodies[i])->posx),((&bodies[i])->posy),((&bodies[i])->posz));
		printf("Velocity: (%d, %d, %d)\n", ((&bodies[i])->vx),((&bodies[i])->vy),((&bodies[i])->vz));
	}

	float xForce = 0;
	float yForce = 0;
	float zForce = 0;
	//Perform the calculations for gravitational forces between bodies for as many ticks as specified by the user
	/*
	Before first tick:
	Generate bodies and randomize based upon the rank boundaries, not universe boundaries
	Sort each rank's population (bodies)
	*/
	for(int i=0;i<ticks;i++){
		/*
		In this loop we update:
		Sum of forces
		Velocity
		Position
		*/
		for(int j=0;j<num_bodies;j++){
			xForce = 0;
			yForce = 0;
			zForce = 0;
			for(int k=0;k<num_bodies;k++){
				if(k!=j){
					float radius = ;
					//Sum up the x,y,z forces on the current object, and apply them in the positive or negative direction as appropriate
					if(bodies[j].posx<bodies[k].posx){
						xForce += gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posx - bodies[k].posx,2);
					}else{
						xForce -= gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posx - bodies[k].posx,2);
					}
					if(bodies[j].posy<bodies[k].posy){
						yForce += gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posy - bodies[k].posy,2);
					}else{
						yForce -= gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posy - bodies[k].posy,2);
					}
					if(bodies[j].posz<bodies[k].posz){
						zForce += gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posz - bodies[k].posz,2);
					}else{
						zForce -= gravity * bodies[j].mass * bodies[k].mass / pow(bodies[j].posz - bodies[k].posz,2);
					}
				}
			}

			// Use the calculated forces to update the velocity of each body
			bodies[j].vx += (xForce / bodies[j].mass) * tickTimeStep;
			bodies[j].vy += (yForce / bodies[j].mass) * tickTimeStep;
			bodies[j].vz += (zForce / bodies[j].mass) * tickTimeStep;

			// Update position
			bodies[j].posx += bodies[j].vx * tickTimeStep;
			bodies[j].posy += bodies[j].vy * tickTimeStep;
			bodies[j].posz += bodies[j].vz * tickTimeStep;

			//Array of the number of objects to be sent to each other rank
			//Ranks ordered bottom to top, left to right, back to forward
			int to_other_ranks[mpi_commsize] = 0;

			// Check rank changes at boundaries based on position



		}

		// TODO: Accept any new bodies that passed the rank boundary (check out MPI_Type_create_struct)

		//Detect and resolve collisions
		for(int i=0;i<num_bodies;i++){
			if(bodies[i]==NULL){
				continue;
			}
			for(int j=i+1;j<num_bodies;j++){
				int hit_distance = bodies[i].radius + bodies[j].radius;
				if(bodies[j]==NULL){
					continue;
				}
				//Check if the bodies have collided, i.e. if the difference in their positions is less than their combined radius
				if((abs(bodies[i].posx-bodies[j].posx)+abs(bodies[i].posy-bodies[j].posy)+abs(bodies[i].posz-bodies[j].posz))<=hit_distance){
					//If they have collided and one is much larger than the other, the smaller one is absorbed (mass added to the larger one_)
					if(bodies[i].mass>bodies[j].mass*10){
						bodies[i].mass += bodies[j].mass;
						bodies[j] = NULL;
						continue;
					}else if(bodies[i].mass*10<bodies[j].mass){
						bodies[j].mass += bodies[i].mass;
						bodies[i] = NULL;
						continue;
					}else{
						//If their masses are similar, both are destroyed
						bodies[i] = NULL;
						bodies[j] = NULL;
					}
				}
			}
		}


	}

}

//tick for loop

	//calculate
	//for planets
		//for planets
			//add acceleration effect

/* Experiments(methods/parameters/explanations):



*/
