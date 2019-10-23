#include "PlantMode.hpp"

#include "LitColorTextureProgram.hpp"
#include "BoneLitColorTextureProgram.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "collide.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <map>
#include <cstddef>
#include <random>
#include <unordered_map>

PlantType const* test_plant = nullptr;
GroundTileType const* sea_tile = nullptr;
GroundTileType const* ground_tile = nullptr;

Mesh const* sea_tile_mesh = nullptr;
Mesh const* ground_tile_mesh = nullptr;
Mesh const* plant_mesh = nullptr;

Load< MeshBuffer > plant_meshes(LoadTagDefault, [](){
	auto ret = new MeshBuffer(data_path("solidarity.pnct"));
	sea_tile_mesh = &ret->lookup("SeaTile");
	ground_tile_mesh = &ret->lookup("GroundTile");
	plant_mesh = &ret->lookup( "Plant" );
	return ret;
});

Load< GLuint > plant_meshes_for_lit_color_texture_program(LoadTagDefault, [](){
	return new GLuint(plant_meshes->make_vao_for_program(lit_color_texture_program->program));
});

PlantType::PlantType( const Mesh* mesh_in ) : mesh(mesh_in)
{
}

const Mesh* PlantType::get_mesh() const
{
	return mesh;
}

GroundTileType::GroundTileType( bool can_plant_in, const Mesh* tile_mesh_in )
	: can_plant(can_plant_in), mesh(tile_mesh_in)
{
}

const Mesh* GroundTileType::get_mesh() const
{
	return mesh;
}

bool GroundTileType::get_can_plant() const
{
	return can_plant;
}

void GroundTile::change_tile_type( const GroundTileType* tile_type_in )
{
	tile_type = tile_type_in;
	tile_drawable->pipeline.start = tile_type->get_mesh()->start;
	tile_drawable->pipeline.count = tile_type->get_mesh()->count;
}

