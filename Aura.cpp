#include "Aura.hpp"
#include "ColorTextureProgram.hpp"

#include "gl_errors.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Aura::Aura(glm::vec3 _center, Type _type) : type(_type), center(_center) {
	assert(_type != none);

	dots = std::vector<Dot>();
	for (int i=0; i<num_dots; i++) {
		dots.emplace_back(_center);
	}

	{ // opengl related
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
	}
}

inline float rand5() {
	return float(rand() % 10000) / 10000.0f;
}

Aura::Dot::Dot(glm::vec3 _center) : center(_center) {
	timer = rand5() * 6.2832f;
	dot_radius = rand5() * 0.05f + 0.02f;
	float_height = rand5() * 0.8f + 0.3f;
	float_azimuth = rand5() * 2.0f * 3.1415926535f;
	float_radius = rand5() * 0.5f + 0.3f;
	float_speed_horizontal = glm::radians(rand5() * 30.0f + 45.0f);
	float_speed_vertical = rand5() * 2.0f + 3.0f;
	position = glm::vec3( cos(float_azimuth), sin(float_azimuth), float_height );
}

void Aura::Dot::update_position(float elapsed) {
	timer += elapsed;
	float_azimuth += elapsed * float_speed_horizontal;
	position = center + float_radius * glm::vec3( 
		cos(float_azimuth), sin(float_azimuth), 
		float_height + 0.15f * sin(timer * float_speed_vertical) );
}

void Aura::update(float elapsed, Scene::Transform* cam) {
	dots_vbo = std::vector<Vertex>();
	assert(strength <= dots.size());
	for(int i=0; i<strength; i++) {
		// update position..
		dots[i].update_position(elapsed);
		// then turn it to face the camera
		glm::vec3 c = dots[i].position; // center of dot
		float r = dots[i].dot_radius;
		glm::quat rotation = cam->rotation;
		Vertex tl = Vertex(r * (rotation * glm::vec3(-0.5, 0.5, 0)) + c, type);
		Vertex tr = Vertex(r * (rotation * glm::vec3(0.5, 0.5, 0)) + c, type);
		Vertex bl = Vertex(r * (rotation * glm::vec3(-0.5, -0.5, 0)) + c, type);
		Vertex br = Vertex(r * (rotation * glm::vec3(0.5, -0.5, 0)) + c, type);
		// make the square and append it to vbo
		std::vector<Vertex> billboard_this = { tl, bl, br, tl, br, tr };	
		dots_vbo.insert(dots_vbo.end(), billboard_this.begin(), billboard_this.end());
	}
}

void Aura::draw(glm::mat4 world_to_clip) {

	// draw the dots w ColorTextureProgram
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, dots_vbo.size() * sizeof(Vertex), dots_vbo.data(), GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(color_texture_program->program);
	glUniformMatrix4fv(
			color_texture_program->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(world_to_clip));

	glBindVertexArray(vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, white_tex);

	glDrawArrays(GL_TRIANGLES, 0, GLsizei(dots_vbo.size() * sizeof(Vertex)));

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GL_ERRORS();
}

