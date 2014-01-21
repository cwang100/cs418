#ifndef _TEAPOT_H
#define _TEAPOT_H

// Selector for the teapot texture
int reflect = 1;
int texture = 1;

// The texture/environment for the teapot
GLuint tex, tex_2;
GLuint env;

// The set of vertices and faces.
vector<tuple<double,double,double>> verts;
vector<tuple<int,int,int>> faces;

// The set of per-vertex normals and face normals.
vector<tuple<double,double,double>> v_normals;
vector<tuple<double,double,double>> f_normals;

/**
 * A debugging function to print the vectors
 * storing the vertices and faces.
 */
template<class T>
void print_vector(vector<tuple<T,T,T>> in) {
	for (int i = 0; i < in.size(); i++) {
		double a = (double)get<0>(in[i]);
		double b = (double)get<1>(in[i]);
		double c = (double)get<2>(in[i]);
		printf("%lf %lf %lf\n",a,b,c);
	}
}

/**
 * A function to calculate the face normals and vertex normals
 * which will be stored into the global arrays listed above.
 */
void calculate_normals() {
	string line;
	ifstream myfile ("teapot_0.obj");
		
	// Since a 1-based index is used for vertices, a dummy element.
	verts.push_back(make_tuple(0.0,0.0,0.0));
	v_normals.push_back(make_tuple(0.0,0.0,0.0));

	double v[3], f[3];

	// Read in the vertices and faces from the privided file.
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
				verts.push_back(make_tuple(v[0],v[1],v[2]));
				break;
			case 'f':
				for (int i = 0; i < 3; i++) {
					f[i] = atoi(strtok(NULL," "));
				}
				faces.push_back(make_tuple(f[0],f[1],f[2]));
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
		system("pause");
		exit(EXIT_FAILURE);
	}

	double x,y,z,x1,y1,z1,x2,y2,z2;
	for (size_t i = 0; i < faces.size(); i++) {

		// Get the two vectors corresponding to two sides of the face.
		x1 = get<0>(verts[get<1>(faces[i])]) - get<0>(verts[get<0>(faces[i])]);
		y1 = get<1>(verts[get<1>(faces[i])]) - get<1>(verts[get<0>(faces[i])]);
		z1 = get<2>(verts[get<1>(faces[i])]) - get<2>(verts[get<0>(faces[i])]);
		
		x2 = get<0>(verts[get<2>(faces[i])]) - get<0>(verts[get<0>(faces[i])]);
		y2 = get<1>(verts[get<2>(faces[i])]) - get<1>(verts[get<0>(faces[i])]);
		z2 = get<2>(verts[get<2>(faces[i])]) - get<2>(verts[get<0>(faces[i])]);

		// Take the cross product of both of them to get the normal vector.
		x = y1*z2-y2*z1;
		y = x2*z1-x1*z2;
		z = x1*y2-x2*y1;

		// Normalize.
		double norm = sqrtf(x*x + y*y + z*z);
		x = x/norm;
		y = y/norm;
		z = z/norm;

		f_normals.push_back(make_tuple(x,y,z));
	}
	
	// Count the number of face normals for a vertex and accumulate the values.
	double acc;
	for (size_t i = 1; i < verts.size(); i++) {

		acc = x = y = z = 0;

		// Find the faces associated with the current vertex.
		for (size_t j = 0; j < faces.size(); j++) {
			
			// Check each element in the tuple to see if it matches the vertex.
			if (get<0>(faces[j]) == i || get<1>(faces[j]) == i || get<2>(faces[j]) == i) {
				x+= get<0>(f_normals[j]);
				y+= get<1>(f_normals[j]);
				z+= get<2>(f_normals[j]);
				acc++;
			}
		}

		// Average and normalize to get the result.
		x = x/acc; y = y/acc; z = z/acc;
		double norm = sqrtf(x*x + y*y + z*z);
		x = x/norm; y = y/norm; z = z/norm;

		v_normals.push_back(make_tuple(x,y,z));
	}	
}

/** 
 * Draws the teapot given the array of 
 * triangles has been initialized.
 */
