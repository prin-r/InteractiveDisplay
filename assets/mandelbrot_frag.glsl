#version 120

// Sets the maximum number of iterations per pixel.
// Note: anything above 256 is a waste of energy,
//       because of the limited floating point precision.

uniform sampler2D tex;
uniform vec2      center;
uniform float     uAspectRatio = 1.33333;
uniform float     scale = 0.01;
uniform int iter = 1;
uniform bool	isWhite = false;

float clamp(float a) {
	if (a > 1.0f) return 1.0f;
	if (a < 0.0f) return 0.0f;
	return a;
}

void main() {
	vec2 texCoord = vec2( 0 );
	vec2 z = vec2(0);
	z.x = ( gl_TexCoord[0].x ) / scale;
	z.y = ( gl_TexCoord[0].y ) / scale;
	vec4 cTest = (vec4(1.0,1.0,1.0,z.x-0.5f));
	vec4 color = cTest;
	if (isWhite) color = vec4(1.0,1.0,1.0,1.0);
	
	float a = texture2D( tex, vec2(gl_TexCoord[0].x,gl_TexCoord[0].y)).a;
	vec3 ttt = vec3(a,a,a);
	//gl_FragColor = vec4(vec3(0,0,0),a*(z.x-0.5f));
	
	//float k = sin(pow(gl_TexCoord[0].x*2.0f,4));
	
	float k = cos(gl_TexCoord[0].x * gl_TexCoord[0].x * 200);
	k = clamp(35.0f - (k + z.y*2.0f)*10.0f);
	
	gl_FragColor = vec4(vec3(0,0,0),clamp(a*k));
	
	//gl_FragColor = vec4(vec3(1,1,1),1.0f - z.y);
	
}
















