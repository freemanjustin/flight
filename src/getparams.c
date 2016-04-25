#include "slicer.h"


void setDefaults(e *E)
{
	// set defaults for the E struct


}
void getparams_xml(char* parameter_filename, e *E)
{
    xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *txt, *txt2, *txt3;
	
	int		mm, dd, yy, hh, min;
	double	sec;
	
	const char *month_name[] = { "", "January", "February", "March", 
		"April", "May", "June", "July", "August", "September", "October",
		"November", "December" };
	
	E->numLayers = 0;
	E->npts = 0;
	E->totalDuration = 0;
	
	setDefaults(E);
	
	doc = xmlParseFile(parameter_filename);
	
	if (doc == NULL ) {
		fprintf(stderr,"Could not open input file %s\n", parameter_filename);
		exit(1);
	}
	
	cur = xmlDocGetRootElement(doc);
	
	if (cur == NULL) {
		fprintf(stderr,"empty input file %s\n", parameter_filename);
		xmlFreeDoc(doc);
		exit(1);
	}
	
	if (xmlStrcmp(cur->name, (const xmlChar *) "SlicerData")) {
		fprintf(stderr,"%s is the wrong type, root node != SlicerData\n", parameter_filename);
		xmlFreeDoc(doc);
		exit(1);
	}
	
	// minimal parameters in an input file are:
	//
	//	1 sphere geometry
	// 	at least 1 layer
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		// read in runtime control parameter
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "params"))){
            //printf( "params :\n" );
			parseInputFile_params (E, doc, cur);
		}
		// read in the layer parameters
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "layer"))){
			txt = xmlGetProp( cur, (const xmlChar *) "name" );
            //printf( "layer %s:\n", txt );
            lr_pack( (char *)txt );
			
			// strdup is a GNU extension - don't use it!
			//E->L[E->numLayers].name = strdup((char*)txt);
			
			// malloc memory for the name
			E->L[E->numLayers].name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
			strcpy(E->L[E->numLayers].name, (char*)txt);
			
			parseInputFile_layers (E, doc, cur, txt);
			E->numLayers++;
			xmlFree( txt );
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "sequence"))){
			
			txt = xmlGetProp( cur, (const xmlChar *) "name" );
            //printf( "sequence name = %s\n", txt );
            lr_pack( (char *)txt );
			
			E->standard_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
			strcpy(E->standard_name, (char*)txt);
			//printf("standard_name = %s\n",E->standard_name );
			
			txt2 = xmlGetProp( cur, (const xmlChar *) "start_date" );
            lr_pack( (char *)txt2 );
			//sscanf(txt,"%lf", &E->start_date);
			//E->standard_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
			//strcpy(E->standard_name, (char*)txt);
			E->start_date  = strtod((char *)txt2, (char **)NULL);
			
			caldat(E->start_date, &mm, &dd, &yy, &hh, &min, &sec);
			//printf("start_date = %f (%2d %s %4d)\n",E->start_date, dd, month_name[mm], yy );
			
			
			
			
			txt3 = xmlGetProp( cur, (const xmlChar *) "preview" );
            lr_pack( (char *)txt3 );
			if(strncmp(txt3,"true",4) == 0){
				E->preview = 1;
			}
			else if(strncmp(txt3,"TRUE",4) == 0){
				E->preview = 1;
			}
			else{
				E->preview = 0;
			}
			//printf("Preview = %d\n", E->preview);
			
			
			if(strncmp(E->standard_name,"sea_surface_temperature_anomaly",31) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
				sprintf(E->long_name,"Sea Surface Temperature Anomaly");
			}
			else if(strncmp(E->standard_name,"sea_surface_temperature",23) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
				sprintf(E->long_name,"Sea Surface Temperature");
			}
			else if(strncmp(E->standard_name,"sea_surface_salinity_anomaly",28) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
				sprintf(E->long_name,"Sea Surface Salinity Anomaly");
			}
			else if(strncmp(E->standard_name,"sea_surface_salinity",20) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
				sprintf(E->long_name,"Sea Surface Salinity");
			}
			else if(strncmp(E->standard_name,"sea_surface_velocity",20) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+11));
				sprintf(E->long_name,"Sea Surface Velocity Magnitude");
			}
			else if(strncmp(E->standard_name,"sea_surface_height_anomaly",26) == 0){
				E->long_name = (char*)malloc(sizeof(char)*(strlen((char*)txt)+1));
				sprintf(E->long_name,"Sea Surface Height Anomaly");
			}
			
			//printf("long_name = %s\n", E->long_name);
			
			//printf("found sequence tag\n");
			parseInputFile_motion (E, doc, cur);
			
		}
		 
		cur = cur->next;
	}
	
	xmlFreeDoc(doc);
	
	//printf("There are %d layers present in %s\n", E->numLayers, parameter_filename);
	//printf("There are %d sequences present in %s\n", E->npts, parameter_filename);
	
	if(E->npts>0){
		// override the camer settings
		E->theta[0] = E->pts[0].pt[0];
		E->theta[1] = E->pts[0].pt[1]; 
		E->theta[2] = E->pts[0].pt[2];
		
		E->viewer[0] = 0.0; 
		E->viewer[1] = 0.0; 
		E->viewer[2] = E->pts[0].zoom;

	}
	
	
	// if we are in preview mode then set the window size to small
	if(E->preview == 1){
	
		E->xWinSize = 320;
		E->yWinSize = 180;
		
	}
	
	return;
 	
}

