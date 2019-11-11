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

TileGrid grid;
int plant_grid_x = 10;
int plant_grid_y = 10;

Mesh const* selector_mesh = nullptr;
Sprite const* magic_book_sprite = nullptr;
Sprite const* magicbook_icon_sprite = nullptr;

struct {
	struct {
		Sprite const* background = nullptr;
		Sprite const* watering_can = nullptr;
		Sprite const* glove = nullptr;
		Sprite const* shovel = nullptr;
		Sprite const* fertilizer = nullptr;
	} tools;
	struct {
		Sprite const* regular = nullptr;
		Sprite const* hand = nullptr;
	} cursor;
	struct {
		Sprite const* icon = nullptr;
		Sprite const* background = nullptr;
		Sprite const* seeds_tab = nullptr;
		Sprite const* harvest_tab = nullptr;
	} storage;
} sprites;

// TODO: rename to sprite_atlas since this contains a lot of non-magicbook stuff
Load< SpriteAtlas > main_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("solidarity"));
	std::cout << "----2D sprites loaded (solidarity):" << std::endl;
	for( auto p : ret->sprites ) {
		std::cout << p.first << std::endl;
	}
	// tools
	sprites.tools.background = &ret->lookup("toolsBackground");
	sprites.tools.watering_can = &ret->lookup("wateringCan");
	sprites.tools.glove = &ret->lookup("glove");
	sprites.tools.shovel = &ret->lookup("shovel");
	sprites.tools.fertilizer = &ret->lookup("fertilizer");
	// cursor
	sprites.cursor.regular = &ret->lookup("cursorNormal");
	sprites.cursor.hand = &ret->lookup("cursorHand");
	// storage
	sprites.storage.icon = &ret->lookup("seedBagClosed"); //TODO
	sprites.storage.background = &ret->lookup("seedMenuBackground");
	sprites.storage.seeds_tab = &ret->lookup("seedBagOpen");
	sprites.storage.harvest_tab = &ret->lookup("harvestBasket");
	// TEMP
	magic_book_sprite = &ret->lookup("magicbookBackground");
	magicbook_icon_sprite = &ret->lookup("magicbookIcon");
	return ret;
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

	//DEBUG - ADD ALL SEEDS & init harvest to all 0
	{
		inventory.change_seeds_num( test_plant, 5 );
		inventory.change_seeds_num( friend_plant, 5 );
		inventory.change_seeds_num( vampire_plant, 5 );
		inventory.change_seeds_num( cactus_plant, 5 );
		inventory.change_seeds_num( fireflower_plant, 5 );
		inventory.change_seeds_num( corpseeater_plant, 5 );

		inventory.change_harvest_num( test_plant, 0 );
		inventory.change_harvest_num( friend_plant, 0 );
		inventory.change_harvest_num( vampire_plant, 0 );
		inventory.change_harvest_num( cactus_plant, 0 );
		inventory.change_harvest_num( fireflower_plant, 0 );
		inventory.change_harvest_num( corpseeater_plant, 0 );
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
	
	std::string island =
		"oodddddooo"
		"oddxXxxddo"
		"dxxXXXxxdo"
		"odxxXxxxdd"
		"dxxxCCxxxd"
		"dxxxCCxxxd"
		"dxxxxxxxdo"
		"odxxXXxdoo"
		"odxdxXXxdo"
		"oododdddoo";

	// Create a lil center island
	{
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				const GroundTileType* type = sea_tile;
				if( island[x + y * plant_grid_x] == 'x' )
				{
					type = grass_short_tile;
				}
				else if( island[x + y * plant_grid_x] == 'X' )
				{
					type = grass_tall_tile;
				}
				else if( island[x + y * plant_grid_x] == 'd' )
				{
					type = dirt_tile;
				}
				else if ( island[x + y * plant_grid_x] == 'C' )
				{
					type = ground_tile;
				}
				grid.tiles[x][y].change_tile_type( type );
			}
		}
	}
	
	{ //make a camera:
		scene.transforms.emplace_back();
		Scene::Transform *transform = &scene.transforms.back();

		float ce = std::cos( camera_elevation );
		float se = std::sin( camera_elevation );
		float ca = std::cos( camera_azimuth );
		float sa = std::sin( camera_azimuth );
		transform->position = camera_radius * glm::vec3( ce * ca, ce * sa, se );
		transform->rotation =
			glm::quat_cast( glm::transpose( glm::mat3( glm::lookAt(
			transform->position,
			glm::vec3( 0.0f, 0.0f, 0.0f ),
			glm::vec3( 0.0f, 0.0f, 1.0f )
			) ) ) );

		scene.cameras.emplace_back(transform);
		camera = &scene.cameras.back();
		camera->near = 0.01f;
		camera->fovy = glm::radians(45.0f);
	}

	forward_camera_dir = camera->transform->rotation * glm::vec3( 0.0f, 0.0f, -1.0f );
	forward_dir = glm::normalize( glm::vec3( forward_camera_dir.x, forward_camera_dir.y, 0.0f ) );

	side_camera_dir = camera->transform->rotation * glm::vec3( 1.0f, 0.0f, 0.0f );
	side_dir = glm::normalize( glm::vec3( side_camera_dir.x, side_camera_dir.y, 0.0f ) );

    // add all order into order vector
	all_orders.push_back(order1);
	all_orders.push_back(order2);
	all_orders.push_back(order3);
	all_orders.push_back(order4);

	{ //DEBUG: init UI

		Button* btn;

		// glove
		btn = new Button (
			screen_size, Button::bl, glm::vec2(60, -77), // position
			glm::vec2(64, 64), // size
			sprites.tools.glove, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.3f, // sprite scale
			Button::show_text, // hover behavior
			"glove", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( current_tool == glove ) current_tool = none;
				else current_tool = glove;
			} );
		UI.tools.push_back(btn);
		UI.all_buttons.push_back(btn);

		// watering can
		btn = new Button (
			screen_size, Button::bl, glm::vec2(142, -72), // position
			glm::vec2(64, 64), // size
			sprites.tools.watering_can, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.3f, // sprite scale
			Button::show_text, // hover behavior
			"watering can", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( current_tool == watering_can ) current_tool = none;
				else current_tool = watering_can;
			} );
		UI.tools.push_back(btn);
		UI.all_buttons.push_back(btn);

		// fertilizer
		btn = new Button (
			screen_size, Button::bl, glm::vec2(230, -82), // position
			glm::vec2(64, 64), // size
			sprites.tools.fertilizer, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.3f, // sprite scale
			Button::show_text, // hover behavior
			"fertilizer", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( current_tool == fertilizer ) current_tool = none;
				else current_tool = fertilizer;
			} );
		UI.tools.push_back(btn);
		UI.all_buttons.push_back(btn);

		// shovel
		btn = new Button (
			screen_size, Button::bl, glm::vec2(315, -80), // position
			glm::vec2(64, 64), // size
			sprites.tools.shovel, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.3f, // sprite scale
			Button::show_text, // hover behavior
			"shovel", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( current_tool == shovel ) current_tool = none;
				else current_tool = shovel;
			} );
		UI.tools.push_back(btn);
		UI.all_buttons.push_back(btn);

		// button that toggles storage menu
		UI.storage.icon_btn = new Button (
			screen_size, Button::br, glm::vec2(-190, -80), // position
			glm::vec2(64, 64), // size
			sprites.storage.icon, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.3f, // sprite scale
			Button::show_text, // hover behavior
			"storage", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				UI.storage.hidden = false;				
				UI.storage.icon_btn->hidden = true;
				for( int i=0; i<UI.storage.tabs.size(); i++) {
					UI.storage.tabs[i]->hidden = false;
					UI.storage.tabs[i]->hidden = false;
				}
			} );
		UI.all_buttons.push_back( UI.storage.icon_btn );

		// seeds tab button
		btn = new Button (
			screen_size, Button::br, glm::vec2(-300, -380), // position
			glm::vec2(64, 64), // size
			sprites.storage.seeds_tab, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.6f, // sprite scale
			Button::show_text, // hover behavior
			"seeds", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( UI.storage.current_tab == 0 ) {
					UI.storage.hidden = true;
					UI.storage.icon_btn->hidden = false;
					for( int i=0; i<UI.storage.tabs.size(); i++) {
						UI.storage.tabs[i]->hidden = true;
						UI.storage.tabs[i]->hidden = true;
					}
				} else {
					UI.storage.current_tab = 0;
				}
			}, true );
		UI.storage.tabs.push_back( btn );
		UI.all_buttons.push_back( btn );

		// harvest tab button
		btn = new Button (
			screen_size, Button::br, glm::vec2(-220, -380), // position
			glm::vec2(64, 64), // size
			sprites.storage.harvest_tab, // sprite
			glm::vec2(32, 32), // sprite anchor
			0.5f, // sprite scale
			Button::show_text, // hover behavior
			"harvest", // text
			glm::vec2(0, -20), // text anchor
			0.4f, // text scale
			[this]() {
				if( UI.storage.current_tab == 1 ) {
					UI.storage.hidden = true;
					UI.storage.icon_btn->hidden = false;
					for( int i=0; i<UI.storage.tabs.size(); i++) {
						UI.storage.tabs[i]->hidden = true;
						UI.storage.tabs[i]->hidden = true;
					}
				} else {
					UI.storage.current_tab = 1;
				}
			}, true );
		UI.storage.tabs.push_back( btn );
		UI.all_buttons.push_back( btn );


		//---- plant buttons
		auto add_plant_button = [this](PlantType const* plant) {
			Button* seed = nullptr;
			Button* harvest = nullptr;
			plant->make_buttons( screen_size, &selectedPlant, &current_tool, &seed, &harvest );
			assert(seed); assert(harvest);
			inventory.set_seed_btn( plant, seed );
			inventory.set_harvest_btn( plant, harvest );
			UI.storage.all_seeds.push_back(seed);
			UI.storage.all_harvest.push_back(harvest);
			UI.all_buttons.push_back(seed);
			UI.all_buttons.push_back(harvest);
		};

		add_plant_button( test_plant );
		add_plant_button( friend_plant );
		add_plant_button( vampire_plant );
		add_plant_button( cactus_plant );
		add_plant_button( fireflower_plant );
		add_plant_button( corpseeater_plant );


		/*
	    buttons.emplace_back (
			glm::vec2(380, 130), // position
			glm::vec2(80, 20), // size
			nullptr, // sprite
			glm::vec2(0, 0), // sprite anchor
			1.0f, // sprite scale
			Button::none, // hover behavior
			"Submit Order", // text
			glm::vec2(0, 0), // text anchor
			0.4f, // text scale
			[ this]() {
				std::cout << "Submit Button Click!" << std::endl;
				std::map< PlantType const*, int > require_plants =  current_order->get_require_plants();
				bool orderFinished = true;
				std::map<PlantType const*, int>::iterator iter = require_plants.begin();
				while(iter != require_plants.end()) {
					PlantType const* require_type = iter->first;
					int needed_num  = iter->second;
					// std::cout << "Harvest num " << require_type->get_name()<<" "<<harvest_plant_map[require_type]<<std::endl;
					if(harvest_plant_map[require_type]<needed_num){
						orderFinished = false;
						break;
					}
					iter++;	
   				}
				   std::cout << orderFinished << std::endl;
				if(orderFinished == true){
					iter = require_plants.begin();
					while(iter != require_plants.end()) {
						PlantType const* require_type = iter->first;
						int needed_num  = iter->second;
						harvest_plant_map[require_type] -= needed_num;
						iter++;
					}
					energy += current_order->get_bonus_cash();
					current_order_idx += 1;
					if(current_order_idx >= all_orders.size()){
						current_order_idx = 0;
					}
					current_order = all_orders[current_order_idx];
				}				
			} );

	    buttons.emplace_back (
			glm::vec2(540, 130), // position
			glm::vec2(80, 20), // size
			nullptr, // sprite
			glm::vec2(0, 0), // sprite anchor
			1.0f, // sprite scale
			Button::none, // hover behavior
			"Cancel Order", // text
			glm::vec2(0, 0), // text anchor
			0.4f, // text scale
			[this]() {
				current_order_idx += 1;
				if(current_order_idx >= all_orders.size()){
					current_order_idx = 0;
				}
				current_order = all_orders[current_order_idx];
				std::cout << "Cancel Button Click!" << std::endl;
			} );
	*/
	}
}

