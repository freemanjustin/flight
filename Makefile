###################################################################
#
#  slicer 
#
# freeman.justin@gmail.com
#
##################################################################


OBJ=	./src/main.o \
		./src/openglSetup.o \
		./src/openglMisc.o \
		./src/draw.o \
		./src/loadTexture.o \
		./src/getparams.o \
		./src/textfile.o \
		./src/sphere.o \
		./src/interp_1d.o \
		./src/rgb2yuv.o \
		./src/mencode.o \
		./src/type.o \
		./src/textureCache.o \
		./freetype-gl/vertex-buffer.o \
		./freetype-gl/vertex-attribute.o \
		./freetype-gl/vector.o \
		./freetype-gl/texture-atlas.o \
		./freetype-gl/texture-font.o \
		./src/jutil.o \
		./src/SOIL/SOIL/src/SOIL.o \
		./src/SOIL/SOIL/src/image_DXT.o \
		./src/SOIL/SOIL/src/image_helper.o \
		./src/SOIL/SOIL/src/stb_image_aug.o
		


# Include flags
INC=   -D_OS_X_ \
	-I./include -I./freetype-gl \
	`freetype-config --cflags` \
	-I./src/SOIL/SOIL/src	\
	`/usr/bin/xml2-config --cflags`

# Libraries
LFLAGS= -lobjc \
	-framework AppKit \
	-framework Foundation \
	-framework OpenGL \
	-framework GLUT \
	`/usr/bin/xml2-config --libs` \
	`freetype-config --libs` \
	-lavcodec -lavformat -lavdevice -lavfilter -lavutil -lswscale \
	-framework CoreFoundation \
	-framework VideoDecodeAcceleration \
	-framework QuartzCore \
	-lm -lbz2 -lz -lx264 -lpng

CFLAGS=	-O3 -g -Wall -Wno-deprecated-declarations

CC=	gcc $(CFLAGS) $(INC) 


# Executable

EXEC=	./bin/slicer_quiet	

$(EXEC):$(OBJ)
	$(CC) -o $(EXEC) $(OBJ) $(LFLAGS)

clean:
	rm $(OBJ)
	rm $(EXEC)
