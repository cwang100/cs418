/**
 * CS418 MP4: Subdivision Surfaces
 * 
 * An implementation of the subdivision algorithm
 * that divides meshes into a greater number of 
 * shapes to create rounded edges. This particular
 * code uses the Catmull-Clark method.
 *
 * @author William Hempy
 * @netid hempy2
 * @credit-hours 3
 */
#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC

#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GL/glut.h>
#include <SOIL/SOIL.h>

#include <iostream>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>
#include <map>

using namespace std;

// Variables for the number of frames per second
// and the angle of automatic rotation.
int nFPS = 30;
float fRotateAngle = 0.f;

bool paused = false;
bool spin = false;
bool show_points = false;
bool show_missing_edges = false;
bool lighting = false;
bool texture = false;
GLuint tex;

// A floating point value used for double comparison
double epsilon = 0.000001;

size_t subdivision_state = 0;

// A structure to hold the vertices from the index-face set
struct vertex_t {
	double x;
	double y;
	double z;
	double nx;
	double ny;
	double nz;
	vertex_t() {
		x = y = z = 0;
		nx = ny = nz = 0;
	}
	vertex_t(double u, double v, double w) {
		x = u; y = v; z = w;
		nx = ny = nz = 0;
	}
	vertex_t& operator+=(const vertex_t& rhs) {
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}
};

inline vertex_t operator+(vertex_t lhs, const vertex_t& rhs) {
	lhs += rhs;
	return lhs;
}

inline vertex_t operator/(vertex_t lhs, double n) {
	lhs.x = lhs.x/n;
	lhs.y = lhs.y/n;
	lhs.z = lhs.z/n;
	return lhs;
}

inline vertex_t operator*(vertex_t lhs, double n) {
	lhs.x = lhs.x*n;
	lhs.y = lhs.y*n;
	lhs.z = lhs.z*n;
	return lhs;
}

inline vertex_t operator-(vertex_t lhs, const vertex_t & rhs) {
	lhs += rhs*(-1);
	return lhs;
}

inline bool operator==(vertex_t lhs, vertex_t rhs) {
	bool a = (fabs(lhs.x - rhs.x) < epsilon);
	bool b = (fabs(lhs.y - rhs.y) < epsilon);
	bool c = (fabs(lhs.z - rhs.z) < epsilon);
	return (a && b && c);
}

vertex_t cross_product(vertex_t& v1, vertex_t& v2) {
	vertex_t res;
	res.x = v1.y*v2.z-v2.y*v1.z;
	res.y = v2.x*v1.z-v1.x*v2.z;
	res.z = v1.x*v2.y-v2.x*v1.y;
	return res;
}

vertex_t normalize(const vertex_t& v) {
	double n = sqrtf(v.x*v.x+v.y*v.y+v.z*v.z);
	vertex_t res;
	res.x = v.x/n;
	res.y = v.y/n;
	res.z = v.z/n;
	return res;
}
// Gloabl storage for vertices
vector<vertex_t> verts;

// A structure to hold the faces of the mesh
struct face_t {
	int v[4];
	vertex_t centroid;
	vertex_t normal;
	face_t(int a, int b, int c, int d) {
		v[0] = a; v[1] = b; v[2] = c; v[3] = d;
		compute_fnormal();
	}
	void compute_fnormal() {
		vertex_t v1 = verts[v[1]] - verts[v[0]];
		vertex_t v2 = verts[v[2]] - verts[v[0]];
		normal = cross_product(v1,v2);
	}
};

// Global storage for faces
vector<face_t> faces;

// A structure used to sort the edges for quicker access
struct halfedge_t {
	halfedge_t * opp;
	halfedge_t * next;
	int vert;
	int face;
	vertex_t pt;
	vertex_t midpt;
	halfedge_t() {
		opp = next = NULL;
	}
};

// Global storage for the halfedges
map<pair<int, int>, halfedge_t*> edges;

