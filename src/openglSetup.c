#include "slicer.h"



// window transformation routine 
void Transform(GLfloat Width, GLfloat Height)
{
	// Set the viewport 
	glViewport(0, 0, Width, Height);              
	
	// Select the projection matrix
	glMatrixMode(GL_PROJECTION);             
	
	// Reset The Projection Matrix
	glLoadIdentity();			
	
	// Calculate The Aspect Ratio Of The Window 
	gluPerspective(45.0,Width/Height,0.1,100.0);  
	
	// Switch back to the modelview matrix
	glMatrixMode(GL_MODELVIEW);                   
}



void myReshape( int w, int h )
{	
	glViewport( 0, 0, w, h );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
    
	if( w<=h ) {
		glFrustum( -4.0, 4.0, -4.0 * (GLfloat) h / (GLfloat) w,
				  4.0 * (GLfloat) h / (GLfloat) w, 1.0, 40.0 );
	}
	else {
		glFrustum( -4.0, 4.0, -4.0 * (GLfloat) w / (GLfloat) h,
				  4.0 * (GLfloat) w / (GLfloat) h, 1.0, 40.0 );
	}
	
    
	// or use glu perspective 
    //gluPerspective( 45.0, w/h, 1.0, 50.0 ); 
	
	glMatrixMode( GL_MODELVIEW );	
	
	// Sanity checks
	if (h==0)	h=1;                   
	if (w==0)	w=1;
	
	// do the transformation
	Transform( w, h );
	E->xWinSize = w;
	E->yWinSize = h;
}




void setShaders(e *E) {

	char *vs,*fs;
	const char *ff,*vv;
	char	vert_shader[1024];
	char	frag_shader[1024];
	
	E->v = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
	E->f = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);


	// load the ground from space shader
	sprintf(vert_shader,"%s/GroundFromSpaceVert.glsl",E->shader_path);
	sprintf(frag_shader,"%s/GroundFromSpaceFrag.glsl",E->shader_path);
	
	vs = textFileRead(vert_shader);
	fs = textFileRead(frag_shader);

	ff = fs;
	vv = vs;

	glShaderSourceARB(E->v, 1, &vv, NULL);
	glShaderSourceARB(E->f, 1, &ff, NULL);

	free(vs);
	free(fs);

	glCompileShaderARB(E->v);
	glCompileShaderARB(E->f);

	E->ground = glCreateProgramObjectARB();
	glAttachObjectARB(E->ground,E->v);
	glAttachObjectARB(E->ground,E->f);

	glLinkProgramARB(E->ground);
	
	//load the sky from space shader
	sprintf(vert_shader,"%s/SkyFromSpaceVert.glsl",E->shader_path);
	sprintf(frag_shader,"%s/SkyFromSpaceFrag.glsl",E->shader_path);
	
	vs = textFileRead(vert_shader);
	fs = textFileRead(frag_shader);

	ff = fs;
	vv = vs;

	glShaderSourceARB(E->v, 1, &vv, NULL);
	glShaderSourceARB(E->f, 1, &ff, NULL);

	free(vs);
	free(fs);

	glCompileShaderARB(E->v);
	glCompileShaderARB(E->f);

	E->atmo = glCreateProgramObjectARB();
	glAttachObjectARB(E->atmo,E->v);
	glAttachObjectARB(E->atmo,E->f);

	glLinkProgramARB(E->atmo);
	
	// load the contrast shader
	
	sprintf(vert_shader,"%s/contrast_vert.glsl",E->shader_path);
	sprintf(frag_shader,"%s/contrast_frag.glsl",E->shader_path);
	
	vs = textFileRead(vert_shader);
	fs = textFileRead(frag_shader);

	ff = fs;
	vv = vs;

	glShaderSourceARB(E->v, 1, &vv, NULL);
	glShaderSourceARB(E->f, 1, &ff, NULL);

	free(vs);
	free(fs);

	glCompileShaderARB(E->v);
	glCompileShaderARB(E->f);

	E->contrast = glCreateProgramObjectARB();
	glAttachObjectARB(E->contrast,E->v);
	glAttachObjectARB(E->contrast,E->f);

	glLinkProgramARB(E->contrast);
	
	// load the textureBlender shader
		
	sprintf(vert_shader,"%s/texBlend_contrast_vert.glsl",E->shader_path);
	sprintf(frag_shader,"%s/texBlend_contrast_frag.glsl",E->shader_path);
	
	vs = textFileRead(vert_shader);
	fs = textFileRead(frag_shader);

	ff = fs;
	vv = vs;

	glShaderSourceARB(E->v, 1, &vv, NULL);
	glShaderSourceARB(E->f, 1, &ff, NULL);

	free(vs);
	free(fs);

	glCompileShaderARB(E->v);
	glCompileShaderARB(E->f);

	E->texBlend = glCreateProgramObjectARB();
	glAttachObjectARB(E->texBlend,E->v);
	glAttachObjectARB(E->texBlend,E->f);

	glLinkProgramARB(E->texBlend);
	
	// load the bump_mappr shader
	
	sprintf(vert_shader,"%s/phonngVert.glsl",E->shader_path);
	sprintf(frag_shader,"%s/phonngFrag.glsl",E->shader_path);
	
	vs = textFileRead(vert_shader);
	fs = textFileRead(frag_shader);
	
	ff = fs;
	vv = vs;
	
	glShaderSourceARB(E->v, 1, &vv, NULL);
	glShaderSourceARB(E->f, 1, &ff, NULL);
	
	free(vs);
	free(fs);
	
	glCompileShaderARB(E->v);
	glCompileShaderARB(E->f);
	
	E->phong = glCreateProgramObjectARB();
	glAttachObjectARB(E->phong,E->v);
	glAttachObjectARB(E->phong,E->f);
	
	glLinkProgramARB(E->phong);
}

