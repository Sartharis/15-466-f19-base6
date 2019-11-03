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
#include "MenuMode.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <cstddef>
#include <random>
#include <unordered_map>

// Sprite const *kitchen_empty = nullptr;

TileGrid grid;
int plant_grid_x = 20;
int plant_grid_y = 20;

Mesh const* selector_mesh = nullptr;
Sprite const *magic_book_sprite = nullptr;
Sprite const* glove_sprite = nullptr;

Load< SpriteAtlas > font_atlas( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

Load< SpriteAtlas > magicbook_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *kret = new SpriteAtlas(data_path("solidarity"));
	std::cout << "----2D sprites loaded:" << std::endl;
	for( auto p : kret->sprites ) {
		std::cout << p.first << std::endl;
	}
	magic_book_sprite = &kret->lookup("magicbookBackground");
	glove_sprite = &kret->lookup("glove");
	return kret;
});

Load< MeshBuffer > ui_meshes( LoadTagDefault, [](){
	auto ret = new MeshBuffer( data_path( "solidarityui.pnct" ) );
	std::cout << "----meshes loaded:" << std::endl;
	for( auto p : ret->meshes ) {
		std::cout << p.first << std::endl;
	}
	selector_mesh = &ret->lookup( "Selector" );
	return ret;
} );

Load< GLuint > ui_meshes_for_firstpass_program( LoadTagDefault, [](){
	return new GLuint( ui_meshes->make_vao_for_program( firstpass_program->program ) );
} );

