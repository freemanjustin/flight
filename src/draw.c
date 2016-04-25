#include "slicer.h"


void addText(int x, int y, char *str){
	
	wchar_t *text;
	
	vec2 pen = {{0,0}};
	vec4 color = {{0.95,0.95,0.95,1.0}};
	
	
	text = malloc(strlen(str)*sizeof(wchar_t));
	swprintf(text, strlen(str), L"%s", str);
	
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    glOrtho(0, E->xWinSize, 0, E->yWinSize, -1, 1);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
	
	vertex_buffer_clear( buffer );
	
	pen.x = x;
	pen.y = y;
	add_text( buffer, font, text, &color, &pen );
	
    glColor4f( 0.0, 0.0, 0.0, 1.0 );
    glBindTexture( GL_TEXTURE_2D, atlas->id );
    vertex_buffer_render( buffer, GL_TRIANGLES, "vtc" );
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );
	free(text);

}

void addText2(int x, int y, char *str){
	
	wchar_t *text;
	
	vec2 pen = {{0,0}};
	vec4 color = {{0.95,0.95,0.95,1.0}};
	
	
	text = malloc(strlen(str)*sizeof(wchar_t));
	swprintf(text, strlen(str), L"%s", str);
	
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    glOrtho(0, E->xWinSize, 0, E->yWinSize, -1, 1);
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity( );
    glDisable( GL_DEPTH_TEST );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_BLEND );
	
	vertex_buffer_clear( buffer );
	
	pen.x = x;
	pen.y = y;
	add_text( buffer, font_small, text, &color, &pen );
	
    glColor4f( 0.0, 0.0, 0.0, 1.0 );
    glBindTexture( GL_TEXTURE_2D, atlas_small->id );
    vertex_buffer_render( buffer, GL_TRIANGLES, "vtc" );
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );
	free(text);
	
}

void draw_logo(e *E, int pos, int logo){
	
	static int		beenHere = 0;
	static float	asset_aspect;
	static float	size, eps;
	static float	u[3];
	static float	v[3];
	
	if(beenHere == 0){
		eps = 0;
		asset_aspect = 1.0;
		size = 128.0;
		
		u[0] = size/asset_aspect;
		u[1] = u[2] = 0.0;
		
		v[0] = v[2] = 0.0;
		v[1] = size*(asset_aspect);
		
		beenHere = 1;
	}
		
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, E->xWinSize, 0, E->yWinSize);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, E->texture[logo]);
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	
	if(pos == BOTTOM_RIGHT){// put logo in lower right hand corner
		//glTranslatef((100-size),-1.0*((100-size)-eps),0);
	}
	else if(pos == BOTTOM_LEFT){
		// put logo in lower left hand corner
		glTranslatef(-100+size+eps,-1.0*((100-size)),0);
	}
	else if(pos == TOP_LEFT){
		// put logo in top left hand corner
		//glTranslatef(-100+size+eps,1.0*((100-size)-eps),0);
	}
	else if(pos == TOP_RIGHT){
		// put logo in top right hand corner
		//glTranslatef(100-size,1.0*((100-size)-eps),0);
		//glTranslatef(E->xWinSize/2.0,E->yWinSize/2.0,0);
		
		// for the 256x256 logo use the following offsets:
		glTranslatef(E->xWinSize-size,E->yWinSize-size,0);
		
	}
	
	glBegin (GL_QUADS);
	glTexCoord2i(1,1); glVertex3f ( u[0] + v[0],  u[1] + v[1],  u[2] + v[2]);
	glTexCoord2i(0,1); glVertex3f (-u[0] + v[0], -u[1] + v[1], -u[2] + v[2]);
	glTexCoord2i(0,0); glVertex3f (-u[0] - v[0], -u[1] - v[1], -u[2] - v[2]);
	glTexCoord2i(1,0); glVertex3f ( u[0] - v[0],  u[1] - v[1],  u[2] - v[2]);
	glEnd ();
	
	
	glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
}


