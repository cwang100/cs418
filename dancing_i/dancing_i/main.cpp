// CS418 MP1
// @author William Hempy
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GL/glew.h>
#include <GL/glut.h>

const int ESC = 27;

// Angle to rotate the object
float fRotateAngle = 0.f;

// Speed that the object dances/rotates
float fDance = 0.f;
float fRotateSpeed = 0.f;

// Control variables for user input
bool dspWireFrame = 0;
bool dance = 1;
bool rotate = 0;

// Framerate variables
int numFrames = 0;
int prevTime = 0;
float fps = 30;
int nFPS = 60;

// Function to calculate framerate
void calcFps();

// Initializes necessary flags for openGL
void init(void)
{
	glClearColor(0.0,0.0,0.0,1.0); // background color is blue
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //change modes to fill in triangles
	prevTime = glutGet(GLUT_ELAPSED_TIME); // get our program start time
	glLineWidth(5.0);
}

// A function to draw the "I" image to the buffer
void display(void)
{
	// put your OpenGL display commands here
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// reset OpenGL transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset transformation matrix to identity

	// setup look at transformation so that 
	// eye is at : (0,0,3)
	// look at center is at : (0,0,0)
	// up direction is +y axis
	gluLookAt(0.f,0.f,3.f,0.f,0.f,0.f,0.f,1.f,0.f);
	glRotatef(fRotateAngle,0.f,1.f,0.f);

	// "dance" using a sinusoidal function.
	float A = 0.25; //amplitude of oscillation in the x direction
	float B = 0.2; //amplitude of oscillation in the y direction

	float x0 = A*sin(0.07*fDance);
	float x1 = A*sin(0.07*fDance);
	float y0 = B*sin(0.15*fDance);
	float y1 = B*sin(0.145*fDance);
	/* Different colors on each vertex to blend
	 * and create the appearance of lighting.
	 */

	// Top half of the "i"
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0,0.0,0.0);			 // # vertices
			glVertex2f(-.6 +x1, 1 +y1);  //1
			glVertex2f(-.6 +x1,.6 +y1);  //2
			glVertex2f(-.2 +x0,.6 +y1);  //3
		glColor3f(1.0,0.25,0.0);
			glVertex2f(.2 +x0, .6 +y1);  //4
			glVertex2f(.6 +x0, .6 +y1);  //5
		glColor3f(1.0,0.35,0.0);
			glVertex2f(.6 +x0,1 +y1);    //6
	glEnd();

	// Bottom half of the "i"
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(0.90,0.15,0.0);
			glVertex2f(.6 +x0,-1 +y0);   //7
		glColor3f(1.0,0.25,0.0);
			glVertex2f(.6 +x0,-.6 +y0);  //8
		glColor3f(1.0,0.35,0.0);
			glVertex2f(.2 +x0,-.6 +y0);  //9
		glColor3f(0.75,0.0,0.0);
			glVertex2f(-.2 +x0,-.6 +y0); //10
		glColor3f(0.75,0.0,0.0);
			glVertex2f(-.6 +x0,-.6 +y0); //11
		glColor3f(0.55,0.0,0.0);
			glVertex2f(-.6 +x0,-1 +y0);  //12
	glEnd();

	// Midsection of the "i"
	glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0,0.0,0.0);
		glVertex2f(-.2 +x0,.6 +y1);		 //13
		glColor3f(1.0,0.25,0.0);
		glVertex2f(.2 +x0,.6 +y1);		 //14
		glVertex2f(.2 +x0,-.6 +y0);		 //15
		glColor3f(0.75,0.0,0.0);
		glVertex2f(-.2 +x0,-.6 +y0);	 //16
	glEnd();
	
	// FPS TEXT 
	glMatrixMode( GL_PROJECTION ) ;
	glPushMatrix() ; 
	glLoadIdentity();
	glMatrixMode( GL_MODELVIEW ) ;
	glPushMatrix() ;
	glLoadIdentity() ;
	glDisable( GL_DEPTH_TEST ) ; // Turn off depth testing
	// Make sure our draw does not affect the "I"

	// Draw the framerate at the bottom corner of the screen
	glRasterPos2f(-.95, -.95) ; 
	glColor4f(0.0, 0.0, 0.0, 0.0);
	char str[300];
	sprintf_s( str, "FPS: %f", fps ) ;
	const char * p = str ;
	while (*p) {
		glutBitmapCharacter( GLUT_BITMAP_HELVETICA_18, *p);
		p++;
	}

	// Revert to the previous matrix and continue
	glEnable( GL_DEPTH_TEST ) ; 
	glMatrixMode( GL_PROJECTION ) ;
	glPopMatrix() ; 
	glMatrixMode( GL_MODELVIEW ) ;
	glPopMatrix() ;

	calcFps();
	glutSwapBuffers();	// swap front/back framebuffer to avoid flickering 
}

