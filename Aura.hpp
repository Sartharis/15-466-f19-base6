#pragma once

#include "Scene.hpp"
#include <glm/glm.hpp>

#include <vector>
#include <list>

// manages aura dots for a tile location (or, manage aura for all tiles?)
struct Aura { 

	enum Type { fire, aqua };

	struct Dot {
		Dot(glm::vec3 _center);
		void update_position(float elapsed);
		// states
		float timer;
		glm::vec3 position = glm::vec3(0, 0, 1.2);
		glm::vec3 center;
		float float_radius, float_height, float_azimuth;
		float float_speed_horizontal, float_speed_vertical;
		float dot_radius = 0.2f;
	};

	struct Vertex {
		Vertex(glm::vec3 _pos, Type type) : position(_pos) {
			switch (type) {
				case fire:
					color = glm::u8vec4(255, 42, 40, 255);
					break;
				case aqua:
					color = glm::u8vec4(50, 135, 255, 255);
					break;
			}
		}
		glm::vec3 position;
		glm::vec2 tex_coord = glm::vec2(0, 0);
		glm::u8vec4 color = glm::u8vec4(255, 255, 255, 255);
	};

	Aura(glm::vec3 _center, Type _type);
	void update(float elapsed, Scene::Transform* cam);
	void draw(glm::mat4 world_to_clip);
	
	// states
	Type type;
	int num_dots = 16; // maximum strength
	int strength = 10;
	glm::vec3 center;
	// internals
	std::vector<Dot> dots;
	std::vector<Vertex> dots_vbo;
	// opengl-related stuff
	GLuint vao, vbo, white_tex;
};
