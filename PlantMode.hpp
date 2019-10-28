#pragma once

#include "Mode.hpp"

#include "BoneAnimation.hpp"
#include "GL.hpp"
#include "Scene.hpp"
#include "Sprite.hpp"
#include "Aura.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <vector>
#include <list>
#include <iostream>

/* Contains info on how a plant works and looks like*/
struct PlantType
{
	PlantType( const Mesh* mesh_in,
				 Aura::Type aura_type_in,
				 int cost_in = 5,
				 bool is_harvestable_in = true,
			   int harvest_gain_in = 7,
			   float growth_time_in = 5.0f, 
			   std::string name_in = "Default Name", 
			   std::string description_in = "Default Description." )
	 :mesh(mesh_in), 
	  aura_type(aura_type_in),
		growth_time(growth_time_in), 
		cost(cost_in), 
		is_harvestable(is_harvestable_in),
		harvest_gain(harvest_gain_in),
		name(name_in), 
		description(description_in) {};

	const Mesh* get_mesh() const { return mesh; };
	Aura::Type get_aura_type() const { return aura_type; };
	float get_growth_time() const { return growth_time; };
	int get_cost() const { return cost; };
	bool get_harvestable() const { return is_harvestable; }
	int get_harvest_gain() const { return harvest_gain; };
	std::string get_name() const { return name; };
	std::string get_description() const { return description; };

private:
	// TODO: each plant type should have multiple meshes attached (always 3?)
	const Mesh* mesh = nullptr;
	Aura::Type aura_type = Aura::none;
	float growth_time = 5.0f;
	int cost = 5;
	bool is_harvestable = true;
	int harvest_gain = 7;
	std::string name = "Default Name";
	std::string description = "Default Description.";
};

/* Contains info on how a tile works and looks like*/
struct GroundTileType
{
	GroundTileType( bool can_plant_in, const Mesh* tile_mesh_in ) : can_plant( can_plant_in ), mesh( tile_mesh_in ){};
	const Mesh* get_mesh() const{ return mesh; };
	bool get_can_plant() const { return can_plant; };

private:
	bool can_plant = true;
	const Mesh* mesh = nullptr;
};

/* Actual instance of a tile with a plant */
struct GroundTile
{
	void change_tile_type( const GroundTileType* tile_type_in );
	void update( float elapsed, Scene::Transform* camera_transform );
	void update_plant_visuals( float percent_grown );
	void apply_pending_update();
	bool try_add_plant(const PlantType* plant_type_in );
	bool try_remove_plant();
	bool try_remove_aura();
	bool is_tile_harvestable();

	// Tile and plant types
	const GroundTileType* tile_type = nullptr;
	const PlantType* plant_type = nullptr;
	Scene::Drawable* tile_drawable = nullptr;
	Scene::Drawable* plant_drawable = nullptr;

	// Tile data. TODO: other properties like fertility?
	int grid_x = 0;
	int grid_y = 0;
	float fire_aura_effect = 0.0f; // in range 0 - 1
	float aqua_aura_effect = 0.0f;
	// each time a tile updates, its aura modifies nearby tiles' pending update struct
	// which gets applied at the end of update call
	struct {
		float fire_aura_effect = 0.0f;
		float aqua_aura_effect = 0.0f;
	} pending_update;

	// Plant data
	float current_grow_time = 0.0f;

	// Aura 
	Aura* aura = nullptr;

	//TEMP!!!!!
	float start_height = -0.4f;
	float end_height = 0.0f;
};

// The 'PlantMode':
struct PlantMode : public Mode {
	PlantMode();
	virtual ~PlantMode();

	void on_click( int x, int y );
	GroundTile* get_tile_under_mouse( int x, int y);
	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//scene:
	std::string action_description = "";
	std::string tile_status_summary = ""; // TEMP
	const PlantType* selectedPlant = nullptr;
	Scene scene;
	Scene::Camera *camera = nullptr;

	GroundTile** grid = nullptr;
	glm::vec2 plant_grid_tile_size = glm::vec2( 1.0f, 1.0f );
	int plant_grid_x = 20;
	int plant_grid_y = 20;

	int energy = 20;

	float camera_radius = 7.5f;
	float camera_azimuth = glm::radians(125.0f);
	float camera_elevation = glm::radians(40.0f);

	//-------- opengl stuff 

	// TODO: if want to allow resize, have to find a better way to pass this
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
