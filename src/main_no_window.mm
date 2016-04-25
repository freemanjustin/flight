// to build:
// g++ -o test main.mm -lobjc -framework AppKit -framework OpenGL

// to build with slicer (in the src directory)
//g++ -c main_no_window.mm  -D_OS_X_ -I../include -I../freetype-gl -I/Users/jfreeman/libpng-1.4.1/include `freetype-config --cflags` 
// -I/Users/jfreeman/slicerTiled_1.2/src/SOIL/SOIL/src `xml2-config --cflags`

// then to build all of slicer:
/*
 g++ -O2 -g -Wall `/usr/bin/xml2-config --cflags` -D_OS_X_ -I./include -I./freetype-gl -I/Users/jfreeman/libpng-1.4.1/include 
`freetype-config --cflags` -I/Users/jfreeman/slicerTiled_1.2/src/SOIL/SOIL/src          
-o ./bin/slicer ./src/openglSetup.o ./src/openglMisc.o ./src/draw.o ./src/loadTexture.o 
./src/getparams.o ./src/textfile.o ./src/sphere.o ./src/interp_1d.o ./src/mencode.o ./src/type.o 
./freetype-gl/vertex-buffer.o ./freetype-gl/vertex-attribute.o ./freetype-gl/vector.o ./freetype-gl/texture-atlas.o 
./freetype-gl/texture-font.o ./src/main_no_window.o -framework Foundation 
-framework OpenGL -framework GLUT -L/Users/jfreeman/libpng-1.4.1/lib -lpng `/usr/bin/xml2-config --libs` 
`freetype-config --libs` ./src/SOIL/SOIL/lib/libsoil.a -lobjc -framework AppKit
*/

//#define kOutputFile "test_image.png"


#include <iostream>         // for error output
#import <OpenGL/OpenGL.h>
#import <OpenGL/glu.h>      // for gluCheckExtension
#import <AppKit/AppKit.h>   // for NSOpenGL...

extern "C" {
	#include "slicer.h"
}

// Simple error reporting macros to help keep the sample code clean
#define REPORTGLERROR(task) { GLenum tGLErr = glGetError(); if (tGLErr != GL_NO_ERROR) { std::cout << "OpenGL error " << tGLErr << " while " << task << "\n"; } }
#define REPORT_ERROR_AND_EXIT(desc) { std::cout << desc << "\n"; return 1; }
#define NULL_ERROR_EXIT(test, desc) { if (!test) REPORT_ERROR_AND_EXIT(desc); }