void draw_colorbar(e *E, int colorbar){
	
	static int	beenHere = 0;
	static float	asset_aspect;
	static float	size, eps;
	static float u[3];// = {15, 0, 0};
	static float v[3];// = {0, 15, 0};
	
	
	if( beenHere == 0){
		
		eps = 0;
		asset_aspect = 1.0;
		size = 128.0;
		
		u[0] = size/asset_aspect;
		u[1] = u[2] = 0.0;
		
		v[0] = v[2] = 0.0;
		v[1] = size*(asset_aspect);
		
		beenHere = 1;
	}
		
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_TEXTURE_2D);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, E->xWinSize, 0, E->yWinSize);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, E->texture[colorbar]);
	
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	// put the colorbar center right
	glTranslatef(E->xWinSize-size,E->yWinSize/2.0-20,0);
	
	glBegin (GL_QUADS);
	glTexCoord2i(1,1); glVertex3f ( u[0] + v[0],  u[1] + v[1],  u[2] + v[2]);
	glTexCoord2i(0,1); glVertex3f (-u[0] + v[0], -u[1] + v[1], -u[2] + v[2]);
	glTexCoord2i(0,0); glVertex3f (-u[0] - v[0], -u[1] - v[1], -u[2] - v[2]);
	glTexCoord2i(1,0); glVertex3f ( u[0] - v[0],  u[1] - v[1],  u[2] - v[2]);
	glEnd ();
	
	glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity( );
	
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
}

