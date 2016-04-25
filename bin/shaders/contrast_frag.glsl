// brightness_contrast.frag

/*
  This filter allows the user to tweak the overall
  brightness and contrast levels of an image.
*/

// Get the image-space position generated in the vertex program.
varying vec2 pos;

// The input image we will be filtering in this kernel.
uniform sampler2D image;

// Brightness
uniform float brightness;

// Contrast
uniform float contrast;

void main()
{
    // Sample the input image at the output position.
    vec4 c = texture2D(image, pos);

    // Add in the contribution of brightness adjustment
    // (positive or negative).
    c += brightness;

    // Expand or contract the color range about 0.5
    // to increase or decrease contrast.
    c = (c-0.5)*contrast + 0.5;

    gl_FragColor = gl_Color * c;
}
