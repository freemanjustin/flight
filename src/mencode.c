#include "slicer.h"


const char mencode[]="mencoder mf://window*.jpg -mf fps=24 -sws 9 -vf harddup -ovc x264 -x264encopts bitrate=8000:keyint=1:vbv_maxrate=10000:vbv_bufsize=5000:nocabac:level_idc=13:global_header -of lavf -lavfopts format=mov -o output.mov";

void mencode_frames(){

	system(mencode);

}