void draw_teapot() {
	static const float sTexCoord[3] = { 0.5, 0, 0 };
    static const float tTexCoord[3] = { 0, 0.5, 0 };
	
	double s,t,u,v,w,x,y,z;
	for (size_t i = 0; i < faces.size(); i++) {
		glBegin(GL_TRIANGLES);
			u = get<0>(v_normals[get<0>(faces[i])]);
			v = get<1>(v_normals[get<0>(faces[i])]);
			w = get<2>(v_normals[get<0>(faces[i])]);
			x = get<0>(verts[get<0>(faces[i])]);
			y = get<1>(verts[get<0>(faces[i])]);
			z = get<2>(verts[get<0>(faces[i])]);

			float length = sqrt(x*x+y*y+z*z);
			
			// Object planar texture mapping
			s = ( x * sTexCoord[0] ) + ( y * sTexCoord[1] ) + ( z * sTexCoord[2] );
			t = ( x * tTexCoord[0] ) + ( y * tTexCoord[1] ) + ( z * tTexCoord[2] );
			glMultiTexCoord2d(GL_TEXTURE1_ARB,s,t);
			glNormal3f(u,v,w);
			glVertex3f(x,y,z);

			u = get<0>(v_normals[get<1>(faces[i])]);
			v = get<1>(v_normals[get<1>(faces[i])]);
			w = get<2>(v_normals[get<1>(faces[i])]);
			x = get<0>(verts[get<1>(faces[i])]);
			y = get<1>(verts[get<1>(faces[i])]);
			z = get<2>(verts[get<1>(faces[i])]);

			length = sqrt(x*x+y*y+z*z);
			
			// Object planar texture mapping
			s = ( x * sTexCoord[0] ) + ( y * sTexCoord[1] ) + ( z * sTexCoord[2] );
			t = ( x * tTexCoord[0] ) + ( y * tTexCoord[1] ) + ( z * tTexCoord[2] );
			glMultiTexCoord2d(GL_TEXTURE1_ARB,s,t);		
			glNormal3f(u,v,w);
			glVertex3f(x,y,z);

			u = get<0>(v_normals[get<2>(faces[i])]);
			v = get<1>(v_normals[get<2>(faces[i])]);
			w = get<2>(v_normals[get<2>(faces[i])]);
			x = get<0>(verts[get<2>(faces[i])]);
			y = get<1>(verts[get<2>(faces[i])]);
			z = get<2>(verts[get<2>(faces[i])]);

			length = sqrt(x*x+y*y+z*z);
			
			// Object planar texture mapping
			s = ( x * sTexCoord[0] ) + ( y * sTexCoord[1] ) + ( z * sTexCoord[2] );
			t = ( x * tTexCoord[0] ) + ( y * tTexCoord[1] ) + ( z * tTexCoord[2] );
			glMultiTexCoord2d(GL_TEXTURE1_ARB,s,t);
			glNormal3f(u,v,w);
			glVertex3f(x,y,z);
		glEnd();
	}
}

/**
 * Loads the texture files from the disk.
 * Places references to the files into global 
 * texture variables.
 */