void draw(e *E, unsigned int scene)
{
	float	camera_magnitude;
    char	str[1024];
    
	camera_magnitude = sqrt(E->viewer[0]*E->viewer[0] + E->viewer[1]*E->viewer[1] + E->viewer[2]*E->viewer[2]);
		
	// switch off textures
	glBindTexture(GL_TEXTURE_2D, 0); 
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glPolygonMode( GL_FRONT, GL_FILL);
	
	// ground sphere
	shaderEnable(E->ground);
		// set up vars for the shaders
		SetUniformParameter3f(E->ground, "v3CameraPos", E->viewer[0], E->viewer[1], E->viewer[2]);
		SetUniformParameter3f(E->ground, "v3LightPos", E->light_direction[0], E->light_direction[1], E->light_direction[2]);
		SetUniformParameter3f(E->ground, "v3InvWavelength", m_fWavelength4_inv[0], m_fWavelength4_inv[1], m_fWavelength4_inv[2]);
		SetUniformParameter1f(E->ground, "fCameraHeight", camera_magnitude);
		SetUniformParameter1f(E->ground, "fCameraHeight2", camera_magnitude*camera_magnitude);
		SetUniformParameter1f(E->ground, "fInnerRadius", m_fInnerRadius);
		SetUniformParameter1f(E->ground, "fInnerRadius2", m_fInnerRadius2);
		SetUniformParameter1f(E->ground, "fOuterRadius", m_fOuterRadius);
		SetUniformParameter1f(E->ground, "fOuterRadius2", m_fOuterRadius2);
		SetUniformParameter1f(E->ground, "fKrESun", m_Kr*m_ESun);
		SetUniformParameter1f(E->ground, "fKmESun", m_Km*m_ESun);
		SetUniformParameter1f(E->ground, "fKr4PI", m_Kr4PI);
		SetUniformParameter1f(E->ground, "fKm4PI", m_Km4PI);
		SetUniformParameter1f(E->ground, "fScale", m_fScale);
		SetUniformParameter1f(E->ground, "fScaleDepth", m_fRayleighScaleDepth);
		SetUniformParameter1f(E->ground, "fScaleOverScaleDepth", m_fScaleOverScaleDepth);
		SetUniformParameter1f(E->ground, "g", m_g);
		SetUniformParameter1f(E->ground, "g2", m_g2);
		SetUniformParameter1i(E->ground, "nSamples", m_nSamples);
		SetUniformParameter1f(E->ground, "fSamples", m_nSamples);
		DrawSphere(E, 128, 0.0, 0.0, 0.0,m_fInnerRadius);
	shaderDisable(E->ground);
	
    
    // sky sphere
	shaderEnable(E->atmo);
		SetUniformParameter3f(E->atmo, "v3CameraPos", E->viewer[0], E->viewer[1], E->viewer[2]);
		SetUniformParameter3f(E->atmo, "v3LightPos", E->light_direction[0], E->light_direction[1], E->light_direction[2]);
		SetUniformParameter3f(E->atmo, "v3InvWavelength", m_fWavelength4_inv[0], m_fWavelength4_inv[1], m_fWavelength4_inv[2]);
		SetUniformParameter1f(E->atmo, "fCameraHeight", camera_magnitude);
		SetUniformParameter1f(E->atmo, "fCameraHeight2", camera_magnitude*camera_magnitude);
		SetUniformParameter1f(E->atmo, "fInnerRadius", m_fInnerRadius);
		SetUniformParameter1f(E->atmo, "fInnerRadius2", m_fInnerRadius2);
		SetUniformParameter1f(E->atmo, "fOuterRadius", m_fOuterRadius);
		SetUniformParameter1f(E->atmo, "fOuterRadius2", m_fOuterRadius2);
		SetUniformParameter1f(E->atmo, "fKrESun", m_Kr*m_ESun);
		SetUniformParameter1f(E->atmo, "fKmESun", m_Km*m_ESun);
		SetUniformParameter1f(E->atmo, "fKr4PI", m_Kr4PI);
		SetUniformParameter1f(E->atmo, "fKm4PI", m_Km4PI);
		SetUniformParameter1f(E->atmo, "fScale", m_fScale);
		SetUniformParameter1f(E->atmo, "fScaleDepth", m_fRayleighScaleDepth);
		SetUniformParameter1f(E->atmo, "fScaleOverScaleDepth", m_fScaleOverScaleDepth);
		SetUniformParameter1f(E->atmo, "g", m_g);
		SetUniformParameter1f(E->atmo, "g2", m_g2);
		SetUniformParameter1i(E->atmo, "nSamples", m_nSamples);
		SetUniformParameter1f(E->atmo, "fSamples", m_nSamples);
		glFrontFace(GL_CW);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		DrawSphere(E, 128, 0.0, 0.0, 0.0, m_fOuterRadius);
		glDisable(GL_BLEND);
		glFrontFace(GL_CCW);
	shaderDisable(E->atmo);
			
    
	
	// planet
	glColor3f(1.0,1.0,1.0);
	// Perform scene rotations
	glRotatef( E->theta[0], 1.0, 0.0, 0.0 );
	glRotatef( E->theta[1], 0.0, -1.0, 0.0 );
	glRotatef( E->theta[2], 0.0, 0.0, 1.0 );
	
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, E->texture[E->numLayers-3]);
	glActiveTexture(GL_TEXTURE1);

	glBindTexture(GL_TEXTURE_2D, E->im[scene]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
	shaderEnable(E->texBlend);
	SetUniformParameter1i(E->texBlend,"Texture0", 0);	
	SetUniformParameter1i(E->texBlend,"Texture1", 1);	
	SetUniformParameter1f(E->texBlend,"brightness;", -0.55);
	SetUniformParameter1f(E->texBlend,"contrast", 1.1);			
    
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE,GL_ONE);
	
    DrawSphere(E, 128, 0.0, 0.0, 0.0,m_fInnerRadius+0.005);
	
	glDisable(GL_BLEND);
	shaderDisable(E->texBlend);
	
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
	
	
	if(E->preview != 1){
	// draw logo asset
		draw_logo(E,TOP_RIGHT,E->numLayers-4);
		draw_colorbar(E, E->numLayers-2);
	}
	
	
    // HUD info
	sprintf(str,"%s\n", E->long_name);
	addText(10,E->yWinSize-E->font_size+5,str);	
	
	sprintf(str,"%2d\n",E->day);
	addText2(10,E->yWinSize-E->font_size-E->font_size_small+5,str);
	
	if(E->preview == 1)
		addText2(24,E->yWinSize-E->font_size-E->font_size_small+5,E->date_string);
	else
		addText2(36,E->yWinSize-E->font_size-E->font_size_small+5,E->date_string);
	
		
}

