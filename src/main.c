
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <GLUT/glut.h>
//#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
//#include <OpenGL/glext.h>


#include "slicer.h"


#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>

//#include <OpenGL/OpenGLAvailability.h>
//#include <OpenGL/CGLCurrent.h>
//#include <OpenGL/CGLDevice.h>
//#include <OpenGL/CGLRenderers.h>
//#include <OpenGL/CGLIOSurface.h>
//#include <OpenGL/gltypes.h>

#include <OpenGL/CGLTypes.h>


/* 5 seconds stream duration */
//#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 24 /* 24 images/s */
//#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

float STREAM_DURATION;
int	STREAM_NB_FRAMES;

static int sws_flags = SWS_BICUBIC;

static AVFrame *picture, *tmp_picture;
static uint8_t *video_outbuf;
static int frame_count = 0;
static int video_outbuf_size;




double getSeconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec*1.0e-6;
}


/* add a video output stream */
static AVStream *add_video_stream(AVFormatContext *oc, enum CodecID codec_id)
{
    AVCodecContext *c;
    AVStream *st;
	
    st = avformat_new_stream(oc, NULL);
    if (!st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }
	
    c = st->codec;
    c->codec_id = codec_id;
    c->codec_type = AVMEDIA_TYPE_VIDEO;
	
    /* put sample parameters */
    c->bit_rate = 800000;
    /* resolution must be a multiple of two */
    c->width = E->xWinSize;//352;
    c->height = E->yWinSize;//288;
    /* time base: this is the fundamental unit of time (in seconds) in terms
	 of which frame timestamps are represented. for fixed-fps content,
	 timebase should be 1/framerate and timestamp increments should be
	 identically 1. */
    c->time_base.den = STREAM_FRAME_RATE;
    c->time_base.num = 1;
    c->gop_size = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt = STREAM_PIX_FMT;
    if (c->codec_id == CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B frames */
        c->max_b_frames = 2;
    }
    if (c->codec_id == CODEC_ID_MPEG1VIDEO){
        /* Needed to avoid using macroblocks in which some coeffs overflow.
		 This does not happen with normal video, it just happens here as
		 the motion of the chroma plane does not match the luma plane. */
        c->mb_decision=2;
    }
    // j addition for x264 encode
    if (c->codec_id == CODEC_ID_H264 ){
		/*mencoder mf://window*.png -mf fps=24 -sws 9 -vf harddup -ovc x264 
		 -x264encopts bitrate=8000:keyint=1:vbv_maxrate=10000:vbv_bufsize=5000:nocabac:level_idc=13:global_header 
		 -of lavf -lavfopts format=mov -o output.mov
		 */
		
		
		//printf("doing it\n");
        c->bit_rate = 8000000; //80,000,000;
        c->pix_fmt = STREAM_PIX_FMT;
        c->bit_rate_tolerance = 0;
        //c->rc_max_rate = 80000000;
        //c->rc_buffer_size = 80000000;
        //c->gop_size = 24;
        //c->max_b_frames = 1;
        //c->b_frame_strategy = 1;
        c->coder_type = 1;
        c->me_cmp = 1;
        c->me_range = 16;
        c->qmin = 10;
        c->qmax = 51;
        c->scenechange_threshold = 40;
        c->flags |= CODEC_FLAG_LOOP_FILTER;
        c->me_method = ME_HEX;
        c->me_subpel_quality = 5;
        c->i_quant_factor = 0.71;
        c->qcompress = 0.6;
        c->max_qdiff = 4;
        //c->directpred = 1;
        //c->flags2 |= CODEC_FLAG2_FASTPSKIP;
		c->flags2 |= CODEC_FLAG2_BPYRAMID + CODEC_FLAG2_MIXED_REFS + CODEC_FLAG2_WPRED + CODEC_FLAG2_8X8DCT + CODEC_FLAG2_FASTPSKIP;
		c->profile = FF_PROFILE_H264_BASELINE;
		
		if(E->preview == 1){
		
			//c->thread_count = 1;
			c->bit_rate = 4000000;
			
		}
		else
			c->thread_count = 8;
		
    }
    
    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	
    return st;
}


static AVFrame *alloc_picture(enum PixelFormat pix_fmt, int width, int height)
{
    AVFrame *picture;
    uint8_t *picture_buf;
    int size;
	
    picture = avcodec_alloc_frame();
    if (!picture)
        return NULL;
    size = avpicture_get_size(pix_fmt, width, height);
    picture_buf = av_malloc(size);
    if (!picture_buf) {
        av_free(picture);
        return NULL;
    }
    avpicture_fill((AVPicture *)picture, picture_buf,
                   pix_fmt, width, height);
    return picture;
}

