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

int max(int a, int b) {
	return (a > b ? a : b);
}

int min(int a, int b) {
	return (a < b ? a : b);
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
int minbodies = 1;
int maxbodies = 2;
int maxMass = 1000;
int minMass = 10;
long int universeSize = 10000; //universe is currently a ranksPerRow
long int rankSize = 0;
int rankMass = 0;
int* otherRankMasses;
int maxAbsVelocity = 100;
int ranksPerRow = 0;
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
	ranksPerRow = root(mpi_commsize, 3);
	//printf("ranksPerRow: %d\n", ranksPerRow);


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
	srand(time(NULL)*(mpi_myrank+1));   // Initialization, should only be called once.

	int num_bodies = rand()%(maxbodies-minbodies)+minbodies;      // Returns a pseudo-random integer between minbodies and maxbodies
	bodies = calloc(num_bodies, sizeof(Body));
		//randomly generate overall number of objects
	for(int i = 0; i < num_bodies; i++) {
		//randomly generate variables of bodies, with control over the bounds of velocity, universe size, and mass
		//randomly generate size of objects
		int randMass = rand()%(maxMass-minMass)+minMass;
		//randomly generate position
		int randPosX = rand()%rankSize + mpi_myrank%ranksPerRow*rankSize;
		int randPosY = rand()%rankSize + mpi_myrank/ranksPerRow%ranksPerRow*rankSize;
		int randPosZ = rand()%rankSize + mpi_myrank/(ranksPerRow*ranksPerRow)*mpi_myrank*rankSize;
		//randomly generate velocity
		int randVelX = rand()%maxAbsVelocity;
		int randVelY = rand()%maxAbsVelocity;
		int randVelZ = rand()%maxAbsVelocity;
		init_body(&bodies[i], randMass, 10, randPosX, randPosY, randPosZ, randVelX, randVelY, randVelZ); //example of init_body
		#ifdef DEBUG1
			printf("ID: %d, Type: %s, Mass: %d\n", ((&bodies[i])->ID), types[((&bodies[i])->type)], ((&bodies[i])->mass));
			printf("Position: (%d, %d, %d)\n", ((&bodies[i])->posx),((&bodies[i])->posy),((&bodies[i])->posz));
			printf("Velocity: (%d, %d, %d)\n", ((&bodies[i])->vx),((&bodies[i])->vy),((&bodies[i])->vz));
		#endif
		#ifdef DEBUG
			int testSize = 1;
			randPosX = mpi_myrank%ranksPerRow*testSize;
			randPosY = mpi_myrank/ranksPerRow%ranksPerRow*testSize;
			randPosZ = mpi_myrank/(ranksPerRow*ranksPerRow)*testSize;
			printf("Rank %d Rank Division: (%d, %d, %d)\n", mpi_myrank, randPosX, randPosY, randPosZ);
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
		
		int offset = 1 + ranksPerRow + ranksPerRow*ranksPerRow;
		MPI_Request request;
		MPI_Status status;
		for(int j = 0; j < 3; j++) { 	//j == x, +1 -1
			for(int k = 0; k < 3; k++) {	//k == y, +-ranksPerRow
				for(int l = 0; l < 3; l++) {  //l == z +-/ranksPerRow*ranksPerRow
					//get mass from ranks
					if(!((j == 1 && k == 1) && l == 1)) {
						//row to be recieved and sent to
						//xbound up down 
						int shouldSend = 0;
						int shouldReceive = 0;
						if(!((mpi_myrank%ranksPerRow == (ranksPerRow-1)) && j == 2)) { //xbound up 
							if(!(((mpi_myrank/ranksPerRow)%ranksPerRow == (ranksPerRow-1)) && k == 2)) {  //ybound up
								if(!((mpi_myrank/(ranksPerRow*ranksPerRow) == ranksPerRow-1) && l == 2)) {  //zbound up
									if(!((mpi_myrank%ranksPerRow == 0) && j == 0)) {  //xbound down
										if(!(((mpi_myrank/ranksPerRow)%ranksPerRow == 0) && k == 0))	{  //ybound down
											if(!((mpi_myrank/(ranksPerRow*ranksPerRow) == 0) && l == 0)) {	//zbound down
												if(mpi_myrank - offset + j + k*ranksPerRow + l*ranksPerRow*ranksPerRow >= 0 && mpi_myrank - offset + j + k*ranksPerRow + l*ranksPerRow*ranksPerRow < mpi_commsize) {
												printf("Rank %d: %d %d %d, Mass: %d to Rank: %d\n", mpi_myrank, j, k, l, rankMass, mpi_myrank - offset + j + k*ranksPerRow + l*ranksPerRow*ranksPerRow);
												MPI_Isend(&rankMass, 1, MPI_INT, mpi_myrank- offset + j + k*ranksPerRow + l*ranksPerRow*ranksPerRow, mpi_myrank, MPI_COMM_WORLD, &request);
											}
								}
							}
						}
					
					}

				}
			}
				if(!((mpi_myrank%ranksPerRow == (ranksPerRow-1)) && j == 0)) { //xbound up 
					if(!(((mpi_myrank/ranksPerRow)%ranksPerRow == (ranksPerRow-1)) && k == 0)) {  //ybound up
						if(!((mpi_myrank/(ranksPerRow*ranksPerRow) == ranksPerRow-1) && l == 0)) {  //zbound up
							if(!((mpi_myrank%ranksPerRow == 0) && j == 2)) {  //xbound down
								if(!(((mpi_myrank/ranksPerRow)%ranksPerRow == 0) && k == 2))	{  //ybound down
									if(!((mpi_myrank/(ranksPerRow*ranksPerRow) == 0) && l == 2)) {	//zbound down
										if(mpi_myrank + offset - ( j + k*ranksPerRow + l*ranksPerRow*ranksPerRow) >= 0 && mpi_myrank + offset - (j + k*ranksPerRow + l*ranksPerRow*ranksPerRow) < mpi_commsize) {
												MPI_Recv(&otherRankMasses[mpi_myrank+offset - j - k*ranksPerRow - l*ranksPerRow*ranksPerRow], 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
												printf("Rank %d: %d %d %d, Mass: %d from rank %d\n", mpi_myrank, j, k, l, otherRankMasses[mpi_myrank+offset - j - k*ranksPerRow - l*ranksPerRow*ranksPerRow], 
													mpi_myrank+offset - j - k*ranksPerRow - l*ranksPerRow*ranksPerRow);
											}
														}
													}
												}
											}
										}
									}
				}
			}
		}
}

		//calculates the velocity considering the adjacent ranks
		for(int j=0;j<num_bodies;j++){
			xForce = 0;
			yForce = 0;
			zForce = 0;

			for(int k=0;k<27;k++){
				//calculate position of bodies
				int randPosY = rand()%rankSize + mpi_myrank/ranksPerRow%ranksPerRow*rankSize;
				int randPosZ = rand()%rankSize + mpi_myrank/(ranksPerRow*ranksPerRow)*mpi_myrank*rankSize;
				int kx = mpi_myrank%ranksPerRow*rankSize + k%3 - rankSize/2;
				int ky = (mpi_myrank/3)%3*rankSize + (k/3)%3 - rankSize/2;
				int kz = (mpi_myrank/9)*rankSize + (k/9) - rankSize/2;

			if(otherRankMasses[k] != 0) {
				float distance = sqrt( pow(bodies[j].posx-kx, 2) + pow(bodies[j].posy-ky, 2) + pow(bodies[j].posz-kz, 2));
				//Sum up the x,y,z forces on the current object, and apply them in the positive or negative direction as appropriate
					xForce += gravity * bodies[j].mass * otherRankMasses[k] / pow(distance,3) * (kx - bodies[j].posx);
					yForce += gravity * bodies[j].mass * otherRankMasses[k] / pow(distance,3) * (ky - bodies[j].posy);
					zForce += gravity * bodies[j].mass * otherRankMasses[k] / pow(distance,3) * (kz - bodies[j].posz);
			}
			
			}

			// Use the calculated forces to update the velocity of each body
			bodies[j].vx += (xForce / bodies[j].mass) * tickTimeStep;
			bodies[j].vy += (yForce / bodies[j].mass) * tickTimeStep;
			bodies[j].vz += (zForce / bodies[j].mass) * tickTimeStep;

		}

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

			//Array of the number of objects to be sent to each other rank
			//Ranks ordered bottom to top, left to right, back to forward
			//int to_other_ranks[mpi_commsize] = 0;

			// Check rank changes at boundaries based on position
			// Compare positions to rank boundaries and figure out the rank it belongs to
			// If the rank it belongs to is a different rank, MPI_Isend(newrank)
			// Set body's type to -1 in current rank


		}

		// TODO: Accept any new bodies that passed the rank boundary 
		//for (int i = 0; i < mpi_commsize; i++) {
		#ifdef DEBUG
			int i = 0;
			int ranksPerRow = root(mpi_commsize, 3);
			for (int x = -1; x <= 1; x++) {
				for (int y = -1; y <= 1; y++) {
					for (int z = -1; z <= 1; z++) {
						// Edge cases - these ranks don't actually exist
						if (mpi_myrank + x < 0) {
							continue;
						} else if (mpi_myrank + ranksPerRow*y < 0) {
							continue;
						} else if (mpi_myrank + ranksPerRow*ranksPerRow*z < 0) {
							continue;
						}
						//int newrank = mpi_myrank + (mpi_myrank + x >= 0 ? x : -27) + (mpi_myrank + ranksPerRow*y >= 0 ? ranksPerRow*y : -27) + (mpi_myrank + ranksPerRow*ranksPerRow*z >= 0 ? ranksPerRow*ranksPerRow*z : -27);
						int newrank = mpi_myrank + (x) + (ranksPerRow*y) + (ranksPerRow*ranksPerRow*z);
						if (newrank >= max(0, mpi_myrank-13) && newrank <= min(mpi_commsize-1, mpi_myrank+13) && mpi_myrank != newrank) {
							//printf("i = %d, Rank %d = (%d, %d, %d)\n", i, newrank, x, y, z);
							// MPI_iprobe(newrank) to check for a body that's been sent
							// Realloc array to store new body
							// increment num_bodies
							// MPI_recv(newrank) if iprobe confirms there's a body that's been sent from another rank
						}
						i++;
					}
				}
			}
			printf("%d\n", i);
		#endif
		//}

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

	totalBodyNum = num_bodies * mpi_commsize;
	totalBodies = calloc(totalBodyNum, sizeof(Body));

	MPI_Gather(bodies, num_bodies, MPI_BODY, totalBodies, num_bodies, MPI_BODY, 0, MPI_COMM_WORLD);

	if(mpi_myrank==0){
		output_Bodies(totalBodies, totalBodyNum);
	}
	MPI_Finalize();

	// Clean up allocated memory
	free(totalBodies);
	free(bodies);
	free(otherRankMasses);

	return EXIT_SUCCESS;

}


/* Experiments(methods/parameters/explanations):

variables:

ticks
tickTimeStep
minbodies
maxbodies
minMass
maxMass
universeSize
maxAbsVelocity

ranks:


implement strong scaling and weak scaling

If the amount of time to complete a work unit with 1 processing element is t1, 
and the amount of time to complete the same unit of work with N processing elements is tN, 
the strong scaling efficiency (as a percentage of linear) is given as:

t1 / ( N * tN ) * 100%


Strong scaling:



If the amount of time to complete a work unit with 1 processing element is t1, 
and the amount of time to complete N of the same work units with N processing elements is tN, 
the weak scaling efficiency (as a percentage of linear) is given as:

( t1 / tN ) * 100%

Weak scaling:



*/
