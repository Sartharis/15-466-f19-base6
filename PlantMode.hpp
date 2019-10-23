#pragma once

#include "Mode.hpp"

#include "BoneAnimation.hpp"
#include "GL.hpp"
#include "Scene.hpp"

#include <SDL.h>
#include <glm/glm.hpp>

#include <vector>
#include <list>

/* Contains info on how a plant works and looks like*/
struct PlantType
{
	PlantType(const Mesh* mesh_in );
	const Mesh* get_mesh() const;

private:
	const Mesh* mesh = nullptr;
};

/* Contains info on how a tile works and looks like*/
struct GroundTileType
{
	GroundTileType( bool can_plant_in, const Mesh* tile_mesh_in );
	const Mesh* get_mesh() const;
	bool get_can_plant() const;

private:
	bool can_plant = true;
	const Mesh* mesh = nullptr;
};

/* Actual instance of a tile with a plant */
struct GroundTile
{
	void change_tile_type( const GroundTileType* tile_type_in );
	bool try_add_plant(const PlantType* plant_type_in );
	bool try_remove_plant();

	// Tile and plant types
	const GroundTileType* tile_type = nullptr;
	const PlantType* plant_type = nullptr;
	Scene::Drawable* tile_drawable = nullptr;
	Scene::Drawable* plant_drawable = nullptr;

	// Tile data
	int grid_x = 0;
	int grid_y = 0;

	// Plant data
};

// The 'PlantMode':
struct PlantMode : public Mode {
	PlantMode();
	virtual ~PlantMode();


	virtual bool handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//scene:
	Scene scene;
	Scene::Camera *camera = nullptr;

	GroundTile** grid = nullptr;
	glm::vec2 plant_grid_tile_size = glm::vec2( 2.0f, 2.0f );
	int plant_grid_x = 20;
	int plant_grid_y = 20;

	float camera_radius = 15.0f;
	float camera_azimuth = glm::radians(90.0f);
	float camera_elevation = glm::radians(45.0f);
};
