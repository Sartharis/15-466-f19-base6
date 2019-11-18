#include "Aura.hpp"
#include "ColorTextureProgram.hpp"

#include "gl_errors.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

static GLuint vbo = 0;
static GLuint vao = 0;
static GLuint white_tex = 0;

static Load< void > setup_gl(LoadTagDefault, [](){

	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	glVertexAttribPointer(
		color_texture_program->Position_vec4,
		3, // size
		GL_FLOAT, // type
		GL_FALSE, // normalized
		sizeof(Aura::Vertex), // stride size
		(GLbyte*) 0 + offsetof(Aura::Vertex, position) // offset
	);
	glEnableVertexAttribArray(color_texture_program->Position_vec4);

	glVertexAttribPointer(
		color_texture_program->TexCoord_vec2, //attribute
		2, //size
		GL_FLOAT, //type
		GL_FALSE, //normalized
		sizeof(Aura::Vertex), // stride size
		(GLbyte *) 0 + offsetof(Aura::Vertex, tex_coord) //offset
	);
	glEnableVertexAttribArray(color_texture_program->TexCoord_vec2);

	glVertexAttribPointer(
		color_texture_program->Color_vec4, //attribute
		4, //size
		GL_UNSIGNED_BYTE, //type
		GL_TRUE, //normalized
		sizeof(Aura::Vertex), //stride
		(GLbyte *) 0 + offsetof(Aura::Vertex, color) //offset
	);
	glEnableVertexAttribArray(color_texture_program->Color_vec4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//--- create 1x1 white texture
	glGenTextures(1, &white_tex);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	glm::uvec2 size = glm::uvec2(1,1);
	std::vector<glm::u8vec4> data(size.x*size.y, glm::u8vec4(255,255,255,255));
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	
});

Aura::Aura(glm::vec3 _center, Type _type, int _max_strength) : type(_type), max_strength(_max_strength), center(_center) {
	assert(_type != none);

	dots = std::vector<Dot>();
	for (int i=0; i<max_strength; i++) {
		dots.emplace_back(_center, type);
	}

}

inline float rand5() {
	return float(rand() % 10000) / 10000.0f;
}

Aura::Dot::Dot(glm::vec3 _center, Aura::Type type) : center(_center) {
	timer = rand5() * 6.2832f;
	dot_radius = rand5() * 0.03f + 0.015f;
	float_height = rand5() * 0.6f + 0.2f;
	float_azimuth = rand5() * 2.0f * 3.1415926535f;
	float_radius = (type==Aura::fire || type==Aura::aqua) ? rand5() * 0.4f + 0.1f : rand5() * 0.8f + 0.2f;
	// float_speed_horizontal = glm::radians(rand5() * 30.0f + 45.0f);
	float_speed_vertical = rand5() * 3.0f + 1.0f;
	position = glm::vec3( cos(float_azimuth), sin(float_azimuth), float_height );
}

void Aura::Dot::update_position_jump(float elapsed) {
	timer += elapsed;
	position = center + float_radius * glm::vec3( 
		cos(float_azimuth), sin(float_azimuth), 
		float_height + 0.15f * sin(timer * float_speed_vertical) );
}

void Aura::Dot::update_position_outward(float elapsed) {
	timer += elapsed;
	float_radius += elapsed * 0.15f;
	if(float_radius > 1.0f) float_radius -= 0.8f;
	position = center + float_radius * glm::vec3( 
		cos(float_azimuth), sin(float_azimuth), 
		float_height + 0.15f * sin(timer * float_speed_vertical) );
}

void Aura::Dot::update_position_inward(float elapsed) {
	timer += elapsed;
	float_radius -= elapsed * 0.15f;
	if(float_radius < 0.2f) float_radius += 0.8f;
	position = center + float_radius * glm::vec3( 
		cos(float_azimuth), sin(float_azimuth), 
		float_height + 0.15f * sin(timer * float_speed_vertical) );
}

void Aura::update(int _strength, float elapsed, Scene::Transform* cam) {
	strength = _strength;
	assert(strength <= dots.size());

	cam_transform = cam;

	if( type == Aura::fire || type == Aura::aqua ) {
		for(int i=0; i<strength; i++) {
			dots[i].update_position_jump(elapsed);
		}
	} else if( type == Aura::help ) {
		for(int i=0; i<strength; i++) {
			dots[i].update_position_outward(elapsed);
		}
	} else if( type == Aura::suck ) {
		for(int i=0; i<strength; i++) {
			dots[i].update_position_inward(elapsed);
		}
	}
}

void Aura::draw(DrawAura &draw_aura) {

	assert( cam_transform );
	
	for(int i=0; i<strength; i++) {
		// turn it to face the camera
		glm::vec3 c = dots[i].position; // center of dot
		float r = dots[i].dot_radius;
		glm::quat rotation = cam_transform->rotation;
		Vertex tl = Vertex(r * (rotation * glm::vec3(-1, 1, 0)) + c, type);
		Vertex tr = Vertex(r * (rotation * glm::vec3(1, 1, 0)) + c, type);
		Vertex bl = Vertex(r * (rotation * glm::vec3(-1, -1, 0)) + c, type);
		Vertex br = Vertex(r * (rotation * glm::vec3(1, -1, 0)) + c, type);
		// make the square and append it to vbo
		std::vector<Vertex> billboard_this = { tl, bl, br, tl, br, tr };	
		draw_aura.vertices.insert(draw_aura.vertices.end(), billboard_this.begin(), billboard_this.end());
	}

}

DrawAura::~DrawAura() {

	// draw the dots w ColorTextureProgram
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Aura::Vertex), vertices.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(color_texture_program->program);
	glUniformMatrix4fv(
			color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(world_to_clip));

	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	glDrawArrays(GL_TRIANGLES, 0, GLsizei( vertices.size() ));

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GL_ERRORS();

}
