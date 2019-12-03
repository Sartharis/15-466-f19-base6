#pragma once

#include "Scene.hpp"
#include <glm/glm.hpp>

#include <vector>
#include <list>
#include <iostream>

// forward declaration
struct DrawAura;

// manages aura dots for a tile location (TODO: manage aura for all tiles? Or make it AuraType instead?)
struct Aura { 

	enum Type { fire, aqua, beacon, help, suck, none };

	struct Dot {
		Dot(glm::vec3 _center, Aura::Type type);
		void update_position_jump(float elapsed);
		void update_position_outward(float elapsed);
		void update_position_inward(float elapsed);
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
				case beacon:
					color = glm::u8vec4(153, 89, 148, 255);
					break;
				case help:
					color = glm::u8vec4(242, 236, 143, 255);
					break;
				case suck:
					color = glm::u8vec4(94, 63, 138, 255);
					break;
				default:
					std::cout << "WARNING: non-exhaustive match of aura type??" << std::endl;
					break;
			}
		}
		glm::vec3 position;
		glm::vec2 tex_coord = glm::vec2(0, 0);
		glm::u8vec4 color;
	};

	Aura(glm::vec3 _center, Type _type, int _max_strength = 5);
	void update(int _strength, float elapsed, Scene::Transform* cam);
	void draw(DrawAura &draw_aura);
	
	// states
	Type type;
	int max_strength = 5; // num dots
	int strength = 0;
	glm::vec3 center;

	// internals
	std::vector<Dot> dots;
	Scene::Transform* cam_transform = nullptr;
	// std::vector<Vertex> dots_vbo;

	// opengl-related stuff
	GLuint vao, vbo, white_tex;
};

struct DrawAura {

	DrawAura( glm::mat4 _world_to_clip ) : world_to_clip(_world_to_clip) {}
	~DrawAura(); // actual drawing

	// internals
	std::vector<Aura::Vertex> vertices = {};
	glm::mat4 world_to_clip;
	// GLuint vao, vbo, white_tex;

};
