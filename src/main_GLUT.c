#include "slicer.h"

int main( int argc, char** argv )
{
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
    
    
    glutInit( &argc, argv );

	setupOpenGLEnvironment();
	setShaders(E);
	glutMainLoop();

	return 0;
}