void parseInputFile_motion (e *E, xmlDocPtr doc, xmlNodePtr cur) {
	
	xmlChar *key;
	//xmlChar *txt;
	char 	*str;
	const char delimiters[] = " ";
	
	//printf("inside parse motion\n");
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "pt"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			
			lr_pack( (char *)key );
			str = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			strcpy(str, (char*)key);
			
			E->pts[E->npts].pt[0] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			E->pts[E->npts].pt[1] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			//E->pts[E->npts].pt[2] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			E->pts[E->npts].zoom = strtof((const char *)strsep(&str, delimiters), (char **)NULL);	// this is zoom level
			E->pts[E->npts].duration = strtof((const char *)strsep(&str, delimiters), (char **)NULL);	// this is duration
			
			if ( E->pts[E->npts].pt[1] < 0.0) 
				E->pts[E->npts].pt[1]+=360.0;
			
						
			//printf("pt %d: lat = %f, lon = %f\n", E->npts,  E->pts[E->npts].pt[0], E->pts[E->npts].pt[1]);
			
			xmlFree(key);
			E->totalDuration += E->pts[E->npts].duration;
			E->npts++;
			free(str);
			
		}
		cur = cur->next;
	}
	
}


void parseInputFile_params (e *E, xmlDocPtr doc, xmlNodePtr cur) {

	xmlChar *key, *txt;
	static int	haveSphere = 0;
	static int 	haveCore = 0;
	static int	haveCamera = 0;
	static int	haveWindow = 0;
	static int	haveColor = 0;
	char 	*str;
	const char delimiters[] = " ";
	
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		/*
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "geometry"))) {
			txt = xmlGetProp( cur, (const xmlChar *) "name" );
			
			if(!xmlStrcmp(txt,(const xmlChar *) "sphere")){	// sphere geometry
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				lr_pack( (char *)key );
				E->what_to_draw = strtol((char *)key, (char **)NULL, 0);
				haveSphere++;
				//printf("got a sphere geometry: %s\n", key);
				//printf("E->what_to_draw = %d\n", E->what_to_draw);
				xmlFree(key);
				
			}
			else if(!xmlStrcmp(txt,(const xmlChar *) "core")){// core geometry
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				lr_pack( (char *)key );
				E->what_to_draw_core = strtol((char *)key, (char **)NULL, 0);
				haveCore++;
				//printf("got a core geometry: %s\n", key);
				//printf("E->what_to_draw_core = %d\n", E->what_to_draw_core);
				xmlFree(key);
			}
		}
		 */
		
		if((!xmlStrcmp(cur->name, (const xmlChar *) "backgroundColor"))){// backgorund color
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				
				lr_pack( (char *)key );
				
				//str = strdup((char*)key);
				str = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
				strcpy(str, (char*)key);
				
				
				E->bg.red = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				E->bg.green = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				E->bg.blue = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				
				
				//printf("E->bgColor.red  = %f, E->bgColor.green = %f, E->bgColor.blue  = %f\n", 
				//					E->bgColor.red ,
				//					E->bgColor.green, 
				//					E->bgColor.blue );
				// set the background color
				//glClearColor(r,g,b, 0.0);
				//exit(1);
				haveColor++;
				xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "window"))){// backgorund color
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				
				lr_pack( (char *)key );
				//str = strdup((char*)key);
				str = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
				strcpy(str, (char*)key);
				
				E->xWinSize = (GLint)strtol((const char *)strsep(&str, delimiters), (char **)NULL,0);
				E->yWinSize = (GLint)strtol((const char *)strsep(&str, delimiters), (char **)NULL,0);
				
				
				haveWindow++;
				xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "camera"))){// backgorund color
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				
				lr_pack( (char *)key );
				//str = strdup((char*)key);
				str = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
				strcpy(str, (char*)key);
				
				E->theta[0] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				E->theta[1] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				E->theta[2] = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
				
				haveCamera++;
				xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "zoom"))){// backgorund color
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				
				lr_pack( (char *)key );
				E->viewer[2] = strtod((char *)key, (char **)NULL);
				
				
				xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "lon_offset"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			
			lr_pack( (char *)key );
			E->lon_offset = strtod((char *)key, (char **)NULL);
			
			
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "shader_path"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->shader_path = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->shader_path,"%s",key);
			strncpy(E->shader_path, (char*)key, strlen((char*)key)+1);
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "font_path"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->font_path = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->font_path,"%s",key);
			strncpy(E->font_path, (char*)key, strlen((char*)key)+1);
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "font_name"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->font_name = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->font_name,"%s",key);
			strncpy(E->font_name, (char*)key, strlen((char*)key)+1);
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "output_path"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->output_path = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->output_path,"%s",key);
			strncpy(E->output_path, (char*)key, strlen((char*)key)+1);
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "progress_file"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->progress_file = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->shader_path,"%s",key);
			strncpy(E->progress_file, (char*)key, strlen((char*)key)+1);
			xmlFree(key);
		}
		else if((!xmlStrcmp(cur->name, (const xmlChar *) "texture_archive"))){// backgorund color
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			lr_pack( (char *)key );
			E->texture_archive_path = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			//sprintf(E->output_path,"%s",key);
			strncpy(E->texture_archive_path, (char*)key, strlen((char*)key)+1);
			//printf("texture_archive_path = %s\n", E->texture_archive_path);
			xmlFree(key);
		}
		
		
	cur = cur->next;
	}
	
	// error check on geometry
	
	if(haveCamera == 0){
		E->theta[0] = 125.0; 
		E->theta[1] = 0.0; 
		E->theta[2] = 225.0;
	
		E->viewer[0] = 0.0; 
		E->viewer[1] = 0.0; 
		E->viewer[2] = 3.0;
	}
	if(haveWindow == 0){
		E->xWinSize = E->yWinSize = 800;
	}
	if(haveColor == 0){
		E->bg.red = E->bg.green = E->bg.blue = 0.0;
	}
	//printf("\n");
    return;
}

