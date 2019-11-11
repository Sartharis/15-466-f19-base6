#version 330

uniform mat4 OBJECT_TO_CLIP;
uniform mat4x3 OBJECT_TO_LIGHT;
uniform mat3 NORMAL_TO_LIGHT;
// uniform float HEALTH;
uniform vec3 PROPERTIES;
in vec4 Position;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;
out vec3 position;
out vec3 normal;
out vec4 color;
out vec2 texCoord;

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

vec4 color_from_health(vec4 col, float health) {
	if (health == 1.0f) return col;

	// overlay with tint color
	vec4 overlay = over(vec4(0.6078, 0.3255, 0, 0.75), col);
	// preserve luminance
	vec4 l = overlay * ( luminance(col) / luminance(overlay) );
	vec4 l_clamped = vec4( min(1, l.r), min(1, l.g), min(1, l.b), min(1, l.a) );// TODO
	// multiply to make darker
	vec4 multiply = l_clamped * 0.3;
	// full unhealthy color
	vec4 unhealthy = multiply;

	// lerp to get result
	return mix(unhealthy, col, health);
}

vec4 soil_color(vec4 col, float moisture, float fertility) {
	if (moisture == 0.0f && fertility == 0.0f) return col;
	vec4 wet_fertile_col = vec4(51.0f/255.0f, 39.0f/255.0f, 28.0f/255.0f, 1.0f);
	vec4 dry_barren_col = vec4(151.0f/255.0f, 137.0f/255.0f, 115.0f/255.0f, 1.0f);
	vec4 wet_barren_col = vec4(99.0f/255.0f, 92.0f/255.0f, 82.0f/255.0f, 1.0f);
	vec4 all_wet = mix(wet_barren_col, wet_fertile_col, fertility);
	vec4 all_dry = mix(dry_barren_col, col, fertility);
	return mix(all_dry, all_wet, moisture);
}

void main() {
	float health = PROPERTIES.x;
	float moisture = PROPERTIES.y;
	float fertility = PROPERTIES.z;

	gl_Position = OBJECT_TO_CLIP * Position;
	position = OBJECT_TO_LIGHT * Position;
	normal = NORMAL_TO_LIGHT * Normal;
  color = color_from_health(Color, health);
	color = soil_color(color, moisture, fertility);
	texCoord = TexCoord;
}
