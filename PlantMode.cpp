#include "PlantMode.hpp"

#include "FirstpassProgram.hpp"
#include "PostprocessingProgram.hpp"
#include "Load.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "load_save_png.hpp"
#include "collide.hpp"
#include "DrawSprites.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstddef>
#include <random>
#include <unordered_map>

PlantType const* test_plant = nullptr;
PlantType const* fireflower = nullptr;
GroundTileType const* sea_tile = nullptr;
GroundTileType const* ground_tile = nullptr;
GroundTileType const* obstacle_tile = nullptr;

// ground tiles
Mesh const* sea_tile_mesh = nullptr;
Mesh const* ground_tile_mesh = nullptr;
Mesh const* obstacle_tile_mesh = nullptr;
// test plant (looks like fern)?
Mesh const* test_plant_1_mesh = nullptr;
Mesh const* test_plant_2_mesh = nullptr;
Mesh const* test_plant_3_mesh = nullptr;
// carrot
Mesh const* carrot_1_mesh = nullptr;
Mesh const* carrot_2_mesh = nullptr;
Mesh const* carrot_3_mesh = nullptr;
// cactus
Mesh const* cactus_1_mesh = nullptr;
Mesh const* cactus_2_mesh = nullptr;
Mesh const* cactus_3_mesh = nullptr;
// fireflower
Mesh const* fireflower_1_mesh = nullptr;
Mesh const* fireflower_2_mesh = nullptr;
Mesh const* fireflower_3_mesh = nullptr;

