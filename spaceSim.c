#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include<math.h>

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

//array of bodies for each rank
Body* bodies;

//universe bounds x, y, z
int boundx, boundy, boundz;
//tick time length
int ticks;




int main(int argc, char** argv) {
	//declare ranks

	//how are bodies distributed randomly?
	Body star;
	Body stroid;
	init_body(&star, "Star", 10, 10, 0, 0, 0, 1, 0, 0);
	init_body(&stroid, "Asteroid", 10, 10, 0, 0, 0, 1, 0, 0);

	printf("ID: %d, Type: %s, Mass: %d\n", ((&star)->ID), ((&star)->type), ((&star)->mass));
	printf("ID: %d, Type: %s, Mass: %d\n", ((&stroid)->ID), ((&stroid)->type), ((&stroid)->mass));
		//declare number, location, and velocity of objects

	//for each tick
		//calculate 
}


