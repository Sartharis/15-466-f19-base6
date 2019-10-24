#version 330

uniform sampler2D TEX;
in vec3 position;
in vec3 normal;
in vec4 color;
in vec2 texCoord;
layout(location = 0) out vec4 outColor0;
layout(location = 1) out vec4 outColor1;

vec4 over(vec4 elem, vec4 canvas) {
  vec4 elem_ = vec4(elem.rgb * elem.a, elem.a);
  float ca = 1 - (1-elem_.a) * (1-canvas.a);
  float cr = (1-elem_.a) * canvas.r + elem_.r;
  float cg = (1-elem_.a) * canvas.g + elem_.g;
  float cb = (1-elem_.a) * canvas.b + elem_.b;
  return vec4(cr, cg, cb, ca);
}

void main() {
  outColor1 = vec4(0, 0, 0, 0);

  // fixed directional lighting
	vec3 n = normalize(normal);
	vec3 l = normalize(vec3(0.4, 0.2, 1.0));
	vec4 albedo = texture(TEX, texCoord) * color;
  vec3 reflectance = albedo.rgb / 3.1415926535;
  
  // toon shade it
  float threshold = 0.5f;
  outColor0 = albedo;
  if (dot(n,l) < threshold) {
    // in shadow
    outColor1 = albedo * vec4(0.8, 0.75, 0.7, 1);
  } 
}