Load< SpriteAtlas > font_atlas( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

Load< MeshBuffer > plant_meshes(LoadTagDefault, [](){
	auto ret = new MeshBuffer(data_path("solidarity.pnct"));
	std::cout << "----meshes loaded:" << std::endl;
	for (auto p : ret->meshes) {
		std::cout << p.first << std::endl;
	}
	sea_tile_mesh = &ret->lookup("sea");
	ground_tile_mesh = &ret->lookup("soil");
	obstacle_tile_mesh = &ret->lookup("unoccupied");
	test_plant_1_mesh = &ret->lookup("leaf1");
	test_plant_2_mesh = &ret->lookup("leaf2");
	test_plant_3_mesh = &ret->lookup("leaf3");
	carrot_1_mesh = &ret->lookup("carrot1");
	carrot_2_mesh = &ret->lookup("carrot2");
	carrot_3_mesh = &ret->lookup("carrot3");
	cactus_1_mesh = &ret->lookup("cactus1");
	cactus_2_mesh = &ret->lookup("cactus2");
	cactus_3_mesh = &ret->lookup("cactus3");
	fireflower_1_mesh = &ret->lookup("fireflower1");
	fireflower_2_mesh = &ret->lookup("fireflower2");
	fireflower_3_mesh = &ret->lookup("fireflower3");
	return ret;
});

Load< GLuint > plant_meshes_for_firstpass_program(LoadTagDefault, [](){
	return new GLuint(plant_meshes->make_vao_for_program(firstpass_program->program));
});

void GroundTile::change_tile_type( const GroundTileType* tile_type_in )
{
	tile_type = tile_type_in;
	tile_drawable->pipeline.start = tile_type->get_mesh()->start;
	tile_drawable->pipeline.count = tile_type->get_mesh()->count;
}

void GroundTile::update( float elapsed, Scene::Transform* camera_transform )
{
	// update plant state
	if( plant_type )
	{
		float target_time = plant_type->get_growth_time();
		current_grow_time += elapsed;
		if( current_grow_time > target_time ) current_grow_time = target_time;
		update_plant_visuals( current_grow_time / target_time );
		// non-harvestable plants are automatically removed (?)
		if( current_grow_time >= target_time && !plant_type->get_harvestable() ) try_remove_plant(); 
	}

	// update aura state after plant update is done
	if( plant_type && (plant_type->get_aura_type() != Aura::none) ) { 
		if( aura && (plant_type->get_aura_type() == aura->type) )
		{ // the tile already has the plant's aura
			aura->update( elapsed, camera_transform );
		}
		else
		{ // the tile doesn't has the plant's aura: either it has something else, or doesn't have any
			try_remove_aura();
			aura = new Aura( tile_drawable->transform->position, plant_type->get_aura_type() );
		}
	} else try_remove_aura();
}

void GroundTile::update_plant_visuals( float percent_grown )
{
	//TEMP!!!!!!
	plant_drawable->transform->position.z = glm::mix( start_height, end_height, percent_grown );
}

void GroundTile::apply_pending_update()
{
	fire_aura_effect = std::min( 1.0f, fire_aura_effect + pending_update.fire_aura_effect );
	aqua_aura_effect = std::min( 1.0f, aqua_aura_effect + pending_update.aqua_aura_effect );
	pending_update.fire_aura_effect = 0.0f;
	pending_update.aqua_aura_effect = 0.0f;
}

bool GroundTile::try_add_plant( const PlantType* plant_type_in )
{
	// If we can plant on the tile and there is no plant already there, add a plant
	if( tile_type->get_can_plant() && !plant_type )
	{
		plant_type = plant_type_in;
		plant_drawable->pipeline.start = plant_type->get_mesh()->start;
		plant_drawable->pipeline.count = plant_type->get_mesh()->count;

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

bool GroundTile::try_remove_aura() 
{
	// set the aura pointer to null. 
	// However if the tile has a plant that gives aura, will add one in next iteration anyway
	if( aura ) 
	{
		delete aura;
		aura = nullptr;
		return true;
	}
	return false;
}

bool GroundTile::is_tile_harvestable()
{
	return plant_type 
		&& plant_type->get_harvestable()
		&& current_grow_time >= plant_type->get_growth_time();
}

PlantMode::PlantMode() 
{
	{
		sea_tile = new GroundTileType( false, sea_tile_mesh );
		ground_tile = new GroundTileType( true, ground_tile_mesh );
		obstacle_tile = new GroundTileType( false, obstacle_tile_mesh );

		test_plant = new PlantType( test_plant_2_mesh, Aura::none, 5, true, 7, 5.0f, "Fern", "Cheap plant. Grows anywhere." );
		fireflower = new PlantType ( fireflower_3_mesh, Aura::fire, 10, false, 0, 10.0f, "Fire flower", "Gives off fire aura." );

		selectedPlant = fireflower;
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
			for( int32_t y = 7; y < 13; ++y )
			{
				grid[x][y].change_tile_type( obstacle_tile );
			}
		}

		for( int32_t x = 8; x < 12; ++x )
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

	{ // init the opengl stuff
		// ------ generate framebuffer for firstpass
		glGenFramebuffers(1, &firstpass_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, firstpass_fbo);
		// and its two color output layers
		glGenTextures(2, firstpass_color_attachments);
		for (GLuint i=0; i<2; i++) {
			glBindTexture(GL_TEXTURE_2D, firstpass_color_attachments[i]);
			glTexImage2D(
				// ended up disabling high resolution draw so the program runs at a reasonable framerate...
				GL_TEXTURE_2D, 0, GL_RGBA, 
				(GLint)(screen_size.x/postprocessing_program->pixel_size), 
				(GLint)(screen_size.y/postprocessing_program->pixel_size), 
				0, GL_RGBA, GL_FLOAT, NULL    
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, firstpass_color_attachments[i], 0    
			);
		}
		// setup associated depth buffer
		glGenRenderbuffers(1, &firstpass_depth_attachment);
		glBindRenderbuffer(GL_RENDERBUFFER, firstpass_depth_attachment);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)screen_size.x, (GLsizei)screen_size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, firstpass_depth_attachment);

		glDrawBuffers(2, color_attachments);
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ------ set up fbo for aura
		glGenFramebuffers(1, &aura_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, aura_fbo);
		// and its color output
		glGenTextures(1, &aura_color_attachment);
		glBindTexture(GL_TEXTURE_2D, aura_color_attachment);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, 
			(GLint)(screen_size.x/postprocessing_program->pixel_size), 
			(GLint)(screen_size.y/postprocessing_program->pixel_size), 
			0, GL_RGBA, GL_FLOAT, NULL    
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aura_color_attachment, 0    
		);
		// make it share depth buffer with firstpass
		glBindRenderbuffer(GL_RENDERBUFFER, firstpass_depth_attachment);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, firstpass_depth_attachment);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		// check status, unbind things
		assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// ------ set up 2nd pass pipeline
		glGenVertexArrays(1, &trivial_vao);
		glBindVertexArray(trivial_vao);

		glGenBuffers(1, &trivial_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, trivial_vbo);
		glBufferData(
			GL_ARRAY_BUFFER, 
			trivial_vector.size() * sizeof(float),
			trivial_vector.data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		GL_ERRORS();

		// ------ ping pong framebuffers for gaussian blur (use this for aura effect later)
		glGenFramebuffers(2, pingpong_fbos);
		glGenTextures(2, pingpong_color_attachments);
		for (unsigned int i = 0; i < 2; i++) {
			glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbos[i]);
			glBindTexture(GL_TEXTURE_2D, pingpong_color_attachments[i]);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA, 
				(GLint)(screen_size.x/postprocessing_program->pixel_size), 
				(GLint)(screen_size.y/postprocessing_program->pixel_size), 
				0, GL_RGBA, GL_FLOAT, NULL
			); // w&h of drawable size
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_color_attachments[i], 0
			);
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}