// Global caches for the faces,vertices and edges.
vector<vector<vertex_t>> vert_cache;
vector<vector<face_t>> face_cache;
vector<map<pair<int,int>,halfedge_t*>> edge_cache;

// Global storage for the lookat eye point
vertex_t eye(0.f,1.f,2.f);

// The control points for the bezier curve
vertex_t p0(0.f,1.0f,3.f);
vertex_t p1(1.f,4.0f,2.f);
vertex_t p2(2.f,-2.0f,-1.f);
vertex_t p3(0.f,2.f,-3.f);

// The current progress along the bezeir curve path.
float t = FLT_EPSILON;
float fPathAngle = 0.f;

/**
 * Attempts to load the file specified 
 * in the input into memory. It stores a 
 * reference to the resulting texture in
 * a global Gluint.
 */
void read_tex_file(char * filename) {
	printf("Loading texture file: \"%s\" . . .",filename);
	tex = SOIL_load_OGL_texture 
	(
		filename,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(tex == 0) // check for an error during the load process
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
	else 
		printf( "success!\n" );
}

/**
 * Maps the texture bound to the 
 * global variable GLuint to the 
 * surface of the mesh.
 */
void map_texture() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/**
 * Disables any flags that were set
 * by the texture mapping.
 */
void disable_flags() {
	glDisable(GL_TEXTURE_2D);
}

/**
 * A function to read the input object file as
 * specified. This is initially our block "i"
 * as drawn out in the imesh3d.obj file included.
 *
 * @input filename the name of the index-face set file
 */
void read_obj_file(char * filename) {
	printf("Opening File . . . ");
	printf("%s\n",filename);

	string line;
	ifstream myfile (filename);
		
	// Since a 1-based index is used for vertices, a dummy element.
	verts.push_back(vertex_t(-100,-100,-100));

	double v[3];
	int f[4];

	// Read in the vertices and faces from the provided file.
	if ( myfile.is_open() ) {
		while ( getline (myfile,line) ) 
		{
			char * dup = _strdup(line.c_str());
			strtok(dup," ");

			switch(dup[0]) {
				case 'v':
					for (int i = 0; i < 3; i++) {
						v[i] = atof(strtok(NULL," "));
					}
					verts.push_back(vertex_t(v[0],v[1],v[2]));
					break;
				case 'f':
					for (int i = 0; i < 4; i++) {
						f[i] = atoi(strtok(NULL," "));
					}
					faces.push_back(face_t(f[0],f[1],f[2],f[3]));
					break;
				default:
					break;
			}
			free(dup);
		}
		myfile.close();
	}
	else {
		cout << "Unable to open file." << endl;
		_CrtDumpMemoryLeaks();
		exit(EXIT_FAILURE);
	}
}

/** 
 * (DEBUGGING FUNCTION)
 * Checks the current mesh for any errors.
 */ 
void sanity_check() {
	// Missing Edge Opposite Check
	int n = 0;
	for (map<pair<int,int>,halfedge_t*>::iterator it = edges.begin(); it != edges.end(); it++) {
		halfedge_t * edge = (*it).second;
		if (edge->opp == NULL) {
			n++;
			// Print all edges which have at least one in common with this one.
			printf("current edge: %d -- (%f,%f,%f)\n",edge->vert,verts[edge->vert].x,verts[edge->vert].y,verts[edge->vert].z);

		}
	}
	
	printf("vertices: %lu faces: %lu edges: %lu missing edges: %d\n",
		verts.size(),faces.size(),edges.size(),n);

	// Duplicate Vertices Check
	for (size_t i = 0; i < verts.size(); i++) {
		for (size_t j = 0; j < verts.size(); j++) {
			if ( verts[j] == verts[i] && i != j) {
				printf("duplicate vertex found: %d,%d\n",i,j);
			}
		}
	}
}

/**
 * A helper function that calculates
 * the centroid for every face.
 */
void calc_face_points() {
	for (vector<face_t>::iterator it = faces.begin(); it != faces.end(); it++) {
		face_t * cur = &(*it);
		cur->centroid = (verts[cur->v[0]] + verts[cur->v[1]] + verts[cur->v[2]] + verts[cur->v[3]])/4;
	}
}

/**
 * A helper function to calculate 
 * the edge midpoints for each of the edges.
 */
void calc_edge_points() {
	for (map<pair<int,int>,halfedge_t*>::iterator it = edges.begin(); it != edges.end(); it++) {
		halfedge_t * cur = (*it).second;
		cur->pt = (verts[cur->vert] + verts[cur->opp->vert] + faces[cur->face].centroid + faces[cur->opp->face].centroid)/4;
		cur->midpt = (verts[cur->vert] + verts[cur->opp->vert])/2;
	}
}

/**
 * A function that initializes the halfedge list given the 
 * initialized list of vertices.
 */
void init_edges() {
	for (size_t i = 0; i < faces.size(); i++) {
		face_t * f = &faces[i];

		// for each edge (u,v) of F 
		for (int k = 0; k < 4; k++) {
			// create the halfedge
			std::pair<int,int> pair = make_pair(f->v[k],f->v[(k+1)%4]);
			edges[ pair ] = new halfedge_t();
			edges[ pair ]->face = i;
			edges[ pair ]->vert = f->v[(k+1)%4];
		}

		// for each edge (u,v) of F
		for (int k = 0; k < 4; k++) {
			// set the next halfedge
			std::pair<int,int> pair = make_pair(f->v[k],f->v[(k+1)%4]);
			std::pair<int,int> next = make_pair(f->v[(k+1)%4],f->v[(k+2)%4]);
			std::pair<int,int> opp  = make_pair(f->v[(k+1)%4],f->v[k]);
			edges[ pair ]->next = edges[ next ];

			// set the opposite (if it exists)
			if ( edges.find(opp) != edges.end() ) {
				edges[ pair ]->opp = edges[ opp  ];
				edges[ opp  ]->opp = edges[ pair ];
			}
		}
	}

	// Recalculate the edge and face points.
	// These MUST be called in this order.
	calc_face_points();
	calc_edge_points();
}

/**
 * Stores the previous set of vertices,
 * faces, and edges for later use.
 */
void cache_mesh() {
	vert_cache.push_back(verts);
	face_cache.push_back(faces);
	edge_cache.push_back(edges);
}

/**
 * Does the shift by averaging
 * adjacent face and edge points for each face.
 */
void shift_vertices() {
	for (vector<face_t>::iterator it = faces.begin(); it != faces.end(); it++) {
		face_t * cur = &(*it);

		// For each original point.
		for (int i = 0 ; i < 4; i++) {
			double n = 0;
			vertex_t * P = &verts[cur->v[(i+1)%4]];
			vertex_t R,F;
			halfedge_t * edge = edges[ make_pair(cur->v[i],cur->v[(i+1)%4]) ];
			halfedge_t * tmp = edge;
			do {
				R += tmp->midpt;
				F += faces[tmp->face].centroid;
				tmp = tmp->next->opp;
				n++;
			} while (tmp != edge);
			R = R/n;
			F = F/n;
			*P = (F + R*2 + (*P)*(n-3))/n;
		}
	}
}

/** 
 * Finds the index in the vertex array matching
 * the given vertex. If no such vertex exists, 
 * it is inserted at the back and that index is 
 * returned.
 */
int find_vert(vertex_t vert) {
	for (size_t i = 0; i < verts.size(); i++) {
		if (verts[i] == vert) {
			return i;
		}
	}

	verts.push_back(vert);
	return verts.size()-1;
}

/**
 * Finishes the last step of the subdivision
 * by connecting the edge points, face points,
 * and new vertices together to form a new mesh.
 */
void connect_points() {
	// Remove the old data to create a new mesh
	faces.clear();
	edges.clear();

	// Use the cached versions to create the new set.
	for (size_t k = 0, latest = face_cache.size()-1; k < face_cache[latest].size(); k++) {
		face_t * cur = &face_cache[latest][k];

		// Connect the points
		verts.push_back(cur->centroid);
		int cen = verts.size() - 1;
		int mpt = cen + 1; 

		halfedge_t * edge = edge_cache[edge_cache.size()-1][ make_pair(cur->v[0],cur->v[1]) ];
		halfedge_t * tmp = edge;

		do {
			int pt_1 = find_vert(tmp->pt);
			int pt_2 = find_vert(tmp->next->pt);
			faces.push_back(face_t(pt_2,tmp->vert,pt_1,cen));
			tmp = tmp->next;
		} while (tmp != edge);

	}
	
	// Initialize the edges
	init_edges();
}

/**
 * Returns whether or not a cached version
 * of the current subdivision state exists.
 */
bool cached_mesh_exists() {
	if (vert_cache.size() > subdivision_state)
		return true;
	else 
		return false;
}

/**
 * Switches the currently displayed mesh to 
 * the globally set subdivision state.
 */
void switch_mesh() {
	verts = vert_cache[subdivision_state];
	faces = face_cache[subdivision_state];
	edges = edge_cache[subdivision_state];
}

/**
 * A function that will change the halfedge structure
 * by dividing the edges according to catmul-clark 
 * subdivision rules.
 *
 * Start with a manifold mesh. 
 * All vertices in the original mesh are called original points.
 *
 * Loops on:
 * 1. each face
 *		- Set a face point for each face to 
 *		  to be the centroid.
 * 2. each edge
 *		- Set an edge point that is the average
 *		  of the two neighboring face points.
 * 3. each original point P
 *		- F = average F of all face points touching P
 *		- R = average all edge midpoints of edges touching P
 *		- "Move" each original point to (n=4): (F + 2R + (n-3)P)/n
 *
 * Connect the new points.
 */
void catmul_clark_subdvision() {
	if (cached_mesh_exists()) {
		switch_mesh();
		return;
	}

	cache_mesh();
	shift_vertices();
	connect_points();
}

/**
 * Computes the vertex normals
 * for a given face using the average
 * of all adjacent faces. 
 * This assumes a correctly initialized
 * halfedge struct.
 */
void compute_vnormals(face_t& face) {
	for (int x = 1,i = 0; i < 4; i++, x = (i+1)%4) {
		int n = 0;
		vertex_t t;
		halfedge_t * edge = edges[ make_pair(face.v[i],face.v[x]) ];
		halfedge_t * tmp  = edge;
		do {
			t += face.normal;
			tmp = tmp->next->opp;
			n++;
		}
		while (tmp != edge);
		t = t/n;
		verts[face.v[x]].nx = t.x;
		verts[face.v[x]].ny = t.y;
		verts[face.v[x]].nz = t.z;
	}
}

/**
 * Draws the mesh as it is currently represented
 * within the global face/vertex arrays.
 */
void draw_mesh() {
	glColor3f(1.0,0.f,0.f);
	static const float sTexCoord[3] = { 1.0, 0, 1.0 };
    static const float tTexCoord[3] = { 0, 1.0, 0 };

	double s,t,u,v,w,x,y,z;
	for (size_t i = 0; i < faces.size(); i++) {
		compute_vnormals(faces[i]);
		glBegin(GL_QUADS);
			for (int j = 0; j < 4; j++) {
				x = verts[faces[i].v[j]].x;
				y = verts[faces[i].v[j]].y;
				z = verts[faces[i].v[j]].z;
				u = verts[faces[i].v[j]].nx;
				v = verts[faces[i].v[j]].ny;
				w = verts[faces[i].v[j]].nz;
				s = ( x * sTexCoord[0] ) + ( y * sTexCoord[1] ) + ( z * sTexCoord[2] );
				t = ( x * tTexCoord[0] ) + ( y * tTexCoord[1] ) + ( z * tTexCoord[2] );
				glTexCoord2d(s,t);
				glNormal3f(u,v,w);
				glVertex3f(x,y,z);
			}
		glEnd();
	}
}

/**
 * A function to change the camera eyepoint
 * such that it will follow a bezier curve path.
 *
 * Example: 
 * B(t) = (1-t)^3*P_0 + 3*(1-t)^2*t*P_1 + 3*(1-t)*t^2*P_2 + t^3*P_3
 */
void camera_rotate() {
	float s = 1-t;
	eye.x = powf(s,3)*p0.x + 3*powf(s,2)*t*p1.x + 3*powf(t,2)*s*p2.x + powf(t,3)*p3.x;
	eye.y = powf(s,3)*p0.y + 3*powf(s,2)*t*p1.y + 3*powf(t,2)*s*p2.y + powf(t,3)*p3.y;
	eye.z = powf(s,3)*p0.z + 3*powf(s,2)*t*p1.z + 3*powf(t,2)*s*p2.z + powf(t,3)*p3.z;
}


/**
 * (DEBUGGING FUNCTION)
 * A function that will print the current status
 * of the edge structure.
 */
void print_edges() {
	std::map<pair<int,int>,halfedge_t*>::iterator it;
	for (it = edges.begin(); it != edges.end(); it++) {
		pair<int,int> pair = (*it).first;
		halfedge_t * edge = (*it).second;
		printf("edge: (%d,%d) vertex: (%f,%f,%f)\n",pair.first,pair.second,
			verts[edge->vert].x,verts[edge->vert].y,verts[edge->vert].z);

		printf("\t edge->next->vertex: (%f,%f,%f)\n",
			verts[edge->next->vert].x,
			verts[edge->next->vert].y,
			verts[edge->next->vert].z);
		printf("\t edge->next->next->vertex: (%f,%f,%f)\n",
			verts[edge->next->next->vert].x,
			verts[edge->next->next->vert].y,
			verts[edge->next->next->vert].z);
		printf("\t edge->next->next->next->vertex: (%f,%f,%f)\n",
			verts[edge->next->next->next->vert].x,
			verts[edge->next->next->next->vert].y,
			verts[edge->next->next->next->vert].z);

		if (edge->opp != NULL)
		printf("\n\t edge->opp->vertex: (%f,%f,%f)\n",
			verts[edge->opp->vert].x,
			verts[edge->opp->vert].y,
			verts[edge->opp->vert].z);
	}
}

/** 
 * (DEBUGGING FUNCTION)
 * Draws the vertices as points, along with
 * all of the edge midpoints and centroids.
 */
void draw_points() {
	for (map<pair<int,int>,halfedge_t*>::iterator it = edges.begin(); it != edges.end(); it++) {
		halfedge_t * cur = (*it).second;
		glBegin(GL_POINTS);
			glColor3f(1.0,1.0,0); // yellow midpoints
			glVertex3f(cur->pt.x,cur->pt.y,cur->pt.z);
			glColor3f(0.f,1.0,0.f); // green face points
			glVertex3f(faces[cur->face].centroid.x,faces[cur->face].centroid.y,faces[cur->face].centroid.z);
		glEnd();
	}
}

/**
 * (DEBUGGING FUNCTION)
 * Draws all edges which have a missing
 * opposite pointer to visualize where 
 * the issues are in the algorithm.
 */
void draw_edges() {
	for (map<pair<int,int>,halfedge_t*>::iterator it = edges.begin(); it != edges.end(); it++) {
		halfedge_t * edge = (*it).second;
		if (edge->opp == NULL) {
			halfedge_t * tmp = edge->next->next->next;
			glColor3f(0.f,1.0,1.0); // cyan
			glBegin(GL_LINES);
				glVertex3f(verts[tmp->vert].x,verts[tmp->vert].y,verts[tmp->vert].z);
				glVertex3f(verts[edge->vert].x,verts[edge->vert].y,verts[edge->vert].z);
			glEnd();
		}
	}
}

/**
 * Enables or disables lighting
 * based on the value of the 
 * global variable.
 */
void enable_lighting() {
	if (lighting) {
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
	}
	else {
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);
	}
}

