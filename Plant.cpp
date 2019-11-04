#include "Plant.hpp"
#include "FirstpassProgram.hpp"
#include "PostprocessingProgram.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include <cstddef>
#include <algorithm>
#include <iostream>

const MeshBuffer* plant_mesh_buffer;
glm::vec2 plant_grid_tile_size = glm::vec2( 1.0f, 1.0f );
PlantType const* test_plant = nullptr;
PlantType const* friend_plant = nullptr;
PlantType const* vampire_plant = nullptr;
PlantType const* carrot_plant = nullptr;
PlantType const* cactus_plant = nullptr;
PlantType const* fireflower_plant = nullptr;
GroundTileType const* sea_tile = nullptr;
GroundTileType const* ground_tile = nullptr;
GroundTileType const* obstacle_tile = nullptr;

// ground tiles
Mesh const* sea_tile_mesh = nullptr;
Mesh const* ground_tile_mesh = nullptr;
Mesh const* obstacle_tile_mesh = nullptr;

// test plant (fern)
Mesh const* test_plant_1_mesh = nullptr;
Mesh const* test_plant_2_mesh = nullptr;
// friend plant
Mesh const* friend_plant_1_mesh = nullptr;
Mesh const* friend_plant_2_mesh = nullptr;
Mesh const* friend_plant_3_mesh = nullptr;
// vampire plant
Mesh const* vampire_plant_1_mesh = nullptr;
Mesh const* vampire_plant_2_mesh = nullptr;
Mesh const* vampire_plant_3_mesh = nullptr;
// cactus
Mesh const* cactus_1_mesh = nullptr;
Mesh const* cactus_2_mesh = nullptr;
Mesh const* cactus_3_mesh = nullptr;
// fireflower
Mesh const* fireflower_1_mesh = nullptr;
Mesh const* fireflower_2_mesh = nullptr;
Mesh const* fireflower_3_mesh = nullptr;

Load< MeshBuffer > plant_meshes( LoadTagDefault, [](){
	auto ret = new MeshBuffer( data_path( "solidarity.pnct" ) );
	std::cout << "----meshes loaded:" << std::endl;
	for( auto p : ret->meshes ) {
		std::cout << p.first << std::endl;
	}
	sea_tile_mesh = &ret->lookup( "sea" );
	ground_tile_mesh = &ret->lookup( "soil" );
	test_plant_1_mesh = &ret->lookup( "tree_trunk" ); //TEMP
	test_plant_2_mesh = &ret->lookup( "tree_1" ); //TEMP
	friend_plant_1_mesh = &ret->lookup( "leaf1" ); //TEMP
	friend_plant_2_mesh = &ret->lookup( "leaf2" ); //TEMP
	friend_plant_3_mesh = &ret->lookup( "leaf3" ); //TEMP
	vampire_plant_1_mesh = &ret->lookup( "carrot1" ); //TEMP
	vampire_plant_2_mesh = &ret->lookup( "carrot2" ); //TEMP
	vampire_plant_3_mesh = &ret->lookup( "carrot3" ); //TEMP
	obstacle_tile_mesh = &ret->lookup( "unoccupied" );
	cactus_1_mesh = &ret->lookup( "cactus1" );
	cactus_2_mesh = &ret->lookup( "cactus2" );
	cactus_3_mesh = &ret->lookup( "cactus3" );
	fireflower_1_mesh = &ret->lookup( "fireflower1" );
	fireflower_2_mesh = &ret->lookup( "fireflower2" );
	fireflower_3_mesh = &ret->lookup( "fireflower3" );

	sea_tile = new GroundTileType( false, sea_tile_mesh );
	ground_tile = new GroundTileType( true, ground_tile_mesh );
	obstacle_tile = new GroundTileType( false, obstacle_tile_mesh );
	test_plant = new PlantType( { test_plant_1_mesh, test_plant_2_mesh }, Aura::none, 5, 10, 5.0f, "Fern", "Cheap plant. Grows anywhere." );
	friend_plant = new PlantType( { friend_plant_1_mesh, friend_plant_2_mesh, friend_plant_3_mesh }, Aura::none, 10, 25, 15.0f, "Friend Fern", "Speeds up growth of neighbors. Needs a neighbor to grow." );
	vampire_plant = new PlantType( { vampire_plant_1_mesh, vampire_plant_2_mesh, vampire_plant_3_mesh }, Aura::none, 20, 60, 20.0f, "Sapsucker", "Grows by stealing nutrients from other plants" );
	cactus_plant = new PlantType( { cactus_1_mesh, cactus_2_mesh, cactus_3_mesh }, Aura::none, 10, 20, 20.0f, "Cactus", "Grows faster under fire aura's influence but dislikes aqua aura." );
	fireflower_plant = new PlantType( { fireflower_1_mesh, fireflower_2_mesh, fireflower_3_mesh }, Aura::fire, 5, 0, 10.0f, "Fire flower", "Gives off fire aura." );

	plant_mesh_buffer = ret;

	return ret;
} );