void shaderEnable(GLhandleARB p){
	glUseProgramObjectARB(p);
}

void shaderDisable(GLhandleARB p){
	glUseProgramObjectARB(NULL);
}

void SetUniformParameter1i(GLhandleARB p, const char *pszParameter, int n1){
	GLint	location;
	location = glGetUniformLocationARB(p, pszParameter); 
	//printf("%s location = %d, val = %d\n", pszParameter, (int)location, n1);
	glUniform1iARB(location, n1);
}

void SetUniformParameter1f(GLhandleARB p, const char *pszParameter, float p1){
	GLint	location;
	location = glGetUniformLocationARB(p, pszParameter);
	//printf("%s location = %d, val = %f\n", pszParameter, (int)location, p1);
	glUniform1fARB(location, p1);
}

void SetUniformParameter3f(GLhandleARB p, const char *pszParameter, float p1, float p2, float p3){
	GLint	location;
	location = glGetUniformLocationARB(p, pszParameter);
	//printf("%s location = %d, val = %f %f %f\n", pszParameter, (int)location, p1, p2, p3);
	glUniform3fARB(location, p1, p2, p3);
}



void update_scene(void){ 
	
	
	int		fdin;
	unsigned char	*src;
	struct stat statbuf;
	
	static int	time = 0;
	static int	beenHere = 0;
	
	static int this_point = 0;
	static int next_point = 1;
	
	char		fname[1024];
	//pngInfo infoLayer;
	
	static int	skip;
	static double	date;
	int		mm, dd, yy, hh, min;
	double	sec;
	
	double	stime, ftime;
	
	const char *month_name[] = { "", "January", "February", "March", 
		"April", "May", "June", "July", "August", "September", "October",
		"November", "December" };
	
	if( beenHere == 0){
		date = E->start_date;
		
		if(E->preview == 1)
			skip = 1;
		else
			skip = 1;
		
		beenHere = 1;
		
	}
	
	if(( (time % skip) == 0) ){
		
		 glDeleteTextures(1, &E->texture[E->numLayers-1]);
		 
		 //sprintf(fname,"OFAM3/sea_surface_temperature/img/%.06f.png",date);
		
		 if(E->preview == 1)
			 sprintf(fname,"%s/%s/img/%.6f_scaled.jpg",E->texture_archive_path, E->standard_name, date);
		 else
			 sprintf(fname,"%s/%s/img/%.6f.jpg",E->texture_archive_path, E->standard_name, date);
		
		
		 // convert julian date to calendar date
		 caldat(date, &mm, &E->day, &yy, &hh, &min, &sec);
		 sprintf(E->date_string,"%s %04d\n", month_name[mm], yy);
		 
		/*
		 E->texture[E->numLayers-1] = pngBind(fname, 
		 PNG_BUILDMIPMAPS, 
		 PNG_SOLID, 
		 &infoLayer, 
		 GL_CLAMP_TO_EDGE, 
		 GL_LINEAR_MIPMAP_LINEAR, 
		 GL_LINEAR_MIPMAP_LINEAR);
		 */
		
		
		//printf("frame: %d, texture = %s\n", time, fname);
		stime = getSeconds();
		E->texture[E->numLayers-1] = SOIL_load_OGL_texture(
							   fname,
							   SOIL_LOAD_RGB,
							   E->texture[E->numLayers-1],
							   SOIL_FLAG_INVERT_Y 
							   );
		ftime = getSeconds();
		printf("texture load time = %f\n", ftime-stime);
		
		/*
		// and now read it
		E->texture[E->numLayers-1] = SOIL_load_OGL_texture_from_memory(
						src,
						statbuf.st_size,
						SOIL_LOAD_RGB,
						E->texture[E->numLayers-1],
						SOIL_FLAG_INVERT_Y 
		 );
		*/
		
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		 
		 if (E->texture[E->numLayers-1] == 0) {
			 fprintf(stderr,"Can't load file: %s\n",fname);
			 fflush(stderr);
			 exit(1);
		 }	
		 else{
			 //date = date + 1.0;  
			 date = date + skip;
			 
		 }
	 }
	
	
	if(time<E->pts[next_point].duration){
		
		// longitude
		E->theta[1] = splint(E->points, E->lon_uw, E->lon_spline,E->npts,(double)this_point + (double)time/E->pts[next_point].duration  ); 
		
		// latitude
		E->theta[0] = splint(E->points, E->lat, E->lat_spline,E->npts,(double)this_point +(double)time/E->pts[next_point].duration );
		
		// zoom
		E->viewer[2] = splint(E->points, E->zoom, E->zoom_spline,E->npts,(double)this_point +(double)time/E->pts[next_point].duration  );
				
		time++;
	}
	else{
		this_point++;
		next_point++;
		time=0;
	}
	
	
}




