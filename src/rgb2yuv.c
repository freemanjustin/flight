#include "slicer.h"


void get_yuv_from_rgb(e *E, AVFrame *pict){
	
	static int					beenHere = 0;
	static struct SwsContext	*img_convert_ctx;
	static unsigned char		*image;
	int							rowBytes;
	int							samplesPerPixel = 3;
	AVFrame						*RGBpict;
	
	//int	numBytes;
	//unsigned int	*buffer;
	
	rowBytes = samplesPerPixel * E->xWinSize;
	
	if(beenHere == 0){
		// Allocate our buffer for the image
		if ((image = malloc(E->yWinSize*rowBytes*sizeof(unsigned char)) ) == NULL) {
			fprintf(stderr,"get_yuv: Failed to allocate memory for image \n");
			return;
		}
		
		img_convert_ctx = sws_getContext( E->xWinSize, E->yWinSize, PIX_FMT_BGR24,
										  E->xWinSize, E->yWinSize, PIX_FMT_YUV420P,
										  SWS_FAST_BILINEAR, NULL, NULL, NULL );
		
		if( img_convert_ctx == NULL ) {
			fprintf( stderr, "can't initialize scaler context" );
			exit(1);
		}
		
		beenHere = 1;
	}	
	
	RGBpict = avcodec_alloc_frame();
	//numBytes=avpicture_get_size(PIX_FMT_YUV420P, E->xWinSize,E->yWinSize);
    //buffer=malloc( numBytes * sizeof(unsigned int));
	//avpicture_fill((AVPicture *)RGBpict, buffer, PIX_FMT_YUV420P,E->xWinSize,E->yWinSize);
	
	
	RGBpict->data[0] = image;
	RGBpict->data[1] = image;
	RGBpict->data[2] = image;
	RGBpict->linesize[0] = rowBytes;
	RGBpict->linesize[1] = rowBytes;
	RGBpict->linesize[2] = rowBytes;
	
	// this will flip the image in the sws_scale() call below
	RGBpict->data[0] += RGBpict->linesize[0]*(E->yWinSize-1); 
	RGBpict->linesize[0] = -RGBpict->linesize[0]; 
	
	// if you start getting weird output behavior then change the GL_PACK_ALIGNMENT back to 1:
	// glPixelStorei(GL_PACK_ALIGNMENT,1);
	
	glPixelStorei(GL_PACK_ALIGNMENT,4);
	// Copy the image into our buffer
	glReadBuffer(GL_BACK_LEFT);
	
    // the original FBO read pixels code
    glReadPixels(0,0,E->xWinSize,E->yWinSize,GL_BGR,GL_UNSIGNED_BYTE,image);
    
	// Convert to YUV:
    sws_scale( img_convert_ctx, (const uint8_t* const*) RGBpict->data, RGBpict->linesize, 0, E->yWinSize, pict->data, pict->linesize );

	
	if(beenHere == -1)
		free(image);
	
}









