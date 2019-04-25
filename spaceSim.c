#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>
#include <time.h>
#include <stddef.h>

//Space Simulator
static inline int root(int input, int n)
{
  return round(pow(input, 1.0/n));
}

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

//Outputs a given array of Bodies. Resulting file contains only the x,y,z-coordinates of each Body
void output_Bodies(Body *final_bodies, int num){
	FILE* output = fopen("universe_contents.csv", "w");
	for(int i=0;i<num;i++){
		fprintf(output,"%d,%d,%d,%d\n",final_bodies[i].mass,final_bodies[i].posx,final_bodies[i].posy,final_bodies[i].posz);
	}
  fclose(output);
}

//MPI variables
int mpi_myrank;
int mpi_commsize;

//array of bodies for each rank
Body* totalBodies;
Body* bodies;
int bodies_index = 0;
int totalBodyNum = 0;

//universe bounds x, y, z
int boundx, boundy, boundz;
//tick time length
int ticks;
int tickTimeStep = 1; // day
int minbodies = 2;
int maxbodies = 3;
int maxMass = 1000;
int minMass = 10;
long int universeSize = 10000; //universe is currently a cube
long int rankSize = 0;
int rankMass = 0;
int* otherRankMasses;
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
	rankSize = universeSize/root(mpi_commsize, 3);

	otherRankMasses = calloc(27, sizeof(int));

	/* create a type for struct body */
    const int nitems = 10;
    int blocklengths[10] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    MPI_Datatype types[10] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Datatype MPI_BODY;
    MPI_Aint offsets[10];

    //Defining variables within the struct in MPI data type
    offsets[0] = offsetof(Body, ID);
    offsets[1] = offsetof(Body, type);
    offsets[2] = offsetof(Body, mass);
    offsets[3] = offsetof(Body, radius);
    offsets[4] = offsetof(Body, posx);
    offsets[5] = offsetof(Body, posy);
    offsets[6] = offsetof(Body, posz);
    offsets[7] = offsetof(Body, vx);
    offsets[8] = offsetof(Body, vy);
    offsets[9] = offsetof(Body, vz);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &MPI_BODY);
    MPI_Type_commit(&MPI_BODY);

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
		int randPosX = rand()%rankSize + mpi_myrank%root(mpi_commsize, 3)*rankSize;
		int randPosY = rand()%rankSize + mpi_myrank/root(mpi_commsize, 3)%root(mpi_commsize, 3)*rankSize;
		int randPosZ = rand()%rankSize + mpi_myrank/(root(mpi_commsize, 3)*root(mpi_commsize, 3))*mpi_myrank*rankSize;
		//randomly generate velocity
		int randVelX = rand()%maxAbsVelocity;
		int randVelY = rand()%maxAbsVelocity;
		int randVelZ = rand()%maxAbsVelocity;
		init_body(&bodies[i], randMass, 10, randPosX, randPosY, randPosZ, randVelX, randVelY, randVelZ); //example of init_body
		#ifdef DEBUG
			printf("ID: %d, Type: %s, Mass: %d\n", ((&bodies[i])->ID), types[((&bodies[i])->type)], ((&bodies[i])->mass));
			printf("Position: (%d, %d, %d)\n", ((&bodies[i])->posx),((&bodies[i])->posy),((&bodies[i])->posz));
			printf("Velocity: (%d, %d, %d)\n", ((&bodies[i])->vx),((&bodies[i])->vy),((&bodies[i])->vz));
		#endif
	}

	//mass of added ranks used between ranks for force calculations
	for(int j = 0; j < num_bodies; j++) {
		rankMass = rankMass + bodies[j].mass;
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

		//add forces of this rank together (for now, assumption is that mass is generally centered in the rank)
		//for now, the force of the rank will be calculated using the added mass
		//mpi translate rankmass
		//commented for debugging purposes
		/*
		int offset = mpi_myrank - 1 - ranksPerRow - ranksPerRow*ranksPerRow;
		MPI_Request request;
		MPI_Status status;
		for(int j = 0; j < 3; j++) { 	//j == x, +1 -1
			for(int k = 0; k < 3; k++) {	//k == y, +-ranksPerRow
				for(int l = 0; l < 3; l++) {  //l == z +-/ranksPerRow*ranksPerRow
					//get mass from ranks
					if(!((j == 1 && k == 1) && l == 1)) {
						//row to be recieved and sent to
						MPI_Isend(&rankMass, 1, MPI_INT, offset + j + k*ranksPerRow + l*ranksPerRow*ranksPerRow, mpi_myrank, MPI_COMM_WORLD, &request);
						MPI_Recv(&otherRankMasses[27 - (j + k*ranksPerRow + l*ranksPerRow*ranksPerRow)], int count, MPI_INT, MPI_ANY_SOURCE,
							MPI_ANY_TAG, MPI_COMM_WORLD, status);
					
					}

				}
			}
		}
		*/
		for(int j=0;j<num_bodies;j++){
			xForce = 0;
			yForce = 0;
			zForce = 0;
			for(int k=0;k<num_bodies;k++){
				if(k!=j){
					float distance = sqrt( pow(bodies[j].posx-bodies[k].posx, 2) + pow(bodies[j].posy-bodies[k].posy, 2) + pow(bodies[j].posz-bodies[k].posz, 2));
					//Sum up the x,y,z forces on the current object, and apply them in the positive or negative direction as appropriate

						xForce += gravity * bodies[j].mass * bodies[k].mass / pow(distance,3) * (bodies[k].posx - bodies[j].posx);
						yForce += gravity * bodies[j].mass * bodies[k].mass / pow(distance,3) * (bodies[k].posy - bodies[j].posy);
						zForce += gravity * bodies[j].mass * bodies[k].mass / pow(distance,3) * (bodies[k].posz - bodies[j].posz);
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

			//Add forces of this rank together for other ranks

			//Array of the number of objects to be sent to each other rank
			//Ranks ordered bottom to top, left to right, back to forward
			//int to_other_ranks[mpi_commsize] = 0;

			// Check rank changes at boundaries based on position



		}

		// TODO: Accept any new bodies that passed the rank boundary (check out MPI_Type_create_struct)

		// Detect and resolve collisions (type == -1 means the body was destroyed)
		for(int i=0;i<num_bodies;i++){
			if(bodies[i].type == -1){
				continue;
			}
			for(int j=i+1;j<num_bodies;j++){
				int hit_distance = bodies[i].radius + bodies[j].radius;
				if(bodies[j].type == -1){
					continue;
				}
				//Check if the bodies have collided, i.e. if the difference in their positions is less than their combined radius
				if((abs(bodies[i].posx-bodies[j].posx)+abs(bodies[i].posy-bodies[j].posy)+abs(bodies[i].posz-bodies[j].posz))<=hit_distance){
					//If they have collided and one is much larger than the other, the smaller one is absorbed (mass added to the larger one_)
					if(bodies[i].mass>bodies[j].mass*10){
						bodies[i].mass += bodies[j].mass;
						bodies[j].type = -1;
						//continue;
					}else if(bodies[i].mass*10<bodies[j].mass){
						bodies[j].mass += bodies[i].mass;
						bodies[i].type = -1;
						//continue;
					}else{
						//If their masses are similar, both are destroyed
						bodies[i].type = -1;
						bodies[j].type = -1;
					}
				}
			}
		}


	}

	// CLean up allocated memory
	totalBodyNum = num_bodies * mpi_commsize;
	totalBodies = calloc(totalBodyNum, sizeof(Body));

	MPI_Gather(bodies, num_bodies, MPI_BODY, totalBodies, num_bodies, MPI_BODY, 0, MPI_COMM_WORLD);
	if(mpi_myrank==0){
			output_Bodies(totalBodies, totalBodyNum);
	}
	MPI_Finalize();
	return EXIT_SUCCESS;

}


/* Experiments(methods/parameters/explanations):


*/
