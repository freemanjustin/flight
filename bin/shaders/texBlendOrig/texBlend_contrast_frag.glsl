uniform sampler2D Texture0;
uniform sampler2D Texture1;
varying vec2 TexCoord0;

// Brightness
uniform float brightness;

// Contrast
uniform float contrast;

void main()
{  
	vec4 texel0, texel1, resultColor;

	texel0 = texture2D(Texture0, TexCoord0);
	texel1 = texture2D(Texture1, TexCoord0);
	
	// original (and correct!)
	resultColor = mix(texel0, texel1, (1.0-texel0.a));
		
	resultColor += brightness;
	resultColor = (resultColor-0.5)*contrast + 0.5;
		
	gl_FragColor = gl_Color * resultColor;
	
	
	
}