void update_scene_with_texture_cache(int scene){ 
	
	
	static int	time = 0;
	static int	beenHere = 0;
	
	static int this_point = 0;
	static int next_point = 1;
		
	static int	skip;
	static double	date;
	int		mm, yy, hh, min;
	double	sec;
	
	
	double	stime, ftime;
	
	const char *month_name[] = { "", "January", "February", "March", 
		"April", "May", "June", "July", "August", "September", "October",
		"November", "December" };
	
	if( beenHere == 0){
		date = E->start_date;
		
		if(E->preview == 1)
			skip = 1;
		else
			skip = 1;
		
		beenHere = 1;
		
	}
	
	if(( (time % skip) == 0) ){
						
		// convert julian date to calendar date
		caldat(date, &mm, &E->day, &yy, &hh, &min, &sec);
		sprintf(E->date_string,"%s %04d\n", month_name[mm], yy);
			
		//stime = getSeconds();
		
		E->im[scene] = SOIL_create_OGL_texture( 
						E->mem_image[scene], E->im_width[scene], E->im_height[scene], E->im_channels[scene], 
						SOIL_CREATE_NEW_ID, 
						SOIL_FLAG_INVERT_Y);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			
		
		
		//ftime = getSeconds();
		//printf("texture load time = %f\n", ftime-stime);
		
		if (E->im[scene] == 0) {
			fprintf(stderr,"failed to create ogl texture unit\n");
			fflush(stderr);
			exit(1);
		}	
		else{

			date = date + skip;
			
			// free the image data
			free( E->mem_image[scene] );
			// unmap the image
			if (munmap(E->im_cache[scene], E->im_size[scene]) == -1) {
				printf("update_scene: Error un-mmapping the image file\n");
				exit(1);
			}
			
			
		}
	    
	}
	
	if(time<E->pts[next_point].duration){
		
		// longitude
		E->theta[1] = splint(E->points, E->lon_uw, E->lon_spline,E->npts,(double)this_point + (double)time/E->pts[next_point].duration  ); 
		
		// latitude
		E->theta[0] = splint(E->points, E->lat, E->lat_spline,E->npts,(double)this_point +(double)time/E->pts[next_point].duration );
		
		// zoom
		E->viewer[2] = splint(E->points, E->zoom, E->zoom_spline,E->npts,(double)this_point +(double)time/E->pts[next_point].duration  );
		
		time++;
	}
	else{
		this_point++;
		next_point++;
		time=0;
	}	
}