int main (int argc, char * const argv[])
{
	NSAutoreleasePool*			pool = [NSAutoreleasePool new];
	
	
    char    parameter_filename[256];
    FILE    *input_stream;
    int 	j;
    
	E = (e*)malloc(sizeof(e));
    if(E == NULL){
        fprintf(stderr,"Not enough memory for structs\n");
        exit(1);
    }
	
	
	if ( argc < 2 ){
		fprintf(stderr, "%s: need an xml file as an argument\n", argv[0] );
		exit(1);
    }
	
	
	// handle the command line args here:
    for (j=1 ; j<argc ; j++){
        // If this is the last argument then it must be the file to process
        if (j == 1){
            if ( (input_stream = fopen( argv[j], "r" )) == NULL ){
                fprintf(stderr,"could not open input file : %s\n", argv[j]);
                exit(1);
            }
            else{   // Always copy the good file name and close the file
                strcpy(parameter_filename, argv[j]);
                fclose(input_stream);
            }
        }
    }
	
    getparams_xml(parameter_filename, E);
    
	//glutInit( &argc, argv );
	
	
	//glutMainLoop();
	
    /*
     * Create an OpenGL context just so that OpenGL calls will work. I'm not using it for actual rendering.
     */
	
	NSOpenGLPixelBuffer*		pixBuf;
	NSOpenGLContext*			openGLContext;
	NSOpenGLPixelFormatAttribute	attributes[] = {
		NSOpenGLPFAPixelBuffer,
		NSOpenGLPFANoRecovery,
		NSOpenGLPFAAccelerated,
		NSOpenGLPFADepthSize, 24,
		(NSOpenGLPixelFormatAttribute) 0
	};
	NSOpenGLPixelFormat*		pixFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];

    // Create an OpenGL pixel buffer
    pixBuf = [[NSOpenGLPixelBuffer alloc] initWithTextureTarget:GL_TEXTURE_RECTANGLE_EXT textureInternalFormat:GL_RGBA textureMaxMipMapLevel:0 pixelsWide:32 pixelsHigh:32];
    NULL_ERROR_EXIT(pixBuf, "Unable to create NSOpenGLPixelBuffer");
	
    // Create the OpenGL context to render with (with color and depth buffers)
    openGLContext = [[NSOpenGLContext alloc] initWithFormat:pixFormat shareContext:nil];
    NULL_ERROR_EXIT(openGLContext, "Unable to create NSOpenGLContext");
	
    [openGLContext setPixelBuffer:pixBuf cubeMapFace:0 mipMapLevel:0 currentVirtualScreen:[openGLContext currentVirtualScreen]];
    [openGLContext makeCurrentContext];
	
    /*
     * Test if framebuffer objects are supported
     */
	
    const GLubyte* strExt = glGetString(GL_EXTENSIONS);
    GLboolean fboSupported = gluCheckExtension((const GLubyte*)"GL_EXT_framebuffer_object", strExt);
    if (!fboSupported)
        REPORT_ERROR_AND_EXIT("Your system does not support framebuffer extension - unable to render scene");
	
    /*
     * Create an FBO
     */
	
    GLuint  renderBuffer = 0;
    GLuint  depthBuffer = 0;
    int     img_width = 4000, img_height = 4000; // <-- pixel size of the rendered scene - hardcoded values for testing
	
	img_width = E->xWinSize;
	img_height = E->yWinSize;
	
    // Depth buffer to use for depth testing - optional if you're not using depth testing
    glGenRenderbuffersEXT(1, &depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, img_width, img_height);
    REPORTGLERROR("creating depth render buffer");
	
    // Render buffer to use for imaging
    glGenRenderbuffersEXT(1, &renderBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA8, img_width, img_height);
    REPORTGLERROR("creating color render buffer");
	
    GLuint  fbo = 0;
    glGenFramebuffersEXT(1, &fbo);
	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
    REPORTGLERROR("binding framebuffer");
	
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, renderBuffer);
    REPORTGLERROR("specifying color render buffer");
	
    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        REPORT_ERROR_AND_EXIT("Problem with OpenGL framebuffer after specifying color render buffer.");
	
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);
    REPORTGLERROR("specifying depth render buffer");
	
    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
        REPORT_ERROR_AND_EXIT("Problem with OpenGL framebuffer after specifying depth render buffer.");
	
    /*
     * Render a simple shape to the FBO
     */
	
	char kOutputFile[80];
	int i = 0;
    float x = 10.0;
    float y = 10.0;
    
    int samplesPerPixel = 4; // R, G, B and A
    int rowBytes = samplesPerPixel * img_width;
    char* bufferData = (char*)malloc(rowBytes * img_height);
    NULL_ERROR_EXIT(bufferData, "Unable to allocate buffer for image extraction.");
	// Flip it vertically - images read from OpenGL buffers are upside-down
    char* flippedBuffer = (char*)malloc(rowBytes * img_height);
    NULL_ERROR_EXIT(flippedBuffer, "Unable to allocate flipped buffer for corrected image.");
	
	
	CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
	CGBitmapInfo bitmapInfo = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little;	// XRGB Little Endian
	int bitsPerComponent = 8;
	
	Boolean isDirectory = false;
	
	CFIndex capacity = 1;
	
	setupOpenGLEnvironment2();
	setShaders(E);
	
	
	for(i=0;i<1000;i++){
		idle2();
		sprintf(kOutputFile,"window%04d.jpg",i);
		
		//glEnable(GL_DEPTH_TEST);
		//REPORTGLERROR("enabling depth testing");
		
		//glClearColor(0.0, 0.0, 0.0, 1.0);
		//glClearColor(E->bg.red,E->bg.green, E->bg.blue, 0.0);
		//REPORTGLERROR("specifying clear color");
		
		glViewport(0, 0, img_width, img_height);
		//REPORTGLERROR("specifying viewport");
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//glOrtho(-img_width / 2, img_width / 2, -img_height / 2, img_height / 2, -1, 1);
		myReshape( img_width, img_height );
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//REPORTGLERROR("setting up view/model matrices");
		
		glLoadIdentity();
		
		gluLookAt( E->viewer[0], E->viewer[1], E->viewer[2],  
				  0.0, 0.0, 0.0, 	// center is at (0,0,0)
				  0.0, 1.0, 0.0 ); // up is in +ve Y direction
		
		glClear(GL_COLOR_BUFFER_BIT);
		//REPORTGLERROR("clearing color buffer");
		
		glClear(GL_DEPTH_BUFFER_BIT);
		//REPORTGLERROR("clearing depth buffer");
		
		/*
		glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3f(0.0, 400.0+y, 0.0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3f(200.0+x, -200.0-y, 0.0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3f(-200.0-x, -200.0-y, 0.0);
		glEnd();
		//REPORTGLERROR("rendering scene");
        x += 50.0;
        y += 50.0;
		*/
		
		draw();
		
		 // Extract the resulting rendering as an image
		
		
		
		
		glReadPixels(0, 0, img_width, img_height, GL_BGRA, GL_UNSIGNED_BYTE, bufferData);
		//REPORTGLERROR("reading pixels from framebuffer");
		
		for (int i = 0 ; i < img_height ; i++)
		{
			bcopy(bufferData + i * rowBytes, flippedBuffer + (img_height - i - 1) * rowBytes, rowBytes);
		}
		
		
		
		
		 // Output the image to a file
		 		
		CGContextRef contextRef = CGBitmapContextCreate(flippedBuffer,
														img_width, img_height, bitsPerComponent, rowBytes, colorSpace, bitmapInfo);
		NULL_ERROR_EXIT(contextRef, "Unable to create CGContextRef.");
		
		CGImageRef imageRef = CGBitmapContextCreateImage(contextRef);
		NULL_ERROR_EXIT(imageRef, "Unable to create CGImageRef.");
		
		
		
		
		CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault,
														 CFStringCreateWithCString (NULL,kOutputFile,kCFStringEncodingUTF8), kCFURLPOSIXPathStyle, isDirectory);
		
		
		NULL_ERROR_EXIT(fileURL, "Unable to create file URL ref.");
		
		CFIndex                 fileImageIndex = 1;
		CFMutableDictionaryRef  fileDict       = NULL;
		CFStringRef             fileUTType     = kUTTypeJPEG; //kUTTypePNG; //kUTTypeJPEG;
		
		// Create an image destination opaque reference for authoring an image file
		CGImageDestinationRef imageDest = CGImageDestinationCreateWithURL(fileURL,
																		  fileUTType,
																		  fileImageIndex,
																		  fileDict);
		
		NULL_ERROR_EXIT(imageDest, "Unable to create CGImageDestinationRef.");
		
		
		CFMutableDictionaryRef imageProps = CFDictionaryCreateMutable(kCFAllocatorDefault,
																	  capacity,
																	  &kCFTypeDictionaryKeyCallBacks,
																	  &kCFTypeDictionaryValueCallBacks);
		
		CGImageDestinationAddImage(imageDest, imageRef, imageProps);
		CGImageDestinationFinalize(imageDest);
		
		
		
		CFRelease(imageDest);
		CFRelease(fileURL);
		CFRelease(imageProps);
		
		CGImageRelease(imageRef);
		
	}
    /*
     * Cleanup
     */
    
    free(flippedBuffer);
    free(bufferData);
    
	CGColorSpaceRelease( colorSpace );
	
    // "un"bind my FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
	
    [openGLContext clearDrawable];
    [openGLContext release];
    [pixBuf release];
	
	[pool release];
	
    return 0;
}