void parseInputFile_layers (e *E, xmlDocPtr doc, xmlNodePtr cur, xmlChar *txt) {

	xmlChar *key;
	char 	*str;
	const char delimiters[] = " ";
		
	int		haveStartRadius = 0;
	int		haveEndRadius = 0;
	int		haveTexture = 0;
	int		haveColor = 0;
		
	cur = cur->xmlChildrenNode;
	while (cur != NULL) {
		/*
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "start_radius"))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("start_radius: %s\n", key);
		    E->L[E->numLayers].start = strtod((char *)key, (char **)NULL);
		    //printf("E->L[%d].start = %f\n", E->numLayers, E->L[E->numLayers].start);
		    xmlFree(key);
		    haveStartRadius++;
 	    }
 	    else if ((!xmlStrcmp(cur->name, (const xmlChar *) "end_radius"))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("end_radius: %s\n", key);
		    E->L[E->numLayers].end = strtod((char *)key, (char **)NULL);
		    //printf("E->L[%d].end = %f\n",E->numLayers, E->L[E->numLayers].end);
		    xmlFree(key);
		    haveEndRadius++;
 	    }
		 */
	    if ((!xmlStrcmp(cur->name, (const xmlChar *) "texture"))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("texture: %s\n", key);
		    sprintf(E->L[E->numLayers].textureMap,"%s",key);
		    //printf("E->L[%d].textureMap =%s\n",E->numLayers,E->L[E->numLayers].textureMap);
		    xmlFree(key);
		    haveTexture++;
			E->L[E->numLayers].wms.haveWMS = 0;
 	    }
 	    else if ((!xmlStrcmp(cur->name, (const xmlChar *) "color"))) {
		    key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
			
			lr_pack( (char *)key );
			//str = strdup((char*)key);
			str = (char*)malloc(sizeof(char)*(strlen((char*)key)+1));
			strcpy(str, (char*)key);
			
			E->L[E->numLayers].colors.red = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			E->L[E->numLayers].colors.green = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			E->L[E->numLayers].colors.blue = strtof((const char *)strsep(&str, delimiters), (char **)NULL);
			
		    //printf("color: %s.\n", key);
		    //printf("E->L[%d].rgb.r = %f, E->L[%d].rgb.g = %f, E->L[%d].rgb.b = %f\n", E->numLayers,E->L[E->numLayers].colors.red, 
		    //						E->numLayers,E->L[E->numLayers].colors.green, 
		    //						E->numLayers,E->L[E->numLayers].colors.blue);
		    xmlFree(key);
		    haveColor++;
 	    } 
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "WMS"))) {
		    parse_WMS_params(E,doc,cur);
 	    } 
		cur = cur->next;
	}
	
	//error check
	if(haveTexture == 0){
		sprintf(E->L[E->numLayers].textureMap,"NULL");
	}
	if(haveTexture > 1){
		fprintf(stderr,"Warning multiple texture definitions in layer: %s\n\tUsing the last definition\n", txt);
	}
	
	if(haveColor > 1){
		fprintf(stderr,"Warning multiple color definitions in layer: %s\n\tUsing the last definition\n", txt);
	}
	
	if(haveColor == 0){
		fprintf(stderr,"Warning color undefined for layer: %s\n\tUsing default color (1.0 1.0 1.0)\n", txt);
		E->L[E->numLayers].colors.red = 1.0;
		E->L[E->numLayers].colors.green = 1.0;
		E->L[E->numLayers].colors.blue = 1.0;
	}
	
	if( (haveColor == 0) && (haveTexture == 0)){
		fprintf(stderr,"Warning: No texture or color defined for layer: %s\n",txt);
	}
	
	//printf("\n");
    return;
}