void setupOpenGLEnvironment(void){
	int	i;
	
	GLfloat  	ambient_light[4];
	GLfloat   	source_light[4];
	
	GLfloat		mat_specular[4];
	GLfloat		mat_shininess[1];
	
	
	float 	light_magnitude;
	char    typeface[2048];
	
	// set up initial values for the E struct
	
	// apply the lon_offset to our view
	//printf("start lon = %f\n", E->theta[1]);
	E->theta[1] = E->theta[1] + E->lon_offset;
	//printf("start lon = %f\n", E->theta[1]);
	//E->theta[1] -= 115.0;
	
	//E->DRAW_ALL=78;
	//E->vis = GLUT_VISIBLE;
	
	E->viewer[0] = 0.0; 
	E->viewer[1] = 0.0; 
	
	//E->center[0] = -0.2;
	//E->center[1] = 0.31; // up/down
	//E->center[2] = 0.0;
	
	E->center[0] = 0.0;
	E->center[1] = 0.0; // up/down
	E->center[2] = 0.0;
	
	
	//ambient_light[0] = 0.5;
	//ambient_light[1] = 0.55;
	//ambient_light[2] = 0.75;
	//ambient_light[3] = 1.0;
	ambient_light[0] = 0.0;
	ambient_light[1] = 0.0;
	ambient_light[2] = 0.0;
	ambient_light[3] = 1.0;
	
	source_light[0] = 1.0;
	source_light[1] = 1.0;
	source_light[2] = 1.0;
	source_light[3] = 1.0;
	
	
	// original - without shaders
	E->light_pos[0] = 2.0;
	E->light_pos[1] = 0.0;
	E->light_pos[2] = 6;
	E->light_pos[3] = 1.0;
	
	// new with shaders
	//E->light_pos[0] = 3.0;
	//E->light_pos[1] = 0.0;
	//E->light_pos[2] = 6.0;
	//E->light_pos[3] = 1.0;
	
	// testing for sky sphere
	//E->light_pos[0] = -2.0;
	//E->light_pos[1] = 0.0;
	//E->light_pos[2] = 6;	// (-20 dark yellow ... 20 to -20 looks good)
	//E->light_pos[3] = 1.0;	// not used so why is it here??
	
	light_magnitude = sqrt(E->light_pos[0]*E->light_pos[0] +
						   E->light_pos[1]*E->light_pos[1] +
						   E->light_pos[2]*E->light_pos[2]); 
	
	E->light_direction[0] = E->light_pos[0]/light_magnitude; 
	E->light_direction[1] = E->light_pos[1]/light_magnitude; 
	E->light_direction[2] = E->light_pos[2]/light_magnitude; 
	
	//GLfloat mat_red_diffuse[] = { 0.7, 0.0, 0.1, 1.0 };
	//GLfloat mat_green_diffuse[] = { 0.0, 0.7, 0.1, 1.0 };
	//GLfloat mat_blue_diffuse[] = { 0.0, 0.1, 0.7, 1.0 };
	//GLfloat mat_yellow_diffuse[] = { 0.7, 0.8, 0.1, 1.0 };
	mat_specular[0] = 1.0;
	mat_specular[1] = 1.0;
	mat_specular[2] = 1.0;
	mat_specular[3] = 1.0;
	mat_shininess[0] = 128.0;
	
	/*
	 // originals - these are the default set so you should use them
	 // shader variables
	 m_nSamples = 20;		// Number of sample rays to use in integral equation
	 m_Kr = 0.0025f;		// Rayleigh scattering constant
	 m_Kr4PI = m_Kr*4.0f*PI;
	 m_Km = 0.0015f;		// Mie scattering constant
	 m_Km4PI = m_Km*4.0f*PI;
	 m_ESun = 8.0f;		// was 10 // Sun brightness constant
	 m_g = -0.85f;		// The Mie phase asymmetry factor
	 
	 m_fInnerRadius = 1.0f;
	 m_fOuterRadius = 1.025f;
	 m_fScale = 1.0f / (m_fOuterRadius - m_fInnerRadius);
	 
	 m_fWavelength[0] = 0.650f;		// 650 nm for red
	 m_fWavelength[1] = 0.570f;		// 570 nm for green
	 m_fWavelength[2] = 0.475f;		// 475 nm for blue
	 
	 m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
	 m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
	 m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);
	 
	 m_fRayleighScaleDepth = 0.25f;
	 m_fMieScaleDepth = 0.1f;
	 */
	
	// shader variables
	m_nSamples = 16;		// Number of sample rays to use in integral equation
	m_Kr = 0.0035f;		// Rayleigh scattering constant
	m_Kr4PI = m_Kr*4.0f*PI;
	m_Km = 0.0015f;		// Mie scattering constant
	m_Km4PI = m_Km*4.0f*PI;
	m_ESun = 6.0f;		// was 10 // Sun brightness constant
	m_g = -0.85f;		// The Mie phase asymmetry factor
	m_g2 = m_g*m_g;
	
	m_fInnerRadius = 1.0f;
	m_fInnerRadius2 = m_fInnerRadius*m_fInnerRadius;
	m_fOuterRadius = 1.025f;
	m_fOuterRadius2 = m_fOuterRadius*m_fOuterRadius;
	m_fScale = 1.0f / (m_fOuterRadius - m_fInnerRadius);
	
	
	 m_fWavelength[0] = 0.650f;		// 650 nm for red
	 m_fWavelength[1] = 0.590f;		// 570 nm for green
	 m_fWavelength[2] = 0.475f;		// 475 nm for blue
	
	
	/*
	// ...white atmosphere...
	m_fWavelength[0] = 0.620f;		// 650 nm for red
	m_fWavelength[1] = 0.620f;		// 570 nm for green
	m_fWavelength[2] = 0.620f;		// 475 nm for blue
	*/
	
	m_fWavelength4[0] = powf(m_fWavelength[0], 4.0f);
	m_fWavelength4[1] = powf(m_fWavelength[1], 4.0f);
	m_fWavelength4[2] = powf(m_fWavelength[2], 4.0f);
	
	m_fWavelength4_inv[0] = 1.0/m_fWavelength4[0];
	m_fWavelength4_inv[1] = 1.0/m_fWavelength4[1];
	m_fWavelength4_inv[2] = 1.0/m_fWavelength4[2];
	
	m_fRayleighScaleDepth = 0.25f;
	m_fMieScaleDepth = 0.1f;
	
	m_fScaleOverScaleDepth = m_fScale / m_fRayleighScaleDepth;
	
	// Enable Smooth Shading
    //glShadeModel(GL_SMOOTH);            
    
    // Depth Buffer Setup
	glClearDepth(1.0f);          
	
	// Enables Depth Testing
    glEnable(GL_DEPTH_TEST);           
    
    // The Type Of Depth Testing To Do
    glDepthFunc(GL_LEQUAL);                 
    
    // Perspective Calculations
    //glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);      
	//glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	
	//glEnable( GL_LINE_SMOOTH);
	//glEnable( GL_POINT_SMOOTH);
	//glEnable( GL_POLYGON_SMOOTH);
	
	
	//lighting model
	//glEnable ( GL_LIGHTING );
	
	/*
   	glLightModelfv ( GL_LIGHT_MODEL_AMBIENT, ambient_light );
   	glLightfv (GL_LIGHT0,GL_DIFFUSE, source_light );
   	glLightfv ( GL_LIGHT0,GL_POSITION, E->light_pos  );
   	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
   	glEnable  ( GL_LIGHT0 );
   	*/
	
   	// Enable material properties for lighting
   	//glEnable ( GL_COLOR_MATERIAL );
   	//glColorMaterial ( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );
	
	// Enable shininess
	//glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	//glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	
	//Enable texture mapping
	//glEnable(GL_TEXTURE_2D);		
	//glTexEnvf ( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glEnable(GL_CULL_FACE); 
	//glColor3f(1.0, 1.0, 1.0);
	
	// backgorund color - if not present in the input file
	// defaults to black background
   	//glClearColor (0.0, 0.0, 0.0, 0.0);  // black
    glClearColor(E->bg.red,E->bg.green, E->bg.blue, 0.0);
   	//glClearColor (1.0, 1.0, 1.0, 0.0);  // white
   	//glClearAccum(0.0, 0.0, 0.0, 0.0);
	
	
	// load the initial texture maps
	loadTextureMaps(E);
	
	
	// set up fps
	E->fps = 24;
	//printf("fps = %d\n", E->fps);
	
	// interp camera points
	E->lat = malloc(E->npts*sizeof(double));
	E->lon = malloc(E->npts*sizeof(double));
	E->lon_uw = malloc(E->npts*sizeof(double));
	E->points = malloc(E->npts*sizeof(double));
	E->zoom = malloc(E->npts*sizeof(double));
	
	for(i=0;i<E->npts;i++){
		E->points[i] = (double)i;
		//E->pts[i].duration*=(double)E->fps;
		E->lat[i] = E->pts[i].pt[0];
		E->lon[i] = E->pts[i].pt[1] - E->lon_offset;
		E->zoom[i] = E->pts[i].zoom;
		//printf("duration[%d] = %f\n",i, E->pts[i].duration);
	}
	//E->totalDuration *= E->fps;
	
	//lons_wrapped(E->lon, E->npts, E->lon_uw);
	lons_wrapped_orig(E->lon, E->npts, E->lon_uw);
	
	E->lat_spline = malloc(E->npts*sizeof(double));
	E->lon_spline = malloc(E->npts*sizeof(double));
	E->zoom_spline = malloc(E->npts*sizeof(double));
	
	spline(E->points, E->lat, E->npts, 0.0, 0.0, &E->lat_spline[0]);
	//spline(E->points, E->lon, E->npts, 0.0, 0.0, &E->lon_spline[0]);
	spline(E->points, E->lon_uw, E->npts, 0.0, 0.0, &E->lon_spline[0]);
	spline(E->points, E->zoom, E->npts, 0.0, 0.0, &E->zoom_spline[0]);
	
	//printf("total frames = %d\n", E->totalDuration);
		
	
	// init freetype-gl
	if(E->preview == 1)
		E->font_size = 20;
	else
		E->font_size = 60;
	
	//printf("using: %s/%s\n", E->font_path, E->font_name);
	sprintf(typeface,"%s/%s", E->font_path, E->font_name);
	//printf("typeface = %s\n", typeface);
	
	atlas  = texture_atlas_new( 512, 512, 1 );
	
	//sprintf(typeface,"%s/League_Gothic.ttf", E->font_path);
	//printf("typeface = %s\n", typeface);
	//font = texture_font_new( atlas, "fonts/League_Gothic.ttf", E->font_size );
	font = texture_font_new( atlas, typeface, E->font_size );
	buffer = vertex_buffer_new( "v3f:t2f:c4f" ); 
	
	if(E->preview == 1)
		E->font_size_small = 18;
	else
		E->font_size_small = 30;
	atlas_small  = texture_atlas_new( 512, 512, 1 );
	font_small = texture_font_new( atlas_small, typeface, E->font_size_small );
	buffer_small = vertex_buffer_new( "v3f:t2f:c4f" ); // vertex = 3f, tex coord = 2f color = 4f!
	
	// initialize the texture cache
	init_texture_cache(E, E->totalDuration);
	populate_texture_mmap_cache(E);
	//printf("done texture cache\n");
	
}









