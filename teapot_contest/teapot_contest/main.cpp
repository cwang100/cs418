/** 
 * CS 418 MP3: Teapot Contest
 *
 * @author Will Hempy
 * @netid hempy2
 */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <SOIL/SOIL.h>

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

#include "teapot.h"
#include "main.h"

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);
	init(); // setting up user data & OpenGL environment
	
	glutMainLoop(); // start the main message-callback loop

	return 0;
}