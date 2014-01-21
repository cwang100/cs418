/*
 * CS 418 MP2 : Flight Simulator 
 * @author Will Hempy
 * @netid hempy2
 */
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <stdio.h>

bool gameover = false;
bool pause = false;
float sealevel;
float seasize = 1;
float polysize;
float pitch = 0.029;
float roll = 0.029;
float velocity = 0.004;
float nFPS = 60;

// The eye point 
float eye_x = 0.50;
float eye_y = 0.00;
float eye_z = 0.25;

// The lookat point
float center_x = 0.00;
float center_y = 0.00;
float center_z = 0.00;

// The up vector
float up_x = 0.0;
float up_y = 1.0;
float up_z = 0.0;

int seed(float x, float y) {
    static int a = 1588635695, b = 1117695901;
	int xi = *(int *)&x;
	int yi = *(int *)&y;
    return ((xi * a) % b) - ((yi * b) % a);
}

void
mountain(float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float s)
{
	float x01,y01,z01,x12,y12,z12,x20,y20,z20;

	if (s < polysize) {
		x01 = x1 - x0;
		y01 = y1 - y0;
		z01 = z1 - z0;

		x12 = x2 - x1;
		y12 = y2 - y1;
		z12 = z2 - z1;

		x20 = x0 - x2;
		y20 = y0 - y2;
		z20 = z0 - z2;

		float nx = y01*(-z20) - (-y20)*z01;
		float ny = z01*(-x20) - (-z20)*x01;
		float nz = x01*(-y20) - (-x20)*y01;

		float den = sqrt(nx*nx + ny*ny + nz*nz);

		if (den > 0.0) {
			nx /= den;
			ny /= den;
			nz /= den;
		}

		glNormal3f(nx,ny,nz);
		glBegin(GL_TRIANGLES);
			glVertex3f(x0,y0,z0);
			glVertex3f(x1,y1,z1);
			glVertex3f(x2,y2,z2);
		glEnd();

		return;
	}

	x01 = 0.5*(x0 + x1);
	y01 = 0.5*(y0 + y1);
	z01 = 0.5*(z0 + z1);

	x12 = 0.5*(x1 + x2);
	y12 = 0.5*(y1 + y2);
	z12 = 0.5*(z1 + z2);

	x20 = 0.5*(x2 + x0);
	y20 = 0.5*(y2 + y0);
	z20 = 0.5*(z2 + z0);

	s *= 0.5;

	srand(seed(x01,y01));
	z01 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x12,y12));
	z12 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);
	srand(seed(x20,y20));
	z20 += 0.3*s*(2.0*((float)rand()/(float)RAND_MAX) - 1.0);

	mountain(x0,y0,z0,x01,y01,z01,x20,y20,z20,s);
	mountain(x1,y1,z1,x12,y12,z12,x01,y01,z01,s);
	mountain(x2,y2,z2,x20,y20,z20,x12,y12,z12,s);
	mountain(x01,y01,z01,x12,y12,z12,x20,y20,z20,s);
}

void init(void) 
{
	GLfloat white[] = {1.0,1.0,1.0,1.0};
	GLfloat lpos[] = {0.0,1.0,0.0,0.0};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	glClearColor (0.5, 0.5, 1.0, 0.0);
	/* glShadeModel (GL_FLAT); */
	glEnable(GL_DEPTH_TEST);

	sealevel = 0.00;
	polysize = 0.01;

	glLoadIdentity();
}

/*
 * A function to calculate the rotation for the pitch
 * of the plane. This will change the locations of the
 * up vector and the lookat vector, but not the eye
 * point itself.
 *
 * @input up Control for the pitch direction
 */
void tilt(bool up)
{
	/* lookat vector */
	float x0 = center_x - eye_x;
	float y0 = center_y - eye_y;
	float z0 = center_z - eye_z;

	/* angles */
	float s;
	float c;

	/* up vector */
	float x1 = up_x - eye_x;
	float y1 = up_y - eye_y;
	float z1 = up_z - eye_z;

   /* The cross product of the 
    * lookat vector and normalize
	* the resultant vector. */
	float x = y0*z1 - z0*y1;
	float y = x1*z0 - x0*z1;
	float z = x0*y1 - y0*x1;

	/* normalize the vector */
	float norm = sqrtf(x*x + y*y + z*z);
	x = x/norm;
	y = y/norm;
	z = z/norm;

	if (up)
	{
		s = sinf(pitch);
		c = cosf(pitch);
	}
	else
	{
		s = sinf(-1*pitch);
		c = cosf(-1*pitch);
	}
	
	up_x = x1*(powf(x,2)*(1-c)+c)
		 + y1*(x*y*(1-c)-z*s)
		 + z1*(x*z*(1-c)+y*s);
	up_y = x1*(x*y*(1-c)+z*s)
		 + y1*(powf(y,2)*(1-c)+c)
		 + z1*(y*z*(1-c)-x*s);
	up_z = x1*(x*z*(1-c)-y*s)
		 + y1*(y*z*(1-c)+x*s)
		 + z1*(powf(z,2)*(1-c)+c);
	up_x += eye_x;
	up_y += eye_y;
	up_z += eye_z;

	center_x = x0*(powf(x,2)*(1-c)+c)
			 + y0*(x*y*(1-c)-z*s)
			 + z0*(x*z*(1-c)+y*s);
	center_y = x0*(x*y*(1-c)+z*s)
			 + y0*(powf(y,2)*(1-c)+c)
			 + z0*(y*z*(1-c)-x*s);
	center_z = x0*(x*z*(1-c)-y*s)
			 + y0*(y*z*(1-c)+x*s)
			 + z0*(powf(z,2)*(1-c)+c);
	center_x += eye_x;
	center_y += eye_y;
	center_z += eye_z;
}

