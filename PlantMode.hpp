#pragma once

#include "Mode.hpp"

#include "BoneAnimation.hpp"
#include "GL.hpp"
#include "Scene.hpp"
#include "Sprite.hpp"
#include "Aura.hpp"
#include "Order.hpp"
#include "Button.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <vector>
#include <list>
#include <unordered_map>
#include <iostream>
#include "Plant.hpp"

struct Inventory
{
	int get_seeds_num( const PlantType* plant );
	void change_seeds_num(const PlantType* plant, int seed_change );

private:
	std::unordered_map<PlantType const*, int> flower_to_seeds;
};

// The 'PlantMode':
struct PlantMode : public Mode {
	PlantMode();
	virtual ~PlantMode();

	bool is_magicbook_open = false;
	int fire_flower_seed_num = 1;
	int fern_seed_num = 1;
	int sapsucker_seed_num = 1;
	int friend_fern_seed_num = 1;
	int cactus_seed_num = 0;
	//called to create menu for current scene:
	void open_book();
	glm::vec2 view_min = glm::vec2(0,0);
	glm::vec2 view_max = glm::vec2(259, 225);
	Scene::Drawable* magic_book = nullptr;
	std::vector< OrderType const* > all_orders;
	int current_order_idx = 0;
	OrderType const* current_order = order1;
	
	// init harvest_plant_map
	// Harvest Plant Map
	std::map< PlantType const*, int > harvest_plant_map;
    
	void on_click( int x, int y );
	GroundTile* get_tile_under_mouse( int x, int y);
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
	virtual void on_resize( glm::uvec2 const& new_drawable_size ) override;
	
	//scene:
	std::string action_description = "";
	const PlantType* selectedPlant = nullptr;
	const PlantType* orderPlant = nullptr;
	Scene scene;
	Scene::Camera *camera = nullptr;
	Scene::Drawable* selector = nullptr;
	
	Inventory inventory;
	int energy = 30;

	glm::vec3 forward_camera_dir = glm::vec3();
	glm::vec3 forward_dir = glm::vec3();

	glm::vec3 side_camera_dir = glm::vec3();
	glm::vec3 side_dir = glm::vec3();

	glm::vec3 camera_bounds_min = glm::vec3(-5.0f, -5.0f,-10000.0f);
	glm::vec3 camera_bounds_max = glm::vec3( 5.0f, 5.0f, 10000.0f );
	glm::vec3 camera_offset = glm::vec3();
	float camera_move_speed = 2.0f;
	float camera_radius = 7.5f;
	float camera_azimuth = glm::radians(125.0f);
	float camera_elevation = glm::radians(40.0f);

	//tool selection
	enum{ glove, watering_can, fertilizer, shovel, seed, none } current_tool = none;

	//UI states:
	struct {
		bool hidden = false;
		// tools
		std::vector<Button> tools = {};
		// storage (seed, harvest)
		struct {
			bool hidden = true;
			enum { seeds, harvest } tab = seeds;
		} storage;
		// magicbook
		struct {
			bool hidden = true;
			std::vector<Button> items;
		} magicbook;
	} UI;

	struct {
		Sprite const* sprite = nullptr;
		std::string text = ""; // TODO: text that floats around cursor?
		float scale = 1.0f;
		glm::vec2 offset = glm::vec2(0, 0);// applied to cursor sprite _before_ scaling
	} cursor;

	//-------- opengl stuff 

	glm::vec2 screen_size = glm::vec2(960, 600); 
	GLuint color_attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	GLuint firstpass_fbo = 0;
	GLuint firstpass_color_attachments[2]; 
	GLuint firstpass_depth_attachment = 0;

	GLuint aura_fbo = 0; // shares depth attachment with firstpass_fbo
	GLuint aura_color_attachment = 0;

	GLuint pingpong_fbos[2];
	GLuint pingpong_color_attachments[2];

	std::vector<float> trivial_vector = {
		-1, -1, 0,
		-1, 1, 0,
		1, 1, 0,
		-1, -1, 0,
		1, 1, 0,
		1, -1, 0
	};

	GLuint trivial_vao = 0;
	GLuint trivial_vbo = 0;

	//--------
};