static void open_video(AVFormatContext *oc, AVStream *st)
{
    AVCodec *codec;
    AVCodecContext *c;
	
    c = st->codec;
	
    /* find the video encoder */
    codec = avcodec_find_encoder(c->codec_id);
    if (!codec) {
        fprintf(stderr, "codec not found\n");
        exit(1);
    }
	
    /* open the codec */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }
	
    video_outbuf = NULL;
    if (!(oc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
		 as long as they're aligned enough for the architecture, and
		 they're freed appropriately (such as using av_free for buffers
		 allocated with av_malloc) */
        video_outbuf_size = 200000;
        video_outbuf = av_malloc(video_outbuf_size);
    }
	
    /* allocate the encoded raw picture */
    picture = alloc_picture(c->pix_fmt, c->width, c->height);
    if (!picture) {
        fprintf(stderr, "Could not allocate picture\n");
        exit(1);
    }
	
    /* if the output format is not YUV420P, then a temporary YUV420P
	 picture is needed too. It is then converted to the required
	 output format */
    tmp_picture = NULL;
    if (c->pix_fmt != PIX_FMT_YUV420P) {
        tmp_picture = alloc_picture(PIX_FMT_YUV420P, c->width, c->height);
        if (!tmp_picture) {
            fprintf(stderr, "Could not allocate temporary picture\n");
            exit(1);
        }
    }
	
	//printf("video_outbuf_size = %d\n", video_outbuf_size);
}




static void write_video_frame(AVFormatContext *oc, AVStream *st)
{
    int out_size, ret;
    AVCodecContext *c;
    static struct SwsContext *img_convert_ctx;
	
    c = st->codec;
	
    
    if (frame_count >= STREAM_NB_FRAMES) {
        // no more frame to compress. The codec has a latency of a few
		// frames if using B frames, so we get the last frames by
		// passing the same picture again 
    } else {
        if (c->pix_fmt != PIX_FMT_YUV420P) {
            // as we only generate a YUV420P picture, we must convert it
			// to the codec pixel format if needed 
            if (img_convert_ctx == NULL) {
                img_convert_ctx = sws_getContext(c->width, c->height,
                                                 PIX_FMT_YUV420P,
                                                 c->width, c->height,
                                                 c->pix_fmt,
                                                 sws_flags, NULL, NULL, NULL);
                if (img_convert_ctx == NULL) {
                    fprintf(stderr, "Cannot initialize the conversion context\n");
                    exit(1);
                }
            }
			
        } else {
			
			get_yuv_from_rgb(E, picture);
			
        }
    }
	
	
    /* encode the image */
    picture->pts=c->frame_number;
    out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, picture);
    //printf("%d doing encode frame part: out_size = %d\n", frame_count, out_size);
    /* if zero size, it means the image was buffered */
    if (out_size > 0) {
        AVPacket pkt;
        av_init_packet(&pkt);
        
        if (c->coded_frame->pts != AV_NOPTS_VALUE)
            pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
        if(c->coded_frame->key_frame)
            pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= video_outbuf;
        pkt.size= out_size;
        
        /* write the compressed frame in the media file */
        //printf("\t doing interleaved_write\n");
        ret = av_interleaved_write_frame(oc, &pkt);
        
    } else {
        ret = 0;
    }
    if (ret != 0) {
        fprintf(stderr, "Error while writing video frame\n");
        exit(1);
    }
    frame_count++;
    
}


static void flush_video_frame(AVFormatContext *oc, AVStream *st)
{
    int out_size, ret;
    AVCodecContext *c;
    
    c = st->codec;
    
    while(1) {

        out_size = avcodec_encode_video(c, video_outbuf, video_outbuf_size, NULL);
        if (out_size == 0) {
            break;
        }
        //printf("FLUSH: write frame %3d (size=%5d)\n", frame_count, out_size);
        AVPacket pkt;
        av_init_packet(&pkt);
        
        //if (c->coded_frame->pts != AV_NOPTS_VALUE)
        //    pkt.pts= av_rescale_q(c->coded_frame->pts, c->time_base, st->time_base);
        //if(c->coded_frame->key_frame)
        //    pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index= st->index;
        pkt.data= video_outbuf;
        pkt.size= out_size;
        ret = av_interleaved_write_frame(oc, &pkt);       
		if (ret != 0) {
			fprintf(stderr, "Error while flushing (writing) video frame\n");
			exit(1);
		}
    }    
}