Load< GLuint > plant_meshes_for_firstpass_program( LoadTagDefault, [](){
	return new GLuint( plant_meshes->make_vao_for_program( firstpass_program->program ) );
} );

TileGrid setup_grid_for_scene( Scene& scene, int plant_grid_x, int plant_grid_y )
{
	
	if(!sea_tile )
	{
		printf( "Wtfff" );
	}

	TileGrid grid;
	// Make the tile grid
	{
		grid.tiles = new GroundTile * [plant_grid_x];
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			grid.tiles[x] = new GroundTile[plant_grid_y];
		}
	}

	grid.size_x = plant_grid_x;
	grid.size_y = plant_grid_y;

	//Populate the tile grid (default is sea)
	{
		Scene::Drawable::Pipeline default_info;
		default_info = firstpass_program_pipeline;
		default_info.vao = *plant_meshes_for_firstpass_program;
		default_info.start = 0;
		default_info.count = 0;

		glm::vec3 tile_center_pos = glm::vec3( ( (float)plant_grid_x - 1 ) * plant_grid_tile_size.x / 2.0f, ( (float)plant_grid_y - 1 ) * plant_grid_tile_size.y / 2.0f, 0.0f );

		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				// Set coordinates
				grid.tiles[x][y].grid_x = x;
				grid.tiles[x][y].grid_y = y;

				// Set up tile drawable and initial pipline for each tile
				scene.transforms.emplace_back();
				Scene::Transform* tile_transform = &scene.transforms.back();
				tile_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( tile_transform );
				Scene::Drawable* tile = &scene.drawables.back();
				tile->pipeline = default_info;
				grid.tiles[x][y].tile_drawable = tile;

				// Set up plant drawable and initial pipline for each plant (empty)
				scene.transforms.emplace_back();
				Scene::Transform* plant_transform = &scene.transforms.back();
				plant_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( plant_transform );
				Scene::Drawable* plant = &scene.drawables.back();
				plant->pipeline = default_info;
				grid.tiles[x][y].plant_drawable = plant;

				// Set default type for the tile
				grid.tiles[x][y].change_tile_type( sea_tile );

			}
		}
	}

	return grid;
}

void GroundTile::change_tile_type( const GroundTileType* tile_type_in )
{
	if( tile_type_in )
	{
		tile_type = tile_type_in;
		tile_drawable->pipeline.start = tile_type->get_mesh()->start;
		tile_drawable->pipeline.count = tile_type->get_mesh()->count;
	}
	else
	{
		printf( "ERROR Passed in null tile type! \n" );
	}
}

void GroundTile::update( float elapsed, Scene::Transform* camera_transform, const TileGrid& grid )
{
	// update plant state
	if( plant_type )
	{
		if( plant_type == test_plant )
		{
			current_grow_time += elapsed;
		}
		else if( plant_type == friend_plant )
		{
			bool has_neighbor = false;
			for( int x = -1; x <= 1; x += 2 )
			{
				if( grid_x + x >= 0 && grid_x + x < grid.size_x )
				{
					GroundTile& tile = grid.tiles[grid_x + x][grid_y];
					const PlantType* plant = tile.plant_type;
					if( plant )
					{
						has_neighbor = true;
						//Boost the neighbor
						tile.current_grow_time += elapsed * 0.1f;
					}
				}
			}
			for( int y = -1; y <= 1; y += 2 )
			{
				if( grid_y + y >= 0 && grid_y + y < grid.size_y )
				{
					GroundTile& tile = grid.tiles[grid_x][grid_y + y];
					const PlantType* plant = tile.plant_type;
					if( plant )
					{
						has_neighbor = true;
						//Boost the neighbor
						tile.current_grow_time += elapsed * 0.1f;
					}
				}
			}

			if( has_neighbor )
			{
				current_grow_time += elapsed;
			}
			else
			{
				current_grow_time -= elapsed;
			}
		}
		else if( plant_type == vampire_plant )
		{
			std::vector<GroundTile*> victims;

			for( int x = -1; x <= 1; x += 1 )
			{
				for( int y = -1; y <= 1; y += 1 )
				{
					if( grid_x + x >= 0 && grid_x + x < grid.size_x && grid_y + y >= 0 && grid_y + y < grid.size_y && ( x != 0 || y != 0 ) )
					{
						GroundTile& tile = grid.tiles[grid_x + x][grid_y + y];
						const PlantType* plant = tile.plant_type;
						if( plant )
						{
							victims.push_back( &tile );
						}
					}
				}
			}

			if( victims.size() > 0 )
			{

				victims[rand() % victims.size()]->current_grow_time -= elapsed * 3.0f;
				current_grow_time += elapsed;
			}
			else
			{
				current_grow_time -= elapsed;
			}
		}
		else if( plant_type == fireflower_plant )
		{
			current_grow_time += elapsed;
		}
		else if( plant_type == cactus_plant )
		{
			current_grow_time += elapsed;
			current_grow_time += elapsed * fire_aura_effect;
			current_grow_time -= elapsed * aqua_aura_effect;
		}

		float target_time = plant_type->get_growth_time();
		if( current_grow_time < -1.0f ) try_remove_plant();
		if( current_grow_time > target_time ) current_grow_time = target_time;
		update_plant_visuals( current_grow_time / target_time );
	}

	// apply aura effect onto neighbors (by putting into pending update)
	if( plant_type && ( plant_type->get_aura_type() != Aura::none ) )
	{
		auto try_apply_aura = [elapsed]( GroundTile& target, Aura::Type aura_type ) {
			if( target.tile_type->get_can_plant() ) {
				switch( aura_type ) {
				case Aura::fire:
					target.pending_update.fire_aura_effect += 0.2f * elapsed;
					break;
				case Aura::aqua:
					target.pending_update.aqua_aura_effect += 0.2f * elapsed;
					break;
				default:
					std::cerr << "non-exhaustive matching of aura type??";
					break;
				}
			}
		};
		// get a list of neighbors
		std::vector< GroundTile* > neighbors = {};
		for( int x = -1; x <= 1; x += 2 ) {
			if( grid_x + x >= 0 && grid_x + x < grid.size_x ) {
				GroundTile& tile = grid.tiles[grid_x + x][grid_y];
				neighbors.push_back( &tile );
			}
		}
		for( int y = -1; y <= 1; y += 2 ) {
			if( grid_y + y >= 0 && grid_y + y < grid.size_y ) {
				GroundTile& tile = grid.tiles[grid_x][grid_y + y];
				neighbors.push_back( &tile );
			}
		}
		// apply effect
		for( auto tile_ptr : neighbors ) {
			assert( tile_ptr );
			try_apply_aura( *tile_ptr, plant_type->get_aura_type() );
		}
	}

	// received aura effects decrease over time
	fire_aura_effect = std::max( 0.0f, fire_aura_effect - 0.1f * elapsed );
	aqua_aura_effect = std::max( 0.0f, aqua_aura_effect - 0.1f * elapsed );
}