/**
 * The GLUT keyboard function that takes input from
 * the user.
 * 
 * @input key the key pressed
 * @input x the mouse location in relative window coordinates
 * @input y the mouse y location in relative window coordinates
 */
void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
		case 27:
			// ESC hit, so quit
			printf("demonstration finished.\n");
			_CrtDumpMemoryLeaks();
			system("pause");
			exit(0);
			break;
		case ',':
			// DEBUG: print the edges
			// print_edges();
			break;
		case '.':
			// DEBUG: show the points
			show_points = !show_points;
			break;
		case '/':
			// DEBUG: show broken edges
			show_missing_edges = !show_missing_edges;
			break;
		case 'p':
			// Pause the camera
			paused = !paused;
			break;
		case 'l':
			lighting = !lighting;
			enable_lighting();
			break;
		case 't':
			texture = !texture;
			if (texture)
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			else
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;
		case 'r':
			spin = !spin;
			break;
	}
}

/**
 * A GLUT function to track the state of 
 * the mouse.
 *
 * @input button the mouse button pressed
 * @input state the current state of the button
 * @input x the mouse location in relative window coordinates
 * @input y the mouse y location in relative window coordinates
 */
void mouse(int button, int state, int x, int y)
{
	// process your mouse control here
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (subdivision_state != 3) {
			subdivision_state++;
			catmul_clark_subdvision();
		}
		return;
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (subdivision_state != 0) {
			subdivision_state--;
			catmul_clark_subdvision();
		}
		return;
	}
}