bool GroundTile::try_add_plant( const PlantType* plant_type_in )
{
	// If we can plant on the tile and there is no plant already there, add a plant
	if( tile_type->get_can_plant() && !plant_type )
	{
		plant_type = plant_type_in;
		plant_drawable->pipeline.start = plant_type->get_mesh()->start;
		plant_drawable->pipeline.count = plant_type->get_mesh()->count;
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


PlantMode::PlantMode() 
{
	{
		sea_tile = new GroundTileType( false, sea_tile_mesh );
		ground_tile = new GroundTileType( true, ground_tile_mesh );
		test_plant = new PlantType( plant_mesh );
	}

	// Make the tile grid
	{
		grid = new GroundTile*[plant_grid_x];
		for( int32_t x = 0; x < plant_grid_x; ++x ) 
		{
			grid[x] = new GroundTile[plant_grid_y];
		}
	}

	//Populate the tile grid (default is sea)
	{
		Scene::Drawable::Pipeline default_info;
		default_info = lit_color_texture_program_pipeline;
		default_info.vao = *plant_meshes_for_lit_color_texture_program;
		default_info.start = 0;
		default_info.count = 0;

		glm::vec3 tile_center_pos = glm::vec3( ( (float)plant_grid_x - 1 ) * plant_grid_tile_size.x / 2.0f, ( (float)plant_grid_y - 1 ) * plant_grid_tile_size.y / 2.0f, 0.0f );

		for( int32_t x = 0; x < plant_grid_x; ++x ) 
		{
			for( int32_t y = 0; y < plant_grid_y; ++y ) 
			{
				// Set coordinates
				grid[x][y].grid_x = x;
				grid[x][y].grid_y = y;

				// Set up tile drawable and initial pipline for each tile
				scene.transforms.emplace_back();
				Scene::Transform* tile_transform = &scene.transforms.back();
				tile_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( tile_transform );
				Scene::Drawable* tile = &scene.drawables.back();
				tile->pipeline = default_info;
				grid[x][y].tile_drawable = tile;

				// Set up plant drawable and initial pipline for each plant (empty)
				scene.transforms.emplace_back();
				Scene::Transform* plant_transform = &scene.transforms.back();
				plant_transform->position = glm::vec3( plant_grid_tile_size.x * x, plant_grid_tile_size.y * y, 0.0f ) - tile_center_pos;
				scene.drawables.emplace_back( plant_transform );
				Scene::Drawable* plant = &scene.drawables.back();
				plant->pipeline = default_info;
				grid[x][y].plant_drawable = plant;

				// Set default type for the tile
				grid[x][y].change_tile_type(sea_tile);

			}
		}
	}


	// Create a lil center island
	{
		for( int32_t x = 7; x < 13; ++x )
		{
			for( int32_t y = 8; y < 12; ++y )
			{
				grid[x][y].change_tile_type( ground_tile );
			}
		}
	}


	{ //make a camera:
		scene.transforms.emplace_back();
		Scene::Transform *transform = &scene.transforms.back();
		transform->position = glm::vec3(0.0f, 0.0f, 0.0f);
		transform->rotation = glm::quat_cast(glm::mat3(glm::lookAt(
			transform->position,
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		)));
		scene.cameras.emplace_back(transform);
		camera = &scene.cameras.back();
		camera->near = 0.01f;
		camera->fovy = glm::radians(45.0f);
	}

}

PlantMode::~PlantMode() {
}

bool PlantMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	return false;
}

void PlantMode::update(float elapsed) 
{
	//camera_azimuth += elapsed;

	// Update Camera Position
	{
		float ce = std::cos( camera_elevation );
		float se = std::sin( camera_elevation );
		float ca = std::cos( camera_azimuth );
		float sa = std::sin( camera_azimuth );
		camera->transform->position = camera_radius * glm::vec3( ce * ca, ce * sa, se );
		camera->transform->rotation =
			glm::quat_cast( glm::transpose( glm::mat3( glm::lookAt(
			camera->transform->position,
			glm::vec3( 0.0f, 0.0f, 0.0f ),
			glm::vec3( 0.0f, 0.0f, 1.0f )
			) ) ) );
	}

	// Query for clicked tile
	int x, y;
	const Uint32 state = SDL_GetMouseState( &x, &y );
	if( state & SDL_BUTTON( SDL_BUTTON_LEFT ) )
	{
		//Get ray from camera to mouse in world space
			

		GLint dim_viewport[4];
		glGetIntegerv( GL_VIEWPORT, dim_viewport );
		int width = dim_viewport[2];
		int height = dim_viewport[3];

		glm::vec3 ray_nds = glm::vec3( 2.0f * x / width - 1.0f, 1.0f - ( 2.0f * y ) / height, 1.0f );
		glm::vec4 ray_clip = glm::vec4( ray_nds.x, ray_nds.y, -1.0f, 1.0f );
		glm::vec4 ray_cam = glm::inverse( camera->make_projection() ) * ray_clip;
		ray_cam = glm::vec4( ray_cam.x, ray_cam.y, -1.0f, 0.0f );
		glm::vec4 ray_wort = glm::inverse( camera->transform->make_world_to_local() ) * ray_cam;
		glm::vec3 ray_wor = glm::vec3(ray_wort.x, ray_wort.y, ray_wort.z);
		ray_wor = glm::normalize( ray_wor );
		

		float col_check_dist = 1000.0f;
		glm::vec3 from_camera_start = camera->transform->position;
		glm::vec3 from_camera_dir = ray_wor;//camera->transform->rotation * glm::vec3( 0.0f, 0.0f, -1.0f );

		// Check collision against each tile
		GroundTile* collided_tile = nullptr;
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				// For now do a small sphere sweep against each triangle (TODO: optimize to line vs box collision if this is really bad)
				float sphere_radius = 0.0001f; 
				glm::vec3 sphere_sweep_from = from_camera_start;
				glm::vec3 sphere_sweep_to = from_camera_start + col_check_dist * from_camera_dir;

				glm::vec3 sphere_sweep_min = glm::min( sphere_sweep_from, sphere_sweep_to ) - glm::vec3( sphere_radius );
				glm::vec3 sphere_sweep_max = glm::max( sphere_sweep_from, sphere_sweep_to ) + glm::vec3( sphere_radius );

				
				float collision_t = 1.0f;
				glm::vec3 collision_at = glm::vec3( 0.0f );
				glm::vec3 collision_out = glm::vec3( 0.0f );

				glm::mat4x3 collider_to_world = grid[x][y].tile_drawable->transform->make_local_to_world();
				const Mesh& collider_mesh = *(grid[x][y].tile_type->get_mesh());

				assert( collider_mesh.type == GL_TRIANGLES ); //only have code for TRIANGLES not other primitive types
				for( GLuint v = 0; v + 2 < collider_mesh.count; v += 3 ) 
				{
					
					//get vertex positions from associated positions buffer:
					glm::vec3 a = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 0], 1.0f );
					glm::vec3 b = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 1], 1.0f );
					glm::vec3 c = collider_to_world * glm::vec4( plant_meshes->positions[collider_mesh.start + v + 2], 1.0f );
					//check triangle:
					bool did_collide = collide_swept_sphere_vs_triangle(
						sphere_sweep_from, sphere_sweep_to, sphere_radius,
						a, b, c,
						&collision_t, &collision_at, &collision_out );

					if( did_collide ) 
					{
						collided_tile = &grid[x][y];
					}
				}
			}
		}

		if( collided_tile )
		{
			collided_tile->try_add_plant( test_plant );
		}
	}
}

void PlantMode::draw(glm::uvec2 const &drawable_size) {
	//Draw scene:
	camera->aspect = drawable_size.x / float(drawable_size.y);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//set up basic OpenGL state:
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	scene.draw(*camera);

	GL_ERRORS();
}