void load_files() {
	printf("Loading texture file . . .");
	tex_2 = SOIL_load_OGL_texture 
	(
		"brick.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(tex_2 == 0) // check for an error during the load process 
    {
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }
	else 
	{
		printf( "success!\n" );
	}

	glActiveTexture(GL_TEXTURE1_ARB);
	printf("Loading texture file . . .");
	tex = SOIL_load_OGL_texture 
	(
		"UIUC_logo.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(tex == 0) // check for an error during the load process 
    {
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
    }
	else 
	{
		printf( "success!\n" );
	}

	glActiveTexture(GL_TEXTURE0_ARB);
	printf("Loading background   . . .");
	env = SOIL_load_OGL_cubemap
	( 
		"Kastellholmen/posx.jpg",
        "Kastellholmen/negx.jpg",
        "Kastellholmen/posy.jpg",
        "Kastellholmen/negy.jpg",
        "Kastellholmen/posz.jpg",
        "Kastellholmen/negz.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

	if(env == 0) // check for an error during the load process 
	{
		printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
	}
	else
	{
		printf( "success!\n" );
	}
}

/**
 * Draws the background as a cube around the
 * camera.
 *
 * @input texID the texture of the background.
 */
void DrawCubeMap( GLuint texID )
{
    glPushMatrix();

    glLoadIdentity();
	glActiveTexture(GL_TEXTURE0_ARB);
    glDepthMask( GL_FALSE );  // Don't write to the depth buffer
    glEnable( GL_TEXTURE_CUBE_MAP );
    glBindTexture( GL_TEXTURE_CUBE_MAP, texID );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE ); // Don't do any blending on the cube map textures

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBegin( GL_QUADS );
    {
        // +X
        glTexCoord3f(  1, -1, -1 );
        glVertex3f(    1, -1, -1 );

        glTexCoord3f(  1,  1, -1 );
        glVertex3f(    1,  1, -1 );

        glTexCoord3f(  1,  1,  1 );
        glVertex3f(    1,  1,  1 );

        glTexCoord3f(  1, -1,  1 );
        glVertex3f(    1, -1,  1 );

        // -X
        glTexCoord3f( -1, -1, -1 );
        glVertex3f(   -1, -1, -1 );

        glTexCoord3f( -1,  1, -1 );
        glVertex3f(   -1,  1, -1 );

        glTexCoord3f( -1,  1,  1 );
        glVertex3f(   -1,  1,  1 );

        glTexCoord3f( -1, -1,  1 );
        glVertex3f(   -1, -1,  1 );

        // +Y
        glTexCoord3f( -1,  1, -1 );
        glVertex3f(   -1,  1, -1 );

        glTexCoord3f(  1,  1, -1 );
        glVertex3f(    1,  1, -1 );

        glTexCoord3f(  1,  1,  1 );
        glVertex3f(    1,  1,  1 );

        glTexCoord3f( -1, 1,  1 );
        glVertex3f(   -1, 1,  1 );

        // -Y
        glTexCoord3f( -1, -1, -1 );
        glVertex3f(   -1, -1, -1 );

        glTexCoord3f(  1, -1, -1 );
        glVertex3f(    1, -1, -1 );

        glTexCoord3f(  1, -1,  1 );
        glVertex3f(    1, -1,  1 );

        glTexCoord3f( -1, -1,  1 );
        glVertex3f(   -1, -1,  1 );

        // +Z
        glTexCoord3f( -1, -1,  1 );
        glVertex3f(   -1, -1,  1 );

        glTexCoord3f(  1, -1,  1 );
        glVertex3f(    1, -1,  1 );

        glTexCoord3f(  1,  1,  1 );
        glVertex3f(    1,  1,  1 );

        glTexCoord3f( -1,  1,  1 );
        glVertex3f(   -1,  1,  1 );

        // -Z
        glTexCoord3f( -1, -1, -1 );
        glVertex3f(   -1, -1, -1 );

        glTexCoord3f(  1, -1, -1 );
        glVertex3f(    1, -1, -1 );

        glTexCoord3f(  1,  1, -1 );
        glVertex3f(    1,  1, -1 );

        glTexCoord3f( -1,  1, -1 );
        glVertex3f(   -1,  1, -1 );

        glEnd();
    }

    glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
    glDisable( GL_TEXTURE_CUBE_MAP );
    glDepthMask( GL_TRUE );

    glPopMatrix();
}

/**
 * Maps the texture loaded in from memory onto 
 * the teapot. 
 */
void map_texture() {
	
	// Map the environment around the teapot
	DrawCubeMap(env);
	
	if (reflect)
	{
		// Map the environment reflection to the teapot
		glActiveTexture(GL_TEXTURE0_ARB);
		glEnable( GL_TEXTURE_CUBE_MAP );
		glBindTexture( GL_TEXTURE_CUBE_MAP, env );

		glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
		glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_REPLACE);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
		
		glEnable(GL_TEXTURE_GEN_S);
		glEnable(GL_TEXTURE_GEN_T);
		glEnable(GL_TEXTURE_GEN_R);
	}
	
	// Map a specified texture onto the teapot.
	glActiveTexture(GL_TEXTURE1_ARB);
	glEnable(GL_TEXTURE_2D);

	if (texture)
		glBindTexture(GL_TEXTURE_2D,tex);
	else
		glBindTexture(GL_TEXTURE_2D,tex_2);

	glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE_EXT);
	glTexEnvf (GL_TEXTURE_ENV, GL_COMBINE_RGB_EXT, GL_INCR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_DECAL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // texture should tile
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/**
 * Disables all flags enabled from texture mapping.
 */
void disable_flags() {
	glActiveTexture(GL_TEXTURE0_ARB);
	glDisable(GL_TEXTURE_CUBE_MAP);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_2D);

	glActiveTexture(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
}

#endif