/**
 * A function that calls itself periodically
 * so we can update the display inside the active
 * openGL window.
 */
void timer(int v)
{
	if (spin) 
		fRotateAngle += 1;
	if (!paused) { // movement has been paused
		fPathAngle += 0.01f;
		t = abs(sinf(fPathAngle)); // we want t to oscillate between [0,1]
	}
	glutPostRedisplay(); // trigger display function by sending redraw into message queue
	glutTimerFunc(1000/nFPS,timer,v); // restart timer again
}

/**
 * A function to handle the resizing of the
 * current window by the user.
 *
 * @input w the current width of the window
 * @input h the current height of the window
 */
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

/**
 * A function to display the objects drawn
 * in a window to the user.
 */
void display(void)
{
	// put your OpenGL display commands here
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// reset OpenGL transformation matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset transformation matrix to identity

	// setup look at transformation so that 
	// eye is at			: p0 (0,1,3)
	// The eye point will now follow a bezier curve path
	// from the initial point to p3 (globally stored);
	camera_rotate();
	// look at center is at : (0,0,0)
	// up direction is +y axis
	gluLookAt(eye.x,eye.y,eye.z,0.f,0.f,0.f,0.f,1.f,0.f);
	glRotatef(fRotateAngle,0.0,1.0,0.0);

	if (texture) map_texture();
	draw_mesh(); // Draw the current state of the mesh
	disable_flags();

	if (show_points)	// A debugging function.
		draw_points();
	if (show_missing_edges)
		draw_edges();

	glFlush();
	glutSwapBuffers();	// swap front/back framebuffer to avoid flickering 
}

/**
 * Initialization for openGL before displaying.
 */
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

	// init your data, setup OpenGL environment here
	glClearColor(0.0,0.0,0.0,1.0); // clear color is black		
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // uncomment this function if you only want to draw wireframe model
	glEnable(GL_DEPTH_TEST);
  
	glDepthFunc(GL_LEQUAL);	// The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);  // Really Nice Perspective 
	glShadeModel(GL_SMOOTH);

	// Enable lighting
	GLfloat white[] = {1.0,1.0,1.0,1.0};
	GLfloat lpos[] = {4.0,4.0,0.0,0.0};
	
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);

	read_obj_file("imesh3d.obj");
	read_tex_file("UIUC_logo.png");
	init_edges();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, (char**)argv);
	init(); // setting up user data & OpenGL environment
	
	glutMainLoop(); // start the main message-callback loop

	_CrtDumpMemoryLeaks();
	return 0;
}