void reshape (int w, int h)
{
	// reset viewport ( drawing screen ) size
	glViewport(0, 0, w, h);
	float fAspect = ((float)w)/h; 
	// reset OpenGL projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.f,fAspect,0.001f,30.f); 
}



void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case ESC : 
		{
			printf("demonstration finished.\n");
			exit(0);
		}
		case 'w' :
		{
			dspWireFrame = !dspWireFrame;
			if (dspWireFrame)
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		}
		case 'r' : 
		{ 
			rotate = !rotate; 
			break;
		}
		case 'd' :
		{
			dance  = !dance;  
			break;
		}
	}
}

void SpecialInput(int key, int x, int y) {

		switch(key) {
			case GLUT_KEY_UP : {
				if (fRotateSpeed < 9)
				fRotateSpeed += 1.f;
			break;
			}
			case GLUT_KEY_DOWN : {
				if (fRotateSpeed < 9)
				fRotateSpeed -= 1.f;
				break;
			}
		}
}


void timer(int v)
{
	if (rotate) { fRotateAngle += 1.f + fRotateSpeed;} // change rotation angles
	if (dance) { fDance += 1.f; }
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again 
}

void calcFps()
{
	// Count the number of frames that have passed.
    numFrames++;
	
	// opengl functions 
	// The number of elapsed milliseconds since the last call.
	int time = glutGet(GLUT_ELAPSED_TIME);
	// printf("%d : %d\n",time,prevTime);
    // Calculate time passed
    int elapsed = time - prevTime;
	// if a second has passed, then calculate the framerate again.
    if (elapsed > 1000) {
        fps = numFrames / (elapsed / 1000.0f);
        prevTime = time;
        numFrames = 0;
    }
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);

	// set up for double-buffering & RGB color buffer & depth test
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 
	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ((const char*)"MP1 - Dancing 'I'");

	// set up GLEW and check for any errors
	GLenum err = glewInit(); 
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error %s\n", glewGetErrorString(err));
		exit(1);
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

	// notify the user when resources are available
	if (GLEW_ARB_vertex_program)
		fprintf(stdout, "Status: ARB vertex programs available.\n");
	if (glewGetExtension("GL_ARB_fragment_program"))
		fprintf(stdout, "Status: ARB fragment programs available.\n");
	if (glewIsSupported("GL_VERSION_1_4  GL_ARB_point_sprite"))
		fprintf(stdout, "Status: ARB point sprites available.\n");



	init(); // setting up user data & OpenGL environment
	
	// set up the call-back functions 
	glutDisplayFunc(display);  // called when drawing 
	glutReshapeFunc(reshape);  // called when change window size
	glutKeyboardFunc(keyboard); // called when received keyboard interaction
	glutSpecialFunc(SpecialInput); // called when recieved arrow key interaction
	glutTimerFunc(100,timer,nFPS); // a periodic timer. Usually used for updating animation
	glutMainLoop(); // start the main message-callback loop

	return 0;
}
