#ifndef _MAIN_H
#define _MAIN_H

int nFPS = 20;
int rot = 1;
float fRotateAngle = 0.f;

void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
		case 27:
			// ESC hit, so quit
			printf("demonstration finished.\n");
			exit(0);
			break;
		case 'q':
			reflect = !reflect;
			break;
		case 'r':
			rot = !rot;
			break;
		case 'w':
			texture = !texture;
			break;
	}
}

void SpecialInput(int key, int x, int y) {
		switch(key) {
			case GLUT_KEY_UP :
				break;
			case GLUT_KEY_DOWN : 
				break;
			case GLUT_KEY_RIGHT :
				fRotateAngle += 1.f;
				break;
			case GLUT_KEY_LEFT :
				fRotateAngle -= 2.f;
				break;
		}
}

void mouse(int button, int state, int x, int y)
{
	// process your mouse control here
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		printf("push left mouse button.\n");
}

void timer(int v)
{
	if (rot) fRotateAngle += 1.f; // change rotation angles
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
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

void display(void)
{
	// put your OpenGL display commands here
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// reset OpenGL transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset transformation matrix to identity

	// setup look at transformation so that 
	// eye is at			: (-1,4,5)
	// look at center is at : (0,1,0)
	// up direction is +y axis
	gluLookAt(-1.f,4.f,5.f,0.f,1.f,0.f,0.f,1.f,0.f);
	glRotatef(fRotateAngle,0.f,1.f,0.f);

	map_texture(); // Maps the texture files to surface.
	draw_teapot(); // draws the teapot from the given triangles
	disable_flags(); // Disables textures enabled from mapping.

	glFlush();
	glutSwapBuffers();	// swap front/back framebuffer to avoid flickering 
}

void init(void)
{
	// set up for double-buffering & RGB color buffer & depth test
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH); 
	glutInitWindowSize (500, 500); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow ((const char*)"MP3: Teapot");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error %s\n", glewGetErrorString(err));
		exit(1);
	}
	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	if (GLEW_ARB_vertex_program)
		fprintf(stdout, "Status: ARB vertex programs available.\n");
	if (glewGetExtension("GL_ARB_fragment_program"))
		fprintf(stdout, "Status: ARB fragment programs available.\n");
	if (glewIsSupported("GL_VERSION_1_4  GL_ARB_point_sprite"))
		fprintf(stdout, "Status: ARB point sprites available.\n");

	// set up the call-back functions 
	glutDisplayFunc(display);  // called when drawing 
	glutReshapeFunc(reshape);  // called when change window size
	glutTimerFunc(100,timer,nFPS); // a periodic timer. Usually used for updating animation
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(SpecialInput);

	// init your data, setup OpenGL environment here
	glClearColor(0.9,0.9,0.9,1.0); // clear color is gray		
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // uncomment this function if you only want to draw wireframe model
	glEnable(GL_DEPTH_TEST);
  
	glDepthFunc(GL_LEQUAL);	// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective 
	glShadeModel(GL_SMOOTH);

	// Enable lighting
	GLfloat white[] = {1.0,1.0,1.0,1.0};
	GLfloat lpos[] = {0.0,5.0,0.0,0.0};
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	calculate_normals(); // initializes the normals from the .obj file.
	load_files(); // Loads the texture files from memory..
}

#endif