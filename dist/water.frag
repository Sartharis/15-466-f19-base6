#version 330

uniform float TIME;
uniform sampler2D DEPTH;
uniform vec2 CANVAS_SIZE;
in vec2 pos;
layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;

float luminance(vec4 col) {
	return col.a * ( col.r*0.299 + col.g*0.587 + col.b*0.114 );
}

vec4 over(vec4 elem, vec4 canvas) {
  vec4 elem_ = vec4(elem.rgb * elem.a, elem.a);
  float ca = 1 - (1-elem_.a) * (1-canvas.a);
  float cr = (1-elem_.a) * canvas.r + elem_.r;
  float cg = (1-elem_.a) * canvas.g + elem_.g;
  float cb = (1-elem_.a) * canvas.b + elem_.b;
  return vec4(cr, cg, cb, ca);
}

/* Perlin noise implementation
 * originally published at: http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf
 * I took the implementation code from: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
 */
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec3 P){
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 / 7.0;
  vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 / 7.0;
  vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

float linearize(float raw_depth) {
  float zNear = 0.01; // TODO: Replace by the zNear of your perspective projection
  float zFar  = 18.0; // TODO: Replace by the zFar  of your perspective projection
  return (2.0 * zNear) / (zFar + zNear - raw_depth * (zFar - zNear));
}

void main() {
	// big wave
	float noise_L = cnoise( vec3(pos.x / 4.0f , pos.y / 4.0f , TIME / 10.0f) );
	vec3 dark_L = vec3( 80.0f / 255.0f, 131.0f / 255.0f, 195.0f / 255.0f );
	vec3 light_L = vec3( 97.0f / 255.0f, 145.0f / 255.0f, 203.0f / 255.0f );
	vec4 L = (noise_L < -0.2f || noise_L > 0.3f) ? vec4(dark_L, 1) : vec4(light_L, 1);
	// small wave
	float noise_S = cnoise(  vec3(pos.x / 1.5f , pos.y / 1.5f , TIME / 5.0f) );
	vec3 col_S = vec3( 184.0f / 255.0f, 211.0f / 255.0f, 226.0f / 255.0f );
	vec4 S = (noise_S > 0.15f && noise_S < 0.2f) ? vec4(col_S, 1) : vec4(0,0,0,0);

	vec2 texcoords = vec2( gl_FragCoord.x / CANVAS_SIZE.x, gl_FragCoord.y / CANVAS_SIZE.y );

	float land_d = linearize(texture( DEPTH, texcoords ).x);
	float sea_d = linearize(gl_FragCoord.z);
	float diff = land_d - sea_d;

	vec4 land_ca = vec4( 207.0f / 255.0f, 183.0f / 255.0f, 145.0f / 255.0f, 1.0f );
	vec3 sea_c = L.rgb;
	float sea_a = 1.0f;
	if (diff < 0.05) sea_a = 0.9f;
	if (diff < 0.04) sea_a = 0.7f;
	if (diff < 0.03) sea_a = 0.5f;

	float wave_width = (sin(TIME) + 1) * 0.002 + 0.001;
	if (diff < wave_width) { 
		sea_a = 1.0f;
		sea_c = col_S;
	}
	outColor1 = over( S, over( vec4(sea_c, sea_a), land_ca ) );
	outColor0 = vec4(0, 0, 0, 0);
}