PlantMode::~PlantMode() {
	for (int i=0; i<grid.size_x; i++) {
		for (int j=0; j<grid.size_y; j++) {
			GroundTile& tile = grid.tiles[i][j];
			if( tile.fire_aura ) delete tile.fire_aura;
			if( tile.aqua_aura ) delete tile.aqua_aura;
		}
	}
	for (int i=0; i<UI.all_buttons.size(); i++) {
		if (UI.all_buttons[i]) delete UI.all_buttons[i];
	}
}

void PlantMode::on_click( int x, int y )
{
	//---- first detect click on UI. If UI handled the click, return.
	for( int i = 0; i < UI.all_buttons.size(); i++ )
	{
		if( UI.all_buttons[i]->try_click( glm::vec2(x, y) ) ) return;
	}

	//---- Otherwise, detect click on tiles.
	GroundTile* collided_tile = get_tile_under_mouse( x, y );

	if( collided_tile ) {

		if( current_tool == glove ) {
			if( collided_tile->plant_type ) {
				// Removing dead plant
				if( collided_tile->is_plant_dead() ) {
					collided_tile->try_remove_plant();
				}
				// Harvesting plant
				else if( collided_tile->is_tile_harvestable() ) {
					int gain = collided_tile->plant_type->get_harvest_gain();
					if( collided_tile->try_remove_plant() ) {
						energy += gain;
						inventory.change_harvest_num( collided_tile->plant_type, 1 );
					}
				}
			}

		} else if( current_tool == watering_can ) {

		} else if( current_tool == fertilizer ) {

		} else if( current_tool == shovel ) {
			if( collided_tile->can_be_cleared(grid) ) { // clearing the ground
				int cost = collided_tile->tile_type->get_clear_cost();
				if( cost <= energy && collided_tile->try_clear_tile() ) {
					energy -= cost;
				}
			}

		} else if( current_tool == seed ) {
			// Planting a plant
			if(selectedPlant && inventory.get_seeds_num( selectedPlant ) > 0) {
				if( collided_tile->try_add_plant( selectedPlant ) ) {
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

			glm::vec3 center = grid.tiles[x][y].tile_drawable->transform->position;
			(void)center;
			float scale = plant_grid_tile_size.x / 2.0f;

			glm::mat4x3 collider_to_world = grid.tiles[x][y].tile_drawable->transform->make_local_to_world();
			glm::vec3 a = collider_to_world * glm::vec4( glm::vec3(1.0f, 1.0f, 0.2f) * scale, 1.0f );
			glm::vec3 b = collider_to_world * glm::vec4( glm::vec3( -1.0f, 1.0f, 0.2f ) * scale, 1.0f );
			glm::vec3 c = collider_to_world * glm::vec4( glm::vec3( 1.0f, -1.0f, 0.2f ) * scale, 1.0f );
			glm::vec3 d = collider_to_world * glm::vec4( glm::vec3( -1.0f, -1.0f, 0.2f ) * scale, 1.0f );

			bool did_collide = collide_swept_sphere_vs_triangle(
				sphere_sweep_from, sphere_sweep_to, sphere_radius,
				a, b, c,
				&collision_t, &collision_at, &collision_out )
				|| collide_swept_sphere_vs_triangle(
				sphere_sweep_from, sphere_sweep_to, sphere_radius,
				b, d, c,
				&collision_t, &collision_at, &collision_out);

			if( did_collide )
			{
				collided_tile = &grid.tiles[x][y];
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
		case SDLK_6:
			selectedPlant = corpseeater_plant;
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
	const Uint8* state = SDL_GetKeyboardState( NULL );

	if( state[SDL_SCANCODE_A] )
	{
		camera_offset.x -= camera_move_speed * elapsed;
	}
	if( state[SDL_SCANCODE_D] )
	{
		camera_offset.x += camera_move_speed * elapsed;
	}
	if( state[SDL_SCANCODE_W] )
	{
		camera_offset.y += camera_move_speed * elapsed;
	}
	if( state[SDL_SCANCODE_S] )
	{
		camera_offset.y -= camera_move_speed * elapsed;
	}

	camera_offset = glm::clamp( camera_offset, camera_bounds_min, camera_bounds_max );
	glm::vec3 real_camera_offset = forward_dir * camera_offset.y + side_dir * camera_offset.x;

	// Update Camera Position
	{
		float ce = std::cos( camera_elevation );
		float se = std::sin( camera_elevation );
		float ca = std::cos( camera_azimuth );
		float sa = std::sin( camera_azimuth );
		camera->transform->position = real_camera_offset + camera_radius * glm::vec3( ce * ca, ce * sa, se );
		camera->transform->rotation =
			glm::quat_cast( glm::transpose( glm::mat3( glm::lookAt(
			camera->transform->position ,
			real_camera_offset,
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
		// apply pending update from neighboring tiles
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid.tiles[x][y].apply_pending_update( elapsed );
			}
		}
		// other visuals
		for( int32_t x = 0; x < plant_grid_x; ++x )
		{
			for( int32_t y = 0; y < plant_grid_y; ++y )
			{
				grid.tiles[x][y].update_aura_visuals( elapsed, camera->transform );
			}
		}
	}

	int x, y;
	SDL_GetMouseState( &x, &y );
	
	// Query for hovered tile
	GroundTile* hovered_tile = get_tile_under_mouse( x, y );
	if( hovered_tile ) {
		//---- update action description
		action_description = "";

		if( current_tool == glove ) {
			if( hovered_tile->plant_type ) 
			{
				if( hovered_tile->is_plant_dead() )
				{
					action_description = "Remove ";
				}
				else if( hovered_tile->is_tile_harvestable() )
				{
					action_description = "Harvest +" + std::to_string( hovered_tile->plant_type->get_harvest_gain());
				}
				else
				{
					action_description = "Growing ";
				}
			}

		} else if( current_tool == watering_can ) {
			if( hovered_tile->tile_type->get_can_plant() && hovered_tile->moisture < 1.0f ) {
				action_description = "Water ";
			}

		} else if( current_tool == fertilizer ) {
			if( hovered_tile->fertility < 1.0f ) {
				action_description = "Fertilize";
			}

		} else if( current_tool == shovel ) {
			if( hovered_tile->can_be_cleared(grid) ) {
				action_description = "Dig -" + std::to_string(hovered_tile->tile_type->get_clear_cost());
			}

		} else if( current_tool == seed ) {
			if( selectedPlant 
					&& hovered_tile->tile_type->get_can_plant()
					&& !hovered_tile->plant_type ) {
				action_description = "Plant ";
			}
		}

		//---- update tile properties (watering & fertilizing)
		if( SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT) ) {// left btn down
			if( current_tool == watering_can ) {
				hovered_tile->moisture = std::min(1.0f, hovered_tile->moisture + elapsed);

			} else if( current_tool == fertilizer ) {
				hovered_tile->fertility = std::min(1.0f, hovered_tile->fertility + elapsed * 2.0f);
			}
		}
	} 

	//Selector positioning
	if( hovered_tile )
	{
		selector->transform->position = hovered_tile->tile_drawable->transform->position + glm::vec3( 0.0f, 0.0f, -0.03f );
	}
	else
	{
		selector->transform->position = glm::vec3( 0.0f, 0.0f, -1000.0f );
	}

	{ // update UI and cursor
		// update buttons' hovered state
		for( int i = 0; i < UI.all_buttons.size(); i++) {
			UI.all_buttons[i]->update_hover( glm::vec2(x, y) );
		}
		// update cursor sprite depending on current tool
		switch( current_tool ) {
		case none:
			cursor.sprite = sprites.cursor.regular;
			cursor.scale = 1.0f;
			cursor.offset = glm::vec2(0, 0);
			break;
		case seed: //TODO
			cursor.sprite = sprites.cursor.hand;
			cursor.scale = 1.0f;
			cursor.offset = glm::vec2(0, 0);
			break;
		case glove:
			cursor.sprite = sprites.tools.glove;
			cursor.scale = 0.24f;
			cursor.offset = glm::vec2(0, 0);
			break;
		case watering_can:
			cursor.sprite = sprites.tools.watering_can;
			cursor.scale = 0.24f;
			cursor.offset = glm::vec2(0, 0);
			break;
		case fertilizer:
			cursor.sprite = sprites.tools.fertilizer;
			cursor.scale = 0.24f;
			cursor.offset = glm::vec2(0, 0);
			break;
		case shovel:
			cursor.sprite = sprites.tools.shovel;
			cursor.scale = 0.24f;
			cursor.offset = glm::vec2(0, 0);
			break;
		}
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
	glClearColor(86.0f / 255.0f, 110.0f / 255.0f, 139.0f / 255.0f, 1.0f);
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
	{ // actual drawing: create draw_aura instance and append the vertices
		DrawAura draw_aura( world_to_clip );
		for (int i=0; i<grid.size_x; i++) {
			for (int j=0; j<grid.size_y; j++) {
				if (grid.tiles[i][j].fire_aura) grid.tiles[i][j].fire_aura->draw( draw_aura );
				if (grid.tiles[i][j].aqua_aura) grid.tiles[i][j].aqua_aura->draw( draw_aura );
			}
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
	//current_order->draw(drawable_size);

	// draw order hint
	{
		DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		std::map< PlantType const*, int > require_plants =  current_order->get_require_plants();
		std::map<PlantType const*, int>::iterator iter = require_plants.begin();
		float text_gap = 80.0f;
		while(iter != require_plants.end()) {
			const PlantType* require_type = iter->first;
			// std:: string name = require_type->get_name();
			int require_num  = iter->second;
			int needed_num = 0;
			if( inventory.get_harvest_num(require_type) < require_num ){
				needed_num = require_num - inventory.get_harvest_num(require_type);
			}
			
			// draw.draw_text( "You Still Need: "+ std::to_string(needed_num)+" "+require_type->get_name(), glm::vec2( 400.0f, drawable_size.y - text_gap ), 0.4f);
			iter++;	
			text_gap += 10.0f;
		}
	}

	{ //draw all the text
		DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text( selectedPlant->get_name() + " x" + std::to_string(inventory.get_seeds_num(selectedPlant)) +" :", glm::vec2( 20.0f, drawable_size.y - 20.0f ), 0.8f);
		draw.draw_text( selectedPlant->get_description(), glm::vec2( 20.0f, drawable_size.y - 60.0f ), 0.6f );
		draw.draw_text( "Energy: " + std::to_string( energy ), glm::vec2( drawable_size.x - 160.0f, drawable_size.y - 20.0f ), 0.6f );

		glm::mat4 world_to_clip = camera->make_projection() * camera->transform->make_world_to_local();
		glm::vec4 sel_clip = world_to_clip * selector->transform->make_local_to_world() * glm::vec4( 0.0f, 0.0f, 0.0f, 1.0f );
		if( sel_clip.w > 0.0f )
		{
			glm::vec3 sel_clip_xyz = glm::vec3( sel_clip );
			glm::vec3 sel_clip_pos = sel_clip_xyz / sel_clip.w;
			glm::vec2 sel_clip_pos_xy = glm::vec2( sel_clip_pos );
			glm::vec2 window_pos = glm::vec2(( ( sel_clip_pos_xy.x + 1.0f ) / 2.0f ) * drawable_size.x, ( ( sel_clip_pos_xy.y + 1.0f ) / 2.0f ) * drawable_size.y );
			float scale = 0.6f;
			glm::vec2 extent_min, extent_max;
			draw.get_text_extents( action_description, glm::vec2( 0.0f, 0.0f ), scale, &extent_min, &extent_max );
			glm::vec2 textbox_size = extent_max - extent_min;
			draw.draw_text( action_description, window_pos + glm::vec2(-textbox_size.x, textbox_size.y)/2.0f + glm::vec2(0.0f, 10.0f) * sel_clip_pos.z, scale / sel_clip_pos.z );
		}
		
		// draw hint text
		draw.draw_text("Press Space to open magic book", glm::vec2( 10.0f, 150.0f ), 0.6f );
	}

	{ //draw UI
		{ //sprites
			DrawSprites draw_sprites( *main_atlas, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
			
			//---- tools
			// background
			draw_sprites.draw( *sprites.tools.background, glm::vec2(-100, 0), 0.3f );
			// tools
			for (int i=0; i<UI.tools.size(); i++) {
				UI.tools[i]->draw_sprite( draw_sprites );
			}

			//---- storage
			if( !UI.storage.hidden ) {
				// unselected tab(s)
				for( int i=0; i<UI.storage.tabs.size(); i++) {
					if( i != UI.storage.current_tab ) UI.storage.tabs[i]->draw_sprite( draw_sprites );
				}
				// background
				draw_sprites.draw( *sprites.storage.background, glm::vec2(screen_size.x, 290) + UI.storage.br_offset, 0.5f );
				// selected tab
				UI.storage.tabs[UI.storage.current_tab]->draw_sprite( draw_sprites );
			} else {
				// background only, no tabs
				draw_sprites.draw( *sprites.storage.background, glm::vec2(screen_size.x, 0) + UI.storage.br_offset, 0.5f );
			}
			// icon
			UI.storage.icon_btn->draw_sprite( draw_sprites );
		}
		// plant-related (seed & harvest)
		if( !UI.storage.hidden ){
			DrawSprites draw_text( neucha_font, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
			DrawSprites draw_plants( *plants_atlas, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
			// seeds
			std::vector< std::pair<PlantType const*, int> > seeds_to_draw;
			auto plant_to_seeds = inventory.get_plant_to_seeds();
			for( auto p : plant_to_seeds ) {
				Button* seed_btn = inventory.get_seed_btn( p.first );
				seed_btn->hidden = true;
				(void)seed_btn;
				if( p.second > 0 ) seeds_to_draw.push_back( p );
			}
			std::sort( seeds_to_draw.begin(), seeds_to_draw.end(), Inventory::comp_fn );
			for( int i=0; i<seeds_to_draw.size(); i++ ) {
				PlantType const* plant = seeds_to_draw[i].first;
				int num_seeds = seeds_to_draw[i].second;
				Button* seed_btn = inventory.get_seed_btn( plant );
				glm::vec2 cell_position = UI.storage.get_cell_position(i);
				seed_btn->hidden = false;
				seed_btn->set_position(Button::br, cell_position, screen_size);
				seed_btn->draw_sprite( draw_plants );
				// seed num text
				glm::vec2 text_position = screen_size + cell_position;
				draw_text.draw_text( std::to_string(num_seeds), 
						glm::vec2(text_position.x, screen_size.y - text_position.y) + glm::vec2(10, -10), 0.35f );
			}
		}

		{ //text
			DrawSprites draw_text( neucha_font, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
			for (int i=0; i<UI.all_buttons.size(); i++) {
				UI.all_buttons[i]->draw_text( draw_text );
			}
		}
		// cursor
		DrawSprites draw_cursor( *main_atlas, glm::vec2(0, 0), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		draw_cursor.draw( 
				*cursor.sprite, 
				glm::vec2(mouse_x + cursor.offset.x, drawable_size.y - mouse_y - cursor.offset.y),
				cursor.scale);
	}
   
	if(is_magicbook_open && Mode::current.get() == this)
	{
		open_book();
	}

}

void PlantMode::on_resize( glm::uvec2 const& new_drawable_size )
{
	{ // init the opengl stuff
		screen_size = glm::vec2( new_drawable_size.x, new_drawable_size.y );
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
	{// re-position buttons
		for( int i = 0; i < UI.all_buttons.size(); i++ ) {
			UI.all_buttons[i]->update_position( screen_size );
		}
	}
}

int Inventory::get_seeds_num(const PlantType* plant ) 
{
	std::unordered_map<PlantType const*, int>::iterator it = plant_to_seeds.find( plant );
	if( it != plant_to_seeds.end())
	{
		return it->second;
	}
	else
	{
		plant_to_seeds.insert( std::make_pair( plant, 0 ) );
		return 0;
	}
}

void Inventory::change_seeds_num(const PlantType* plant, int seed_change )
{
	std::unordered_map<PlantType const*, int>::iterator it = plant_to_seeds.find( plant );
	if( it != plant_to_seeds.end() )
	{
		it->second += seed_change;
	}
	else
	{
		plant_to_seeds.insert( std::make_pair( plant, seed_change ) );
	}
}

int Inventory::get_harvest_num( const PlantType* plant ) {
	std::unordered_map<PlantType const*, int>::iterator it = plant_to_harvest.find( plant );
	if( it != plant_to_harvest.end() ) {
		return it->second;
	} else {
		plant_to_harvest.insert( std::make_pair( plant, 0 ) );
		return 0;
	}
}

void Inventory::change_harvest_num( const PlantType* plant, int harvest_change ) {
	std::unordered_map<PlantType const*, int>::iterator it = plant_to_harvest.find( plant );
	if( it != plant_to_harvest.end() ) {
		it->second += harvest_change;
	} else {
		plant_to_harvest.insert( std::make_pair( plant, harvest_change ) );
	}
}

Button* Inventory::get_seed_btn( const PlantType* plant ) {
	std::unordered_map<PlantType const*, Button*>::iterator it = plant_to_seed_btn.find( plant );
	assert( it != plant_to_seed_btn.end() );
	return it->second;
}

Button* Inventory::get_harvest_btn( const PlantType* plant ) {
	std::unordered_map<PlantType const*, Button*>::iterator it = plant_to_harvest_btn.find( plant );
	assert( it != plant_to_harvest_btn.end() );
	return it->second;
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
		menu->atlas = neucha_font->atlas; // for test 
		menu->view_min = view_min;
		menu->view_max = view_max;
		menu->background = shared_from_this();
		Mode::current = menu;
	}
