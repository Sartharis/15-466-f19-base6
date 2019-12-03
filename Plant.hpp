#pragma once

#include "Aura.hpp"
#include "Sprite.hpp"
#include "UIElem.hpp"
#include "Load.hpp"
#include <vector>
#include "Mesh.hpp"
#include <glm/glm.hpp>

struct TileGrid;

enum Tool { default_hand, watering_can, fertilizer, shovel, seed };

/* Contains info on how a plant works and looks like*/
struct PlantType
{
	PlantType( const std::vector<const Mesh*> meshes_in,
				 Sprite const* seed_sprite_in,
				 Sprite const* harvest_sprite_in,
			   Aura::Type aura_type_in,
			   int cost_in = 5,
			   int harvest_gain_in = 7,
			   float growth_time_in = 5.0f,
			   std::string name_in = "Default Name",
			   std::string description_in = "Default Description." )
		:meshes( meshes_in ),
		seed_sprite( seed_sprite_in ),
		harvest_sprite( harvest_sprite_in ),
		aura_type( aura_type_in ),
		growth_time( growth_time_in ),
		cost( cost_in ),
		harvest_gain( harvest_gain_in ),
		name( name_in ),
		description( description_in ) {
		assert( meshes.size() > 0 );
	};

	const Mesh* get_mesh( float percent_grown ) const {
		return meshes[get_growth_stage( percent_grown )];
	};
	float get_stage_percent( float percent_grown ) const {
		float float_index = ( glm::max( 0.0f, percent_grown ) * ( meshes.size() - 1 ) );
		return percent_grown >= 1.0f ? 1.0f : float_index - floor( float_index );
	};
	int get_growth_stage( float percent_grown ) const {
		return percent_grown >= 1.0f ?
			int( meshes.size() ) - 1 :
			int( floor( glm::max( 0.0f, percent_grown ) * ( meshes.size() - 1 ) ) );
	}

	Aura::Type get_aura_type() const { return aura_type; };
	float get_growth_time() const { return growth_time; };
	int get_cost() const { return cost; };
	int get_harvest_gain() const { return harvest_gain; };
	std::string get_name() const { return name; }
	std::string get_description() const { return description; };
	Sprite const* get_seed_sprite() const { return seed_sprite; };
	Sprite const* get_harvest_sprite() const { return harvest_sprite; };
	void make_menu_items(const PlantType** selectedPlant, Tool* current_tool,
			UIElem** seed_item, UIElem** harvest_item ) const;

private:
	// TODO: each plant type should have multiple meshes attached (always 3?)
	const std::vector<const Mesh*> meshes = {};
	Sprite const* seed_sprite = nullptr;
	Sprite const* harvest_sprite = nullptr;
	Aura::Type aura_type = Aura::none;
	float growth_time = 5.0f;
	int cost = 5;
	int harvest_gain = 7;
	std::string name = "Default Name";
	std::string description = "Default Description.";
};

/* Contains info on how a tile works and looks like*/
struct GroundTileType
{
	GroundTileType( bool can_plant_in, const Mesh* tile_mesh_in, int clear_cost_in ) : clear_cost(clear_cost_in), can_plant( can_plant_in ), mesh( tile_mesh_in ){};
	int get_clear_cost() const{ return clear_cost; };
	const Mesh* get_mesh() const{ return mesh; };
	bool get_can_plant() const { return can_plant; };

private:
	int clear_cost = 40;
	bool can_plant = true;
	const Mesh* mesh = nullptr;
};

/* Actual instance of a tile with a plant */
struct GroundTile
{
	void change_tile_type( const GroundTileType* tile_type_in );
	void update( float elapsed, Scene::Transform* camera_transform, const TileGrid& grid );
	void update_plant_visuals();
	void apply_pending_update( float elapsed );
	void update_aura_visuals( float elapsed, Scene::Transform* camera_transform );
	
	static bool try_swap_plants(GroundTile& tile_a, GroundTile& tile_b );
	bool try_add_plant( const PlantType* plant_type_in );
	bool try_remove_plant();
	
	bool is_tile_harvestable();
	bool is_plant_dead();
	
	bool can_be_cleared(const TileGrid& grid) const;
	bool try_clear_tile();
	bool is_cleared() const;
	
	void change_health( float change );

	// Tile and plant types
	const GroundTileType* tile_type = nullptr;
	const PlantType* plant_type = nullptr;
	Scene::Drawable* tile_drawable = nullptr;
	Scene::Drawable* plant_drawable = nullptr;

	// Tile data
	int grid_x = 0;
	int grid_y = 0;

	const float plant_health_restore_rate = 1.0f / 5.0f;
	const float plant_health_fertilization_restore_rate = 1.0f / 3.0f;
	const float moisture_dry_rate = 0.01f;

	float plant_health = 1.0f;
	float moisture = 1.0f;
	float fertilization = 0.0f;

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
	Aura* fire_aura = nullptr;
	Aura* aqua_aura = nullptr;
	Aura* help_aura = nullptr;
	Aura* suck_aura = nullptr;
	Aura* beacon_aura = nullptr;

};


struct TileGrid
{
	GroundTile** tiles;
	int size_x;
	int size_y;

	bool is_in_grid( int x, int y ) const;
};

const int fertilization_cost = 10;
const float fertilization_duration = 5.0f;
extern const MeshBuffer* plant_mesh_buffer;
extern Mesh const* sea_mesh;
extern glm::vec2 plant_grid_tile_size;
TileGrid setup_grid_for_scene( Scene& scene, int plant_grid_x, int plant_grid_y );
extern PlantType const* test_plant;
extern PlantType const* friend_plant;
extern PlantType const* vampire_plant;
extern PlantType const* cactus_plant;
extern PlantType const* fireflower_plant;
extern PlantType const* waterflower_plant;
extern PlantType const* beaconflower_plant;
extern PlantType const* corpseeater_plant;
extern PlantType const* spreader_source_plant;
extern PlantType const* spreader_child_plant;
extern PlantType const* teleporter_plant;
extern GroundTileType const* sea_tile;
extern std::vector< PlantType const* > all_plants;
extern GroundTileType const* ground_tile;
extern GroundTileType const* grass_short_tile;
extern GroundTileType const* grass_tall_tile;
extern GroundTileType const* dirt_tile;
extern GroundTileType const* empty_tile;
extern Load< GLuint > plant_meshes_for_firstpass_program;
extern Load< GLuint > plant_meshes_for_water_program;
