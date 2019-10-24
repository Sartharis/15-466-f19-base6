#version 330

layout(location = 0) in vec3 Position;
out vec2 TexCoords;

void main() {
	gl_Position = vec4(Position, 1);
  TexCoords = (Position.xy + vec2(1, 1)) / 2;
}