/*
 * A function to calculate a rotation about the lookat 
 * direction that uses the equation for rotation about
 * an arbitrary axis. 
 *
 * @input clockwise The rotation direction.
 */
void spin(bool clockwise)
{	
	/* lookat vector */
	float x = center_x - eye_x;
	float y = center_y - eye_y;
	float z = center_z - eye_z;

	/* angles */
	float s;
	float c;

	/* normalize the vector */
	float norm = sqrtf(x*x + y*y + z*z);
	x = x/norm;
	y = y/norm;
	z = z/norm;

	float x0 = up_x - eye_x;
	float y0 = up_y - eye_y;
	float z0 = up_z - eye_z;

	if (clockwise) 
	{
		s = sinf(roll);
		c = cosf(roll);
	} 
	else 
	{
		s = sinf(-1*roll);
		c = cosf(-1*roll);
	}

	up_x = x0*(powf(x,2)*(1-c)+c)
		 + y0*(x*y*(1-c)-z*s)
		 + z0*(x*z*(1-c)+y*s);
	up_y = x0*(x*y*(1-c)+z*s)
		 + y0*(powf(y,2)*(1-c)+c)
		 + z0*(y*z*(1-c)-x*s);
	up_z = x0*(x*z*(1-c)-y*s)
		 + y0*(y*z*(1-c)+x*s)
		 + z0*(powf(z,2)*(1-c)+c);
	up_x += eye_x;
	up_y += eye_y;
	up_z += eye_z;

}

void move()
{
	/* automatic translation */
	float x_add = center_x - eye_x;
	float y_add = center_y - eye_y;
	float z_add = center_z - eye_z;

	eye_x += velocity*(x_add);
	eye_y += velocity*(y_add);
	eye_z += velocity*(z_add);

	center_x += velocity*(x_add);
	center_y += velocity*(y_add);
	center_z += velocity*(z_add);

	up_x += velocity*(x_add);
	up_y += velocity*(y_add);
	up_z += velocity*(z_add);
}

void display(void){

	if (eye_y < sealevel - 0.15) { 
		printf("GAME OVER\n");
		gameover = true;
		return;
	}

	static GLfloat angle = 0.0;

	GLfloat tanamb[] = {0.2,0.15,0.1,1.0};
	GLfloat tandiff[] = {0.4,0.3,0.2,1.0};

	GLfloat seaamb[] = {0.0,0.0,0.2,1.0};
	GLfloat seadiff[] = {0.0,0.0,0.8,1.0};
	GLfloat seaspec[] = {0.5,0.5,1.0,1.0};

	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);

	glLoadIdentity();
	
	if (!pause)
	move();

	/* viewing transformation  */
	gluLookAt (eye_x, eye_y, eye_z, center_x, center_y, center_z, up_x, up_y, up_z);

	/** A flying teapot?
	glPushMatrix();
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		glColor3f(1.0,0.0,0.0);
		glutSolidTeapot(0.09);
		glTranslatef(1.0,0.0,0.0);
		glRotatef(90.0,0.1,0.0,0.0);
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
	*/

	glPushMatrix();
	glTranslatef(0.0,-0.15,0.0);
	glRotatef(-90,1.0,0.0,0.0);
	glTranslatef(-0.4, -0.5, 0.0);      
	/* modeling transformation */

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, tanamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, tandiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tandiff);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	mountain(0.0,0.0,0.0, 1.0,0.0,0.0, 0.0,1.0,0.0, 1.0);
	mountain(1.0,1.0,0.0, 0.0,1.0,0.0, 1.0,0.0,0.0, 1.0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, seaamb);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, seadiff);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, seaspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0);

	glNormal3f(0.0,0.0,1.0);
	
	glBegin(GL_QUADS);
		glVertex3f(-seasize,-seasize,sealevel);
		glVertex3f(seasize,-seasize,sealevel);
		glVertex3f(seasize,seasize,sealevel);
		glVertex3f(-seasize,seasize,sealevel);
	glEnd();
	glPopMatrix();

	glutSwapBuffers();
	glFlush ();

	glutPostRedisplay();
}

void reshape (int w, int h)
{
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();
	gluPerspective(90.0,1.0,0.01,10.0);
	glMatrixMode (GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
		case 'p':
			pause = !pause;
			break;
		case ' ':
			if (velocity < 0.02)
			velocity += 0.001;
			break;
		case '\b':
			if (velocity > 0.002)
			velocity -= 0.001;
			break;
		case '-':
			sealevel -= 0.01;
			break;
		case '+':
		case '=':
			sealevel += 0.01;
			break;
		case 'f':
			polysize *= 0.5;
			break;
		case 'c':
			polysize *= 2.0;
			break;
		case 27:
			exit(0);
			break;
   }
}

void SpecialInput(int key, int x, int y) {
		switch(key) {
			case GLUT_KEY_UP :
				tilt(1);
				break;
			case GLUT_KEY_DOWN : 
				tilt(0);
				break;
			case GLUT_KEY_RIGHT :
				spin(1);
				break;
			case GLUT_KEY_LEFT :
				spin(0);
				break;
		}
}

void timer(int v)
{
	if (!gameover) {
		glutPostRedisplay(); 
		glutTimerFunc(1000/nFPS,timer,v);
	}
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize (500, 500); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutSpecialFunc(SpecialInput);
   glutTimerFunc(100,timer,nFPS); 
   glutMainLoop();
   return 0;
}
