#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h> /* mmap() is defined in this header */
#include <fcntl.h>
#include <unistd.h>

#include <pthread.h>

#include <time.h>


//#include "/opt/local/include/GL/glew.h"

// glut include
//#ifdef _OS_X_
//#include </System/Library/Frameworks/GLUT.framework/Headers/glut.h>
//#include <GLUT/glut.h>
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
//#endif

//#ifdef _LINUX_
//#include <GL/gl.h>
//#include <GL/glut.h>
//#endif


// libxml2 includes
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

// libpng include
#include <png.h>

// freetype-gl headers
#include "freetype-gl.h"
#include "vertex-buffer.h"

#include "loadTexture.h"
#include "SOIL.h"

#include "jutil.h"

#include "libavutil/mathematics.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

// defines
#define PI 3.14159265358979323846
#define TWOPI	(2.0*PI)
#define PID2	(PI/2.0)
#define TRUE 1
#define FALSE 0

#define	TOP_LEFT	0
#define	TOP_RIGHT	1
#define	BOTTOM_LEFT	2
#define	BOTTOM_RIGHT	3


#define toDeg   (180.0/PI)
#define toRad   (PI/180.0)

#define space_	(10)
#define	VertexCount	((90 / space_) * (360 / space_) * 4)


#define RES 400

#define MAX_LAYERS	32
#define MAX_SEQUENCE	1024


typedef struct {
	int thread_id;
	int	scene_num;
} param;

typedef struct{
	double	x;
	double	y;
	double	z;
}XYZ;

typedef struct {
	int X;
    int Y;
    int Z;
    double U;
    double V;
}VERTICES;

typedef struct {
    float x, y, z;
} point;



typedef struct {
	float	red;
	float	green;
	float	blue;
}Colors;

typedef struct{
	int		haveWMS;
	char	url[2048];
	char	layer[1024];
	char	colormap[1024];
	char	wms_request[2048];
	double		lon_min;
	double		lon_max;
	double		lat_min;
	double		lat_max;
	int		width;
	int		height;
	double	min;
	double	max;
	
}WMS;

typedef struct {
	char		*name;
	float		start;
	float		end;
	char		textureMap[1024];
	WMS			wms;
	Colors		colors;
	int			animate;
	int			frames;
	
}layers;

typedef struct {
	float	pt[3];
	float	zoom;
	float	duration;
	float	direction;
}pt;

// struct definition
typedef struct {
	int	DRAW_ALL;
	GLfloat theta[3];
	GLdouble viewer[3];
	GLfloat	  center[3];
	GLfloat   light_pos[4];
	GLfloat	  light_direction[3];
	
	int			npts;
	int			totalDuration;
	int			fps;

	GLint xWinSize;
	GLint yWinSize;

	int startx, starty;
	
	GLuint  texture[MAX_LAYERS]; // current limit is 6 texture maps

	GLint vis;
	GLint action;
	
	char	*shader_path;
	char	*font_path;
	char	*font_name;
	char	*output_path;
	char	*progress_file;
	
	int		font_size;
	int		font_size_small;
	char	*texture_archive_path;
	double	start_date;
	
	
	// params red in from file
	int		numLayers;
	int		what_to_draw;
	int		what_to_draw_core;
	layers	L[MAX_LAYERS];
	pt		pts[MAX_SEQUENCE];
	Colors	bg;
		
	double	lon_offset;
	
	// pthreads
	int	n_threads;
	
	// text titles derived from input XML
	char		*standard_name;
	char		*long_name;
	char		*date_string;
	int			day;
	int			preview;
	
	// interp params
	double	*lat;
	double	*lon;
	double	*lon_uw;
	double	*zoom;
	double	*points;
	double	*lat_spline;
	double	*lon_spline;
	double	*zoom_spline;
	
	// texture mem cache 
	unsigned char	**im_cache;
	unsigned char	**mem_image;
	off_t			*im_size;
	GLuint			*im;
	int				*im_width;
	int				*im_height;
	int				*im_channels;
		
	// shader params
	GLhandleARB v;
	GLhandleARB f;
	GLhandleARB ground;
	GLhandleARB atmo;
	GLhandleARB contrast;
	GLhandleARB	texBlend;
	GLhandleARB	bump_mappr;
	GLhandleARB phong;
	
}e;