void GroundTile::update_plant_visuals( float percent_grown )
{
	if( plant_type )
	{
		plant_drawable->transform->scale = glm::mix( glm::vec3( 0.75f, 0.75f, 0.35f ), glm::vec3( 1.0f, 1.0f, 1.0f ), plant_type->get_stage_percent( percent_grown ) );
		const Mesh* plant_mesh = plant_type->get_mesh( percent_grown );
		plant_drawable->pipeline.start = plant_mesh->start;
		plant_drawable->pipeline.count = plant_mesh->count;
	}
}

void GroundTile::apply_pending_update()
{
	// move update from pending_update
	fire_aura_effect = std::min( 1.0f, fire_aura_effect + pending_update.fire_aura_effect );
	aqua_aura_effect = std::min( 1.0f, aqua_aura_effect + pending_update.aqua_aura_effect );
	pending_update.fire_aura_effect = 0.0f;
	pending_update.aqua_aura_effect = 0.0f;
}

void GroundTile::update_aura_visuals( float elapsed, Scene::Transform* camera_transform )
{ // TODO: always have aura created but only update when there is effect?
	// create corresponding aura if not already exist
	if( fire_aura_effect > 0 && (!fire_aura) ) {
		fire_aura = new Aura( tile_drawable->transform->position, Aura::fire );
	}
	if( aqua_aura_effect > 0 && (!aqua_aura) ) {
		aqua_aura = new Aura( tile_drawable->transform->position, Aura::fire );
	}
	// or delete if no longer has aura effect	
	if( fire_aura_effect == 0 && fire_aura ) {
		delete fire_aura;
		fire_aura = nullptr;
	}
	if( aqua_aura_effect == 0 && aqua_aura ) {
		delete aqua_aura;
		aqua_aura = nullptr;
	}
	// update aura accordingly
	if( fire_aura ) fire_aura->update( 
			int(floor(fire_aura_effect * fire_aura->max_strength)), // strength
			elapsed, camera_transform );
	if( aqua_aura ) aqua_aura->update( 
			int(floor(aqua_aura_effect * aqua_aura->max_strength)), // strength
			elapsed, camera_transform );
}

bool GroundTile::try_add_plant( const PlantType* plant_type_in )
{
	// If we can plant on the tile and there is no plant already there, add a plant
	if( tile_type->get_can_plant() && !plant_type )
	{
		plant_type = plant_type_in;
		plant_drawable->pipeline.start = plant_type->get_mesh( 0.0f )->start;
		plant_drawable->pipeline.count = plant_type->get_mesh( 0.0f )->count;

		current_grow_time = 0.0f;
		update_plant_visuals( 0.0f );
		return true;
	}
	return false;
}

bool GroundTile::try_remove_plant()
{
	// If there is a plant on tile, kick it out and hide the drawable
	if( plant_type )
	{
		plant_type = nullptr;
		plant_drawable->pipeline.start = 0;
		plant_drawable->pipeline.count = 0;
		return true;
	}
	return false;
}

bool GroundTile::is_tile_harvestable()
{
	return plant_type && current_grow_time >= plant_type->get_growth_time();
}

