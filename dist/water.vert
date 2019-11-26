#version 330

uniform mat4 OBJECT_TO_CLIP;
uniform mat4x3 OBJECT_TO_LIGHT;
uniform mat3 NORMAL_TO_LIGHT;
in vec4 Position;
in vec3 Normal;
in vec4 Color;
in vec2 TexCoord;

out vec2 pos;

void main() {
	gl_Position = OBJECT_TO_CLIP * Position;
	pos = ( OBJECT_TO_LIGHT * Position ).xy;
}