PlantMode::~PlantMode() {
	for (int i=0; i<plant_grid_x; i++) {
		for (int j=0; j<plant_grid_y; j++) {
			grid[i][j].try_remove_aura();
		}
	}
}

void PlantMode::on_click( int x, int y )
{
	GroundTile* collided_tile = get_tile_under_mouse( x, y );

	if( collided_tile )
	{
		if( collided_tile->plant_type )
		{
			if( collided_tile->is_tile_harvestable() )
			{
				int gain = collided_tile->plant_type->get_harvest_gain();
				if( collided_tile->try_remove_plant() )
				{
					energy += gain;
				}
			}
		}
		else
		{
			if( collided_tile->tile_type == obstacle_tile )
			{
				collided_tile->change_tile_type(ground_tile);
			}
			else if(selectedPlant && energy >= selectedPlant->get_cost())
			{
				if( collided_tile->try_add_plant( selectedPlant ) )
				{
					energy -= selectedPlant->get_cost();
				}
			}
		}
	}
}

GroundTile* PlantMode::get_tile_under_mouse( int x, int y )
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
	glm::vec3 ray_wor = glm::vec3( ray_wort.x, ray_wort.y, ray_wort.z );
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
			(void)sphere_sweep_min;
			(void)sphere_sweep_max;

			float collision_t = 1.0f;
			glm::vec3 collision_at = glm::vec3( 0.0f );
			glm::vec3 collision_out = glm::vec3( 0.0f );

			glm::mat4x3 collider_to_world = grid[x][y].tile_drawable->transform->make_local_to_world();
			const Mesh& collider_mesh = *( grid[x][y].tile_type->get_mesh() );

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
	
	return collided_tile;
}

bool PlantMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//ignore any keys that are the result of automatic key repeat:
	if (evt.type == SDL_KEYDOWN && evt.key.repeat) {
		return false;
	}

	if( evt.type == SDL_MOUSEBUTTONDOWN )
	{
		if( evt.button.button == SDL_BUTTON_LEFT )
		{
			on_click( evt.motion.x, evt.motion.y );
		}
		return false;
	}

	return false;
}

void PlantMode::update(float elapsed) 
{
	// camera_azimuth += 0.5f * elapsed;

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

	// update tiles
	{
		// initial update for grids themselves
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid[x][y].update( elapsed, camera->transform );
			}
		}
		// apply aura's effect on nearby tiles (if there is aura)
		float full_amount = 0.2f * elapsed;
		float half_amount = 0.1f * elapsed;
		auto try_apply_aura = [](GroundTile& target, Aura::Type aura_type, float amount) {
			if( target.tile_type->get_can_plant() ) {
				switch (aura_type) {
					case Aura::fire:
						target.pending_update.fire_aura_effect = amount;
						break;
					case Aura::aqua:
						target.pending_update.aqua_aura_effect = amount;
						break;
					default:
						std::cerr << "non-exhaustive matching of aura type??";
						break;
				}
			}
		};
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				if( grid[x][y].aura )
				{ // apply full effect on horizontal/vertical neighbors, half effect on diagonal neighbors
					// TODO: should this effect be cumulative (stronger over time), or constant?
					Aura::Type aura_type = grid[x][y].aura->type;
					// diagonals
					if( x-1>=0 && y-1>=0 ) try_apply_aura( grid[x-1][y-1], aura_type, half_amount );
					if( x-1>=0 && y+1<plant_grid_y ) try_apply_aura( grid[x-1][y+1], aura_type, half_amount );
					if( x+1<plant_grid_x && y-1>=0 ) try_apply_aura( grid[x+1][y-1], aura_type, half_amount );
					if( x+1<plant_grid_x && y+1<plant_grid_y ) try_apply_aura( grid[x+1][y+1], aura_type, half_amount );
					// horizontal/vertical
					if( x-1>=0 ) try_apply_aura( grid[x-1][y], aura_type, full_amount );
					if( x+1<plant_grid_x ) try_apply_aura( grid[x+1][y], aura_type, full_amount );
					if( y-1>=0 ) try_apply_aura( grid[x][y-1], aura_type, full_amount );
					if( y+1<plant_grid_y ) try_apply_aura( grid[x][y+1], aura_type, full_amount );
				}
			}
		}
		// apply pending update
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid[x][y].apply_pending_update();
			}
		}
	}

	// Query for hovered tile
	int x, y;
	const Uint32 state = SDL_GetMouseState( &x, &y );
	(void)state;
	if( true )
	{
		GroundTile* hovered_tile = get_tile_under_mouse( x, y );
		if( hovered_tile && hovered_tile->plant_type)
		{
			if( hovered_tile->is_tile_harvestable() )
			{
				action_description = "Harvest +" + std::to_string( hovered_tile->plant_type->get_harvest_gain());
			}
			else
			{
				action_description = "Growing "; //+ std::to_string(hovered_tile->current_grow_time / hovered_tile->plant_type->get_growth_time());
			}
		}
		else if ( hovered_tile && selectedPlant && hovered_tile->tile_type->get_can_plant() )
		{
			action_description = "Plant -" + std::to_string(selectedPlant->get_cost());
		}
		else
		{
			action_description = "";
		}

		if( hovered_tile && hovered_tile->tile_type->get_can_plant() ) 
		{
			auto f2s = [](float f) { // from: https://stackoverflow.com/questions/16605967/set-precision-of-stdto-string-when-converting-floating-point-values
				std::ostringstream out;
				out.precision(2);
				out << std::fixed << f;
				return out.str();
			};
			std::string fire = f2s( hovered_tile->fire_aura_effect );
			std::string aqua = f2s( hovered_tile->aqua_aura_effect );
			tile_status_summary = "Fire: " + fire + ", Aqua: " + aqua;
		}
		else
		{
			tile_status_summary = "";
		}
	}
}