void parse_WMS_params (e *E, xmlDocPtr doc, xmlNodePtr cur) {
	xmlChar *key;
	char	getString[2048];
	cur = cur->xmlChildrenNode;
	
	E->L[E->numLayers].wms.haveWMS = 1;
	
	while (cur != NULL) {
		if ((!xmlStrcmp(cur->name, (const xmlChar *) "url"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS URL: %s\n", key);
		    sprintf(E->L[E->numLayers].wms.url,"%s",key);
		    //printf("E->L[%d].wms.url =%s\n",E->numLayers,E->L[E->numLayers].wms.url);
		    xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "layer"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS layer: %s\n", key);
		    sprintf(E->L[E->numLayers].wms.layer,"%s",key);
		    //printf("E->L[%d].wms.layer =%s\n",E->numLayers,E->L[E->numLayers].wms.layer);
		    xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "range"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS range: %s\n", key);
			sscanf((char*)key,"%lf %lf\n", &E->L[E->numLayers].wms.min, &E->L[E->numLayers].wms.max); 
		    //printf("E->L[%d].range_min = %f, E->L[%d].range_max = %f\n",E->numLayers, E->L[E->numLayers].wms.min,E->numLayers, E->L[E->numLayers].wms.max);
		    xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "colormap"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS colormap: %s\n", key);
		    sprintf(E->L[E->numLayers].wms.colormap,"%s",key);
		    //printf("E->L[%d].wms.colormap = %s\n",E->numLayers,E->L[E->numLayers].wms.colormap);
		    xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "bbox"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS bbox: %s\n", key);
		    sscanf((char*)key,"%lf %lf %lf %lf\n",&E->L[E->numLayers].wms.lon_min,&E->L[E->numLayers].wms.lat_min,&E->L[E->numLayers].wms.lon_max,&E->L[E->numLayers].wms.lat_max);
		    //printf("E->L[%d].wms.bbox = %f %f %f %f\n",E->numLayers,E->L[E->numLayers].wms.lon_min,E->L[E->numLayers].wms.lat_min,E->L[E->numLayers].wms.lon_max,E->L[E->numLayers].wms.lat_max);
		    xmlFree(key);
		}
		else if ((!xmlStrcmp(cur->name, (const xmlChar *) "dimensions"))) {
			key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
		    lr_pack( (char *)key );
		    //printf("WMS dimensions: %s\n", key);
		    sscanf((char*)key,"%d %d\n", &E->L[E->numLayers].wms.width, &E->L[E->numLayers].wms.height); 
		    //printf("E->L[%d].textureMap =%s\n",E->numLayers,E->L[E->numLayers].textureMap);
		    xmlFree(key);
		}
		cur = cur->next;
	}
	
	// set the WMS request string
	sprintf(E->L[E->numLayers].wms.wms_request,"%s?LAYERS=%s&COLORSCALERANGE=%f,%f&TRANSPARENT=false&FORMAT=image/png&SERVICE=WMS&VERSION=1.1.1&REQUEST=GetMap&STYLES=boxfill/%s&SRS=EPSG:4326&BBOX=%f,%f,%f,%f&WIDTH=%d&HEIGHT=%d", 
			E->L[E->numLayers].wms.url,																	
			E->L[E->numLayers].wms.layer,
																					E->L[E->numLayers].wms.min,
																					E->L[E->numLayers].wms.max,
																					E->L[E->numLayers].wms.colormap,
																					E->L[E->numLayers].wms.lon_min,
																					E->L[E->numLayers].wms.lat_min,
																					E->L[E->numLayers].wms.lon_max,
																					E->L[E->numLayers].wms.lat_max,
																					E->L[E->numLayers].wms.width, 
																					E->L[E->numLayers].wms.height
			);
	
	
	//printf("%s\n", E->L[E->numLayers].wms.wms_request);
	
	// for now lets just download the file
	sprintf(getString,"curl -o wms_map.png -O \"%s\" >& /dev/null\n", E->L[E->numLayers].wms.wms_request);
	system(getString);
	
	// set the texture map to the file we just downloaded
	sprintf(E->L[E->numLayers].textureMap,"wms_map.png");
	
}



void loadTextureMaps(e	*E)
{
	int	i;
	//pngInfo infoLayer;
	pngSetStandardOrientation(1);
	pngSetViewingGamma(1.45);
	
	// increment the number of layers that we have
	// this used to be in the XML file but for this version
	// of the code we generate the path and file name at run time
	
	E->numLayers++;
	// generate the colorbar filename
	sprintf(E->L[E->numLayers-1].textureMap,"%s/colorbars/%s256x256.png",E->texture_archive_path, E->standard_name);
	
	E->numLayers++;
	// now generate the surface texture file name
	sprintf(E->L[E->numLayers-1].textureMap,"%s/%s/img/%.6f.jpg",E->texture_archive_path, E->standard_name, E->start_date);
	
	
	// load the texture maps
	// start from the center and go to the surface
	for(i=0;i<E->numLayers;i++){
		if(strncmp("NULL", E->L[i].textureMap,4) == 0){
			// don't assign a texture map
		}
		else{
			
			 // original libpng loader
			/*
			E->texture[i] = pngBind(E->L[i].textureMap, 
									PNG_BUILDMIPMAPS, 
									PNG_ALPHA, 
									&infoLayer, 
									GL_CLAMP_TO_EDGE, 
									//GL_REPEAT,
									GL_LINEAR_MIPMAP_LINEAR, 
									GL_LINEAR_MIPMAP_LINEAR);
			 
			*/
			
			E->texture[i] =  SOIL_load_OGL_texture(
												   E->L[i].textureMap,
												   SOIL_LOAD_AUTO,
												   SOIL_CREATE_NEW_ID,
												   //1,
												   SOIL_FLAG_INVERT_Y 
												   );
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			
			
			if (E->texture[i] == 0) {
				fprintf(stderr,"getparams.c: Can't load file: %s\n",E->L[i].textureMap);
				exit(1);
			}
			
			else{
				//printf("getparams.c: %s: Size=%i,%i Depth=%i Alpha=%i\n", 
				//					E->L[i].textureMap,
				//					infoLayer.Width, 
				//					infoLayer.Height, 
				//					infoLayer.Depth, 
				//					infoLayer.Alpha);
				//printf("getparams.c: numlayers = %d, texture[%d] =  %s\n", E->numLayers, i, E->L[i].textureMap);
			}
			
		}
	}
}