static void close_video(AVFormatContext *oc, AVStream *st)
{
    avcodec_close(st->codec);
    av_free(picture->data[0]);
    av_free(picture);
    if (tmp_picture) {
        av_free(tmp_picture->data[0]);
        av_free(tmp_picture);
    }
    av_free(video_outbuf);
}



// Process has done i out of n rounds,
// and we want a bar of width w and resolution r.
static inline void progress_bar(int x, int n, int r, int w)
{
	int	i;
    // Only update r times.
    //if ( x % (n/r) != 0 ) return;
	
    // Calculuate the ratio of complete-to-incomplete.
    float ratio = x/(float)n;
    int   c     = ratio * w;
	
    // Show the percentage complete.
    printf("%3d%% [", (int)(ratio*100) );
	
    // Show the load bar.
    for (i=0; i<c; i++)
		printf("=");
	
    for (i=c; i<w; i++)
		printf(" ");
	
    // ANSI Control codes to go back to the
    // previous line and clear it.
    //printf("]\n\033[F\033[J");
	printf("]\n\033[A\033[K");
}


int main (int argc, char **argv)
{
	CGLContextObj ctx;	// the OpenGL Hardware Accelerated Context
	
	CGLPixelFormatAttribute attributes[] = {
		kCGLPFANoRecovery,
		kCGLPFAColorSize, (CGLPixelFormatAttribute)24,
		kCGLPFADepthSize, (CGLPixelFormatAttribute)24,
		//kCGLPFAAccelerated,			// this one doesn't work with server side code :(
		kCGLPFARemotePBuffer,
		kCGLPFAAllowOfflineRenderers,
		kCGLPFASampleBuffers,1,
		kCGLPFASamples,4,
		kCGLPFAMultisample,
		(CGLPixelFormatAttribute)0
	};
	
	CGLPixelFormatObj pix;
	GLint num; // stores the number of possible pixel formats
	
	
    char    parameter_filename[256];
    FILE    *input_stream;
	FILE	*progress_file;
    int 	i;
	
	GLuint  fbo = 0;
	GLuint  renderBuffer = 0;
    GLuint  depthBuffer = 0;
	
	
	char filename[1024];// = "output_libav.mov";
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *video_st;
	
	unsigned int start_time, end_time;
	

	
    
	E = (e*)malloc(sizeof(e));
    if(E == NULL){
        fprintf(stderr,"Not enough memory for structs\n");
        exit(1);
    }
	//malloc date_string
	E->date_string = malloc(1024*sizeof(char));
	
	
	if ( argc < 2 ){
		fprintf(stderr, "%s: need an xml file as an argument\n", argv[0] );
		exit(1);
    }
	
	
	// handle the command line args here:
    for (i=1 ; i<argc ; i++){
        // If this is the last argument then it must be the file to process
        if (i == 1){
            if ( (input_stream = fopen( argv[i], "r" )) == NULL ){
                fprintf(stderr,"could not open input file : %s\n", argv[i]);
                exit(1);
            }
            else{   // Always copy the good file name and close the file
                strcpy(parameter_filename, argv[i]);
                fclose(input_stream);
            }
        }
    }
	
    getparams_xml(parameter_filename, E);
	
	// open the progress file
	progress_file = fopen(E->progress_file,"w");
	if( progress_file == NULL){
		fprintf(stderr,"Failed to open progress file\n");
		exit(1);
	}
    	
    CGLChoosePixelFormat( attributes, &pix, &num );
	CGLCreateContext( pix, NULL, &ctx ); // second parameter can be another context for object sharing
	CGLDestroyPixelFormat( pix );
	CGLSetCurrentContext( ctx );
	
	CGLLockContext(ctx);
	
	// Create an FBO	
	
    // Depth buffer to use for depth testing - optional if you're not using depth testing
    glGenRenderbuffersEXT(1, &depthBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depthBuffer);
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, E->xWinSize, E->yWinSize);
	
    // Render buffer to use for imaging
    glGenRenderbuffersEXT(1, &renderBuffer);
    glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderBuffer);
	// this was GL_RGBA8
    glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGB8, E->xWinSize, E->yWinSize);
	
    
    glGenFramebuffersEXT(1, &fbo);
	
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
	
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, renderBuffer);
	
    glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthBuffer);
	

	setupOpenGLEnvironment();
	setShaders(E);
		
	glEnable(GL_MULTISAMPLE);
	
	
	//----------------- enable libav h264 write --------
	
	STREAM_DURATION = E->totalDuration/(float)STREAM_FRAME_RATE;	// movie length in seconds
	STREAM_NB_FRAMES = ((int)(STREAM_DURATION * STREAM_FRAME_RATE));
	
    //printf("Stream Duration = %f\n", STREAM_DURATION);
    //printf("Stream nb frames = %d\n", STREAM_NB_FRAMES);
    
	// initialize libavcodec, and register all codecs and formats
    av_register_all();
	av_log_set_level(AV_LOG_QUIET);
	
	fmt = av_guess_format("mov", NULL, NULL);
	if (!fmt) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        fmt = av_guess_format("mpeg", NULL, NULL);
    }
    if (!fmt) {
        fprintf(stderr, "Could not find suitable output format\n");
        exit(1);
    }
	
    // allocate the output media context 
    oc = avformat_alloc_context();
    if (!oc) {
        fprintf(stderr, "failed to allocate libav context\n");
        exit(1);
    }
    oc->oformat = fmt;
    //sprintf(filename,"%s/output_libav.mov",E->output_path);
    sprintf(filename,"%s",E->output_path);
    snprintf(oc->filename, sizeof(oc->filename), "%s", filename);
	
    // add the audio and video streams using the default format codecs
	// and initialize the codecs 
    video_st = NULL;
    if (fmt->video_codec != CODEC_ID_NONE) {
        video_st = add_video_stream(oc, fmt->video_codec);
    }
    
	
    // set the output parameters (must be done even if no
	//  parameters).
    if (av_set_parameters(oc, NULL) < 0) {
        fprintf(stderr, "Invalid libav output format parameters\n");
        exit(1);
    }
	
    av_dump_format(oc, 0, filename, 1);
	
    // now that all the parameters are set, we can open the audio and
	// video codecs and allocate the necessary encode buffers 
    if (video_st)
        open_video(oc, video_st);
    
    // open the output file, if needed
    if (!(fmt->flags & AVFMT_NOFILE)) {
        if (avio_open(&oc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "libav: Could not open '%s'\n", filename);
            exit(1);
        }
    }
	
    // write the stream header, if any
    avformat_write_header(oc, NULL);
	
	//--------------------------- end h264 write setup -------------------------------------
	
	// set the number of threads
	
	if(E->totalDuration < 32)
		E->n_threads = E->totalDuration;
	else
		E->n_threads = 32;
	
	// main render loop
	for(i=0;i<E->totalDuration;i++){
		
		//printf("render loop i = %d\n", i);
		if( (i%E->n_threads) == 0){
			
			if( (E->totalDuration - i) < E->n_threads)
				E->n_threads = (E->totalDuration - i);
			
			update_texture_cache(E,i);
			
		}
		
		//update_scene();
		update_scene_with_texture_cache(i);
		
		
		glViewport(0, 0, E->xWinSize, E->yWinSize);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		myReshape( E->xWinSize, E->yWinSize );

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		glLoadIdentity();
		
		gluLookAt( E->viewer[0], E->viewer[1], E->viewer[2],  
				  0.0, 0.0, 0.0, 	// center is at (0,0,0)
				  0.0, 1.0, 0.0 ); // up is in +ve Y direction
		
		glClear(GL_COLOR_BUFFER_BIT);
		
		glClear(GL_DEPTH_BUFFER_BIT);
		
		draw(E, i);
		
		// free the mem
		glDeleteTextures( 1, &E->im[i] );
		
		write_video_frame(oc, video_st);
		
		if(i%20 == 0){
			progress_bar(i, E->totalDuration, 1, 30);
			fprintf(progress_file,"%u\t%d\n",(unsigned)time(NULL), (int) (100.0*(float)i/(float)E->totalDuration));
			fflush(progress_file);
		}
		
	}
    
	// ------------ clean up the h264 movie stuff ---------------------------
	
	//printf("doing flush frames...\n");
    flush_video_frame(oc, video_st);
    //avcodec_flush_buffers(video_st->codec);
	
	// write the trailer, if any.  the trailer must be written
	// before you close the CodecContexts open when you wrote the
	// header; otherwise write_trailer may try to use memory that
	// was freed on av_codec_close() 
    av_write_trailer(oc);
	
    // close each codec 
    if (video_st)
        close_video(oc, video_st);
	
    // free the streams 
    for(i = 0; i < oc->nb_streams; i++) {
        av_freep(&oc->streams[i]->codec);
        av_freep(&oc->streams[i]);
    }
	
    if (!(fmt->flags & AVFMT_NOFILE)) {
        // close the output file 
        avio_close(oc->pb);
    }
	
    // free the stream 
    av_free(oc);
	
	//-----------------------------------------------------------------------------
	
	
    // "un"bind my FBO
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    	
    CGLSetCurrentContext(NULL);
    CGLDestroyContext(ctx);
	
	fclose(progress_file);
	free_texture_cache(E, E->totalDuration);
	
	
    return 0;
}