PlantMode::PlantMode() 
{

	grid = setup_grid_for_scene( scene, plant_grid_x, plant_grid_y );

	{
		selectedPlant = test_plant;
	}

	//DEBUG - ADD ALL SEEDS
	{
		inventory.change_seeds_num( test_plant, 5 );
		inventory.change_seeds_num( friend_plant, 5 );
		inventory.change_seeds_num( vampire_plant, 5 );
		inventory.change_seeds_num( cactus_plant, 5 );
		inventory.change_seeds_num( fireflower_plant, 5 );
	}

	{
		//Create a selector mesh
		scene.transforms.emplace_back();
		Scene::Transform* selector_transform = &scene.transforms.back();
		selector_transform->position = glm::vec3();
		scene.drawables.emplace_back( selector_transform );
		selector = &scene.drawables.back();

		Scene::Drawable::Pipeline selector_info;
		selector_info = firstpass_program_pipeline;
		selector_info.vao = *ui_meshes_for_firstpass_program;
		selector_info.start = selector_mesh->start;
		selector_info.count = selector_mesh->count;
		selector->pipeline = selector_info;
	}
	
	// Create a lil center island
	{
		for( int32_t x = 7; x < 13; ++x )
		{
			for( int32_t y = 7; y < 13; ++y )
			{
				grid.tiles[x][y].change_tile_type( obstacle_tile );
			}
		}

		for( int32_t x = 8; x < 12; ++x )
		{
			for( int32_t y = 8; y < 12; ++y )
			{
				grid.tiles[x][y].change_tile_type( ground_tile );
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

	{ //DEBUG: init UI

		// glove button
		buttons.emplace_back (
			glm::vec2(50, 500), // position
			glm::vec2(64, 64), // size
			glove_sprite, // sprite
			glm::vec2(32, 32), // sprite anchor
			"sample text", // text
			glm::vec2(0, 0), // text anchor
			[]() {
				std::cout << "clicked on glove." << std::endl;
			} );
	}


}

PlantMode::~PlantMode() {
	for (int i=0; i<grid.size_x; i++) {
		for (int j=0; j<grid.size_y; j++) {
			grid.tiles[i][j].try_remove_aura();
		}
	}
}

void PlantMode::on_click( int x, int y )
{
	//---- first detect click on UI. If UI handled the click, return.
	for( int i = 0; i < buttons.size(); i++ )
	{
		if( buttons[i].try_click( glm::vec2(x, y) ) ) return;
	}

	//---- Otherwise, detect click on tiles.
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
			else if(selectedPlant && inventory.get_seeds_num( selectedPlant ) > 0)
			{
				if( collided_tile->try_add_plant( selectedPlant ) )
				{
					inventory.change_seeds_num( selectedPlant, -1 );
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
	for( int32_t x = 0; x < grid.size_x; ++x )
	{
		for( int32_t y = 0; y < grid.size_y; ++y )
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

			glm::mat4x3 collider_to_world = grid.tiles[x][y].tile_drawable->transform->make_local_to_world();
			const Mesh& collider_mesh = *( grid.tiles[x][y].tile_type->get_mesh() );

			assert( collider_mesh.type == GL_TRIANGLES ); //only have code for TRIANGLES not other primitive types
			for( GLuint v = 0; v + 2 < collider_mesh.count; v += 3 )
			{

				//get vertex positions from associated positions buffer:
				glm::vec3 a = collider_to_world * glm::vec4( plant_mesh_buffer->positions[collider_mesh.start + v + 0], 1.0f );
				glm::vec3 b = collider_to_world * glm::vec4( plant_mesh_buffer->positions[collider_mesh.start + v + 1], 1.0f );
				glm::vec3 c = collider_to_world * glm::vec4( plant_mesh_buffer->positions[collider_mesh.start + v + 2], 1.0f );
				//check triangle:
				bool did_collide = collide_swept_sphere_vs_triangle(
					sphere_sweep_from, sphere_sweep_to, sphere_radius,
					a, b, c,
					&collision_t, &collision_at, &collision_out );

				if( did_collide )
				{
					collided_tile = &grid.tiles[x][y];
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

	if( evt.type == SDL_KEYDOWN )
	{
		switch( evt.key.keysym.sym ){
		case SDLK_1:
			selectedPlant = test_plant;
			break;
		case SDLK_2:
			selectedPlant = friend_plant;
			break;
		case SDLK_3:
			selectedPlant = vampire_plant;
			break;
		case SDLK_4:
			selectedPlant = fireflower_plant;
			break;
		case SDLK_SPACE:
			if(is_magicbook_open==false){
				is_magicbook_open = true;
			}else{
				is_magicbook_open = false;
			}
		case SDLK_5:
			selectedPlant = cactus_plant;
			break;
		default:
			break;
		}
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
	//camera_azimuth += 0.5f * elapsed;

	if( energy < 5 )
	{
		energy++;
	}

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
		for( int32_t x = 0; x < grid.size_x; ++x )
		{
			for( int32_t y = 0; y < grid.size_y; ++y )
			{
				grid.tiles[x][y].update( elapsed, camera->transform, grid );
			}
		}
		// apply pending update
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid.tiles[x][y].apply_pending_update();
			}
		}
	}

	// Query for hovered tile
	int x, y;
	const Uint32 state = SDL_GetMouseState( &x, &y );
	(void)state;
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
		action_description = "Plant ";
	}
	else
	{
		action_description = "";
	}

	//Selector positioning
	if( hovered_tile )
	{
		selector->transform->position = hovered_tile->tile_drawable->transform->position + glm::vec3( 0.0f, 0.0f, -0.01f );
	}
	else
	{
		selector->transform->position = glm::vec3( 0.0f, 0.0f, -1000.0f );
	}

	//Description for aura effect
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

void PlantMode::draw(glm::uvec2 const &drawable_size) {
	//Draw scene:
	camera->aspect = float( drawable_size.x) / float(drawable_size.y);

	//---- first pass ----
	glBindFramebuffer(GL_FRAMEBUFFER, firstpass_fbo);
	glViewport(0, 0, 
		(GLsizei)( drawable_size.x / postprocessing_program->pixel_size),
		(GLsizei)( drawable_size.y / postprocessing_program->pixel_size));
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
	for (int i=0; i<grid.size_x; i++) {
		for (int j=0; j<grid.size_y; j++) {
			if (grid.tiles[i][j].aura) grid.tiles[i][j].aura->draw(world_to_clip);
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
	glViewport(0, 0, (GLsizei)drawable_size.x, (GLsizei)drawable_size.y);
	// set uniform so the shader performs desired task
	glUniform1i(postprocessing_program->TASK_int, 3);
	// set uniform for texture offset
	glUniform2f(postprocessing_program->TEX_OFFSET_vec2, 
		postprocessing_program->pixel_size / drawable_size.x,
		postprocessing_program->pixel_size / drawable_size.y);
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

	//test draw order
	// current_order->draw(drawable_size);

	{ //draw all the text
		DrawSprites draw( *font_atlas, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text( selectedPlant->get_name() + " (" + std::to_string(inventory.get_seeds_num(selectedPlant)) +") :", glm::vec2( 20.0f, drawable_size.y - 40.0f ), 3.0f);
		draw.draw_text( selectedPlant->get_description(), glm::vec2( 20.0f, drawable_size.y - 75.0f ), 2.0f );
		draw.draw_text( "Energy: " + std::to_string( energy ), glm::vec2( drawable_size.x - 160.0f, drawable_size.y - 40.0f ), 2.0f );

		glm::mat4 world_to_clip = camera->make_projection() * camera->transform->make_world_to_local();
		glm::vec4 sel_clip = world_to_clip * selector->transform->make_local_to_world() * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
		if( sel_clip.w > 0.0f )
		{
			glm::vec3 sel_clip_xyz = glm::vec3( sel_clip );
			glm::vec3 sel_clip_pos = sel_clip_xyz / sel_clip.w;
			glm::vec2 sel_clip_pos_xy = glm::vec2( sel_clip_pos );
			glm::vec2 window_pos = glm::vec2(( ( sel_clip_pos_xy.x + 1.0f ) / 2.0f ) * drawable_size.x, ( ( sel_clip_pos_xy.y + 1.0f ) / 2.0f ) * drawable_size.y );
			glm::vec2 extent_min, extent_max;
			draw.get_text_extents( action_description, glm::vec2( 0.0f, 0.0f ), 2.0f, &extent_min, &extent_max );
			draw.draw_text( action_description, window_pos - (extent_max /2.0f) + glm::vec2(10.0f, 160.0f) + glm::vec2(0.0f,-150.0f) * sel_clip_pos.z, 2.0f / sel_clip_pos.z );
		}
		
		//draw.draw_text( tile_status_summary, glm::vec2( 0.7f, 0.65f), 0.006f );

		// draw hint text
		draw.draw_text("Press Space to open magic book", glm::vec2( drawable_size.x/2.0f, 50.0f ), 2.0f );
	}

	{ //draw the buttons
		DrawSprites draw_sprites( *magicbook_atlas, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		DrawSprites draw_text( *font_atlas, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		for (int i=0; i<buttons.size(); i++) {
			buttons[i].draw( draw_sprites, draw_text );
		}
	}
   

    if(is_magicbook_open && Mode::current.get() == this)
	{
		open_book();
	}

	//test order


}

void PlantMode::on_resize( glm::uvec2 const& new_drawable_size )
{
	{ // init the opengl stuff
		// ------ generate framebuffer for firstpass
		glGenFramebuffers( 1, &firstpass_fbo );
		glBindFramebuffer( GL_FRAMEBUFFER, firstpass_fbo );
		// and its two color output layers
		glGenTextures( 2, firstpass_color_attachments );
		for( GLuint i = 0; i < 2; i++ ) {
			glBindTexture( GL_TEXTURE_2D, firstpass_color_attachments[i] );
			glTexImage2D(
				// ended up disabling high resolution draw so the program runs at a reasonable framerate...
				GL_TEXTURE_2D, 0, GL_RGBA,
				(GLint)( new_drawable_size.x / postprocessing_program->pixel_size ),
				(GLint)( new_drawable_size.y / postprocessing_program->pixel_size ),
				0, GL_RGBA, GL_FLOAT, NULL
			);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, firstpass_color_attachments[i], 0
			);
		}
		// setup associated depth buffer
		glGenRenderbuffers( 1, &firstpass_depth_attachment );
		glBindRenderbuffer( GL_RENDERBUFFER, firstpass_depth_attachment );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (GLsizei)new_drawable_size.x, (GLsizei)new_drawable_size.y );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, firstpass_depth_attachment );

		glDrawBuffers( 2, color_attachments );
		assert( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		// ------ set up fbo for aura
		glGenFramebuffers( 1, &aura_fbo );
		glBindFramebuffer( GL_FRAMEBUFFER, aura_fbo );
		// and its color output
		glGenTextures( 1, &aura_color_attachment );
		glBindTexture( GL_TEXTURE_2D, aura_color_attachment );
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA,
			(GLint)( new_drawable_size.x / postprocessing_program->pixel_size ),
			(GLint)( new_drawable_size.y / postprocessing_program->pixel_size ),
			0, GL_RGBA, GL_FLOAT, NULL
		);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, aura_color_attachment, 0
		);
		// make it share depth buffer with firstpass
		glBindRenderbuffer( GL_RENDERBUFFER, firstpass_depth_attachment );
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, firstpass_depth_attachment );
		glDrawBuffer( GL_COLOR_ATTACHMENT0 );
		// check status, unbind things
		assert( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		// ------ set up 2nd pass pipeline
		glGenVertexArrays( 1, &trivial_vao );
		glBindVertexArray( trivial_vao );

		glGenBuffers( 1, &trivial_vbo );
		glBindBuffer( GL_ARRAY_BUFFER, trivial_vbo );
		glBufferData(
			GL_ARRAY_BUFFER,
			trivial_vector.size() * sizeof( float ),
			trivial_vector.data(),
			GL_STATIC_DRAW );

		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( float ), (void*)0 );
		glEnableVertexAttribArray( 0 );

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );
		GL_ERRORS();

		// ------ ping pong framebuffers for gaussian blur (use this for aura effect later)
		glGenFramebuffers( 2, pingpong_fbos );
		glGenTextures( 2, pingpong_color_attachments );
		for( unsigned int i = 0; i < 2; i++ ) {
			glBindFramebuffer( GL_FRAMEBUFFER, pingpong_fbos[i] );
			glBindTexture( GL_TEXTURE_2D, pingpong_color_attachments[i] );
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA,
				(GLint)( new_drawable_size.x / postprocessing_program->pixel_size ),
				(GLint)( new_drawable_size.y / postprocessing_program->pixel_size ),
				0, GL_RGBA, GL_FLOAT, NULL
			); // w&h of drawable size
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpong_color_attachments[i], 0
			);
		}
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
}

int Inventory::get_seeds_num(const PlantType* plant ) 
{
	std::unordered_map<PlantType const*, int>::iterator it = flower_to_seeds.find( plant );
	if( it != flower_to_seeds.end())
	{
		return it->second;
	}
	else
	{
		flower_to_seeds.insert( std::make_pair( plant, 0 ) );
		return 0;
	}
}

void Inventory::change_seeds_num(const PlantType* plant, int seed_change )
{
	std::unordered_map<PlantType const*, int>::iterator it = flower_to_seeds.find( plant );
	if( it != flower_to_seeds.end() )
	{
		it->second += seed_change;
	}
	else
	{
		flower_to_seeds.insert( std::make_pair( plant, seed_change ) );
	}
}

void PlantMode::open_book(){
		std::vector< MenuMode::Item > items;
		glm::vec2 at(-110.0f, view_max.y - 180.0f);
		auto add_text = [&items,&at](std::string text) {
			items.emplace_back(text, nullptr, 1.0f, glm::u8vec4(0x00, 0x00, 0x00, 0xff), nullptr, at);
			at.y -= 10.0f;
		};

		auto add_choice = [&items,&at](std::string text, std::function< void(MenuMode::Item const &) > const &fn) {
			items.emplace_back(text, nullptr, 0.8f, glm::u8vec4(0x00, 0x00, 0x00, 0x88), fn, at + glm::vec2(16.0f, 0.0f));
			items.back().selected_tint = glm::u8vec4(0x00, 0x00, 0x00, 0xff);
			at.y -= 15.0f;
		};

		auto add_buy_choice = [&items, &at, this]( const PlantType* plant ) {

			std::string text = plant->get_name() + " " + std::to_string( plant->get_cost() ) + " energy";
			std::function< void( MenuMode::Item const& ) > const& fn =
				[this, plant]( MenuMode::Item const& ){
				if( energy >= plant->get_cost() ){
					energy -= plant->get_cost();
					inventory.change_seeds_num( plant, 1 );
				}
			};

			items.emplace_back( text, nullptr, 0.8f, glm::u8vec4( 0x00, 0x00, 0x00, 0x88 ), fn, at + glm::vec2( 16.0f, 0.0f ) );
			items.back().selected_tint = glm::u8vec4( 0x00, 0x00, 0x00, 0xff );
			at.y -= 15.0f;
		};

		add_buy_choice( test_plant );
		add_buy_choice( friend_plant );
		add_buy_choice( cactus_plant );
		add_buy_choice( vampire_plant );
		add_buy_choice( fireflower_plant );

		add_choice("Close the book", [this](MenuMode::Item const &){
			is_magicbook_open = false;
			Mode::current = shared_from_this();
		});
		at.y = view_max.y - 160.0f; //gap before choices
		at.x += 10.0f;
		add_text("Welcome to magic book");
		std::shared_ptr< MenuMode > menu = std::make_shared< MenuMode >(items);
		menu->atlas = font_atlas; // for test 
		menu->view_min = view_min;
		menu->view_max = view_max;
		menu->background = shared_from_this();
		Mode::current = menu;
	}