void PlantMode::draw(glm::uvec2 const &drawable_size) {
	//Draw scene:
	camera->aspect = drawable_size.x / float(drawable_size.y);

	//---- first pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, firstpass_fbo);
	glViewport(0, 0, 
		(GLsizei)(screen_size.x / postprocessing_program->pixel_size),
		(GLsizei)(screen_size.y / postprocessing_program->pixel_size));
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//-- set up basic OpenGL state --
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_BLEND);
	// draw the scene
	scene.draw(*camera);
	// draw aura
	glBindFramebuffer(GL_FRAMEBUFFER, aura_fbo);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glm::mat4 world_to_clip = camera->make_projection() * camera->transform->make_world_to_local();
	for (int i=0; i<plant_grid_x; i++) {
		for (int j=0; j<plant_grid_y; j++) {
			if (grid[i][j].aura) grid[i][j].aura->draw(world_to_clip);
		}
	}

	//---- postprocessing pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(postprocessing_program->program);
	glBindVertexArray(trivial_vao);
	glBindBuffer(GL_ARRAY_BUFFER, trivial_vbo);

	// gaussian blur
	glActiveTexture(GL_TEXTURE0);
	bool horizontal = true, first_iteration = true;
	int amount = 2; // must be even
	for (int i=0; i<amount; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, pingpong_fbos[!horizontal]);
		// set horizontal uniform (task)
		glUniform1i(postprocessing_program->TASK_int, (int)(!horizontal));
		// bind input texture
		glUniform1i(postprocessing_program->TEX0_tex, 0);
		glBindTexture(GL_TEXTURE_2D, 
			first_iteration ? aura_color_attachment : pingpong_color_attachments[horizontal]
		);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		horizontal = !horizontal;
		if (first_iteration) first_iteration = false;
	}

	//-- combine all results
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, (GLsizei)screen_size.x, (GLsizei)screen_size.y);
	// set uniform so the shader performs desired task
	glUniform1i(postprocessing_program->TASK_int, 3);
	// set uniform for texture offset
	glUniform2f(postprocessing_program->TEX_OFFSET_vec2, 
		postprocessing_program->pixel_size / screen_size.x, 
		postprocessing_program->pixel_size / screen_size.y);
	// bind inputs
	glUniform1i(postprocessing_program->TEX0_tex, 0);
	glUniform1i(postprocessing_program->TEX1_tex, 1);
	glUniform1i(postprocessing_program->TEX2_tex, 2);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, firstpass_color_attachments[0]);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, firstpass_color_attachments[1]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, pingpong_color_attachments[1]);

	// draw
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// unbind things
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glUseProgram(0);
	GL_ERRORS();

	// TEXT
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glDisable( GL_DEPTH_TEST );

	{ //draw all the text
		DrawSprites draw( *font_atlas, glm::vec2( -1.0f, -1.0f ), glm::vec2( 1.0f, 1.0f ), drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text( selectedPlant->get_name() + " (" + std::to_string(selectedPlant->get_cost()) +") : " + selectedPlant->get_description(), glm::vec2( -1.5f, 0.85f ), 0.006f);
		draw.draw_text( "Energy: " + std::to_string( energy ), glm::vec2( 0.7f, 0.85f ), 0.006f );
		draw.draw_text( action_description, glm::vec2( 0.7f, 0.75f ), 0.006f );
		draw.draw_text( tile_status_summary, glm::vec2( 0.7f, 0.65f), 0.006f );
	}
}


