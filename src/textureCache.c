#include "slicer.h"

void init_texture_cache(e *E, unsigned int size){
	
	E->im_cache = (unsigned char **)malloc(size*sizeof(unsigned char*));
	if(E->im_cache == NULL){
		
		printf("Could not initialize texture cache - im_cache\n");
		exit(1);
	}
	
	E->mem_image = (unsigned char **)malloc(size*sizeof(unsigned char*));
	if(E->mem_image == NULL){
		
		printf("Could not initialize texture cache - im_cache\n");
		exit(1);
	}
	
	E->im_size = malloc(size * sizeof(off_t));
	if(E->im_size == NULL){
		
		printf("Could not initialize texture cache  - im_size\n");
		exit(1);
	}
	
	E->im = malloc(size*sizeof(GLuint));
	if(E->im == NULL){
		
		printf("Could not initialize texture cache  - im\n");
		exit(1);
	}
	
	E->im_width = malloc(size*sizeof(int));
	if(E->im_width == NULL){
		
		printf("Could not initialize texture cache  - im\n");
		exit(1);
	}
	
	E->im_height = malloc(size*sizeof(int));
	if(E->im_height == NULL){
		
		printf("Could not initialize texture cache  - im\n");
		exit(1);
	}
	
	E->im_channels = malloc(size*sizeof(int));
	if(E->im_channels == NULL){
		
		printf("Could not initialize texture cache  - im\n");
		exit(1);
	}
	
}

void populate_texture_mmap_cache(e *E){
	
	int	i;
	
	static double	date;
	int				image_file;
	struct stat		statbuf;
	
	char		fname[1024];
	
	date = E->start_date;
	
	for(i=0;i<E->totalDuration;i++){
		
		if(E->preview == 1)
			sprintf(fname,"%s/%s/img/%.6f_scaled.jpg",E->texture_archive_path, E->standard_name, date);
		else
			sprintf(fname,"%s/%s/img/%.6f.jpg",E->texture_archive_path, E->standard_name, date);
		
		// open the texture file
		if ((image_file = open (fname, O_RDONLY)) < 0){
			printf ("populate_texture_cache: can't open %s for reading\nexiting\n", fname);
			exit(1);
		}
		
		// find size of the image file 
		if (fstat (image_file, &statbuf) < 0){
			printf ("populate_texture_cache: fstat error\n");
			exit(1);
		}
		
		E->im_size[i] = statbuf.st_size;
		// mmap the image file
		if ((E->im_cache[i] = mmap (0, E->im_size[i], PROT_READ, MAP_PRIVATE, image_file, 0)) == NULL){
			printf ("populate_texture_cache: mmap failed\n");
			exit(1);
		}
		
		// close the input file 
		if( close(image_file) < 0 ){
			printf("populate_texture_cache: Error closing the image file\n");
			exit(1);
		}
		
		
		date++;
	}
	
}

int	get_int(int id){
	
	return id;
}


void *load_texture_maps(void *arg)
{
	
	param *p=(param *)arg;
		
	if(p->scene_num > 4018)
		return NULL;
		
	E->mem_image[p->scene_num] = SOIL_load_image_from_memory(
													 E->im_cache[p->scene_num],
													 E->im_size[p->scene_num],
													 &E->im_width[p->scene_num], &E->im_height[p->scene_num], &E->im_channels[p->scene_num], 
														SOIL_LOAD_RGB
													 );
	
	// ADD ERROR CHECK
	
	if(E->mem_image[p->scene_num] == 0){ 
		//printf("thread %d failed to load image %d from memory\n", p->thread_id, p->scene_num);
		//exit(1);
		//printf("width = %d, height = %d, channels = %d\n", E->im_width[p->id], E->im_height[p->id], E->im_channels[p->id]);
	}
		
	// return (NULL);
}


void update_texture_cache(e *E, int scene){
	
	int				i;
	pthread_t		*threads;
	pthread_attr_t	pthread_custom_attr;
	param			*p;
	
	threads = (pthread_t *)malloc(E->n_threads*sizeof(*threads));
	
	pthread_attr_init(&pthread_custom_attr);
	
	p =	(param *)malloc(sizeof(param)*E->n_threads);
		
	// start up the threads
	for (i=0; i<E->n_threads; i++){
		p[i].thread_id= i;
		p[i].scene_num = scene+i;
		pthread_create(&threads[i], &pthread_custom_attr, load_texture_maps, (void *)(p+i));
	}
	
	// Synchronize the completion of each thread
	
	for (i=0; i<E->n_threads; i++) {
		pthread_join(threads[i],NULL);
	}
		
	free(p);
	
}


void free_texture_cache(e *E, unsigned int size){

	int	i;
	
	
	/*
	for(i=0;i<size;i++){
	
		// unmap the mmaped memory
		if (munmap(E->im_cache[i], E->im_size[i]) == -1) {
			printf("update_scene: Error un-mmapping the image file\n");
			exit(1);
		}

	}
	*/
	
	free(E->im_cache);
	free(E->im_size);
}