// freetype-gl globals
typedef struct {
    float x, y, z;    // position
    float s, t;       // texture
    float r, g, b, a; // color
} vertex_t;


texture_atlas_t *atlas;
texture_atlas_t *atlas_small;
texture_font_t *font;
texture_font_t *font_small;
vertex_buffer_t *buffer;
vertex_buffer_t *buffer_small;

// E is a global
e *E;


// shader vars
VERTICES VERTEX[VertexCount];

// Lightpos in Object Space

int m_nSamples;
float m_Kr, m_Kr4PI;
float m_Km, m_Km4PI;
float m_ESun;
float m_g;
float m_g2;

float m_fInnerRadius;
float m_fInnerRadius2;

float m_fOuterRadius;
float m_fOuterRadius2;

float m_fScale;
float m_fWavelength[3];
float m_fWavelength4[3];
float m_fWavelength4_inv[3];
float m_fRayleighScaleDepth;
float m_fMieScaleDepth;
float m_fScaleOverScaleDepth;

// prototypes

void Transform(GLfloat Width, GLfloat Height);
void myReshape( int w, int h );

void WindowDump_PPM(void);
void WindowDump_PNG(void);

void draw(e *E, unsigned int scene);
void drawFloor(void);

char *lr_pack( char *c );

void getparams_xml(char* parameter_filename, e *E);
void parseInputFile_params (e *E, xmlDocPtr doc, xmlNodePtr cur);
void parseInputFile_layers (e *E, xmlDocPtr doc, xmlNodePtr cur, xmlChar *txt);
void parse_WMS_params (e *E, xmlDocPtr doc, xmlNodePtr cur);
void parseInputFile_motion (e *E, xmlDocPtr doc, xmlNodePtr cur);

void loadTextureMaps(e	*E);

void setDefaults(e *E);

void write_xml(e *E);

float Determinant4f(const float m[16]);
int		GenerateInverseMatrix4f(float i[16], const float m[16]);

void		Square (void);
void		world_billboard (void);

void setShaders(e *E);
void shaderEnable(GLhandleARB p);
void shaderDisable(GLhandleARB p);
void SetUniformParameter1i(GLhandleARB p, const char *pszParameter, int n1);
void SetUniformParameter1f(GLhandleARB p, const char *pszParameter, float p1);
void SetUniformParameter3f(GLhandleARB p, const char *pszParameter, float p1, float p2, float p3);

char *textFileRead(char *fn);
int textFileWrite(char *fn, char *s);

void DrawSphere(e *E, int n, double xc, double yc, double zc, double r);
void DrawSphereII(e *E, double xc, double yc, double zc, double r);
void DrawSphere_invert_normals(e *E, int n, double xc, double yc, double zc, double r);

void draw_background(void);

void drawPoint(e *E, float r);
void Draw_Axes(void);


void spline(double *x, double *y, int n, double yp1, double ypn, double *y2);
double splint(double *xa, double *ya, double *y2a, int n, double x);
void lons_wrapped(double *lons, int n, double *lons_uw);
void lons_wrapped_orig(double *lons, int n, double *lons_uw);

void mencode_frames();

// freetype-gl prototypes
void add_text( vertex_buffer_t * buffer, texture_font_t * font,wchar_t * text, vec4 * color, vec2 * pen );
void addText(int x, int y, char *str);

// delete this once you fix up the font rendering
void addText2(int x, int y, char *str);
void draw_logo(e *E, int	position, int	logo);
void draw_colorbar(e *E, int colorbar);




// no window functions
void update_scene(void);
void setupOpenGLEnvironment(void);



// image conversion functions
void get_yuv_from_rgb(e *E, AVFrame *picture);


// texture cache
void init_texture_cache(e *E, unsigned int size);
void populate_texture_mmap_cache(e *E);
void update_scene_with_texture_cache(int scene);
void free_texture_cache(e *E, unsigned int size);


double getSeconds();

void *load_texture_maps(void *arg);
void update_texture_cache(e *E, int scene);

int	get_int(int id);


