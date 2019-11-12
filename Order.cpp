#include "Load.hpp"
// #include "Mesh.hpp"
// #include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Order.hpp"
#include "Button.hpp"
#include <cstddef>
#include <algorithm>
#include <iostream>
#include "PlantMode.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <map>
#include <random>
#include <unordered_map>

Sprite const *orderbg_sprite = nullptr;
OrderType const* order1 = nullptr;
OrderType const* order2 = nullptr;
OrderType const* order3 = nullptr;
OrderType const* order4 = nullptr;
OrderType const* order5 = nullptr;
OrderType const* order6 = nullptr;
OrderType const* order7 = nullptr;

Load< SpriteAtlas > order_atlas(LoadTagLate, []() -> SpriteAtlas const * {
	SpriteAtlas const *kret = new SpriteAtlas(data_path("orders"));
	orderbg_sprite = &kret->lookup("orderBg");
	std::map< PlantType const*, int > require_plants_in1;
	require_plants_in1.insert( std::pair<PlantType const*, int>( test_plant, 4 ) );

	std::map< PlantType const*, int > require_plants_in2;
	require_plants_in2.insert( std::pair<PlantType const*, int>( friend_plant, 1 ) );
	require_plants_in1.insert( std::pair<PlantType const*, int>( test_plant, 2 ) );

	std::map< PlantType const*, int > require_plants_in3;
	require_plants_in3.insert( std::pair<PlantType const*, int>( cactus_plant, 3 ) );

	std::map< PlantType const*, int > require_plants_in4;
	require_plants_in4.insert( std::pair<PlantType const*, int>( friend_plant, 2 ) );

	std::map< PlantType const*, int > require_plants_in5;
	require_plants_in5.insert( std::pair<PlantType const*, int>( vampire_plant, 1 ) );

	std::map< PlantType const*, int > require_plants_in6;
	require_plants_in6.insert( std::pair<PlantType const*, int>( corpseeater_plant, 1 ) );
	require_plants_in6.insert( std::pair<PlantType const*, int>( friend_plant, 2 ) );

	std::map< PlantType const*, int > require_plants_in7;
	require_plants_in7.insert( std::pair<PlantType const*, int>( vampire_plant, 1 ) );
	require_plants_in5.insert( std::pair<PlantType const*, int>( corpseeater_plant, 2 ) );

	order1 = new OrderType("ORDER", "I need some ferns.",require_plants_in1, 150,nullptr);
	order2 = new OrderType("ORDER", "I need cacti for self-defense",require_plants_in2,200,nullptr);
	order3 = new OrderType("ORDER", "My dad has been lonely lately. I need companion carrots.",require_plants_in3,250,nullptr);	
	order4 = new OrderType("ORDER", "I need a Sap Sucker, don't ask me why.",require_plants_in4,300,nullptr);	
	order5 = new OrderType( "ORDER", "I need a Sap Sucker, don't ask me why.", require_plants_in5, 350, nullptr );
	order6 = new OrderType( "ORDER", "I need a Sap Sucker, don't ask me why.", require_plants_in6, 400, nullptr );
	order7 = new OrderType( "ORDER", "I need a Sap Sucker, don't ask me why.", require_plants_in7, 450, nullptr );
	return kret;
});

// TODO: would ideally get rid of this along with the draw function (and write the draw code in PlantMode::draw directly)
Load< SpriteAtlas > font_atlas_tmp( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

void OrderType::draw(glm::uvec2 const &drawable_size, Inventory& inventory) const {

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{// draw sprite
		DrawSprites draw(*order_atlas, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);
		glm::vec2 ul = glm::vec2(view_min.x+350.0f, view_max.y);
  		//draw.draw(*orderbg_sprite, ul);
	}//<-- gets drawn on deallocation
	

	{// draw text
		DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text("ORDER for $" + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x  - 400.0f, drawable_size.y - 25.0f), 0.6f);
		//draw.draw_text(get_description(), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 50.0f ), 0.6f );
		

		auto iter = require_plants.begin();
		float text_gap = 0.0f;

		while( iter != require_plants.end() ) {
			const PlantType* require_type = iter->first;
			if( require_type )
			{
				int require_num = iter->second;
				draw.draw_text( " - " + require_type->get_name() + ": " + std::to_string( inventory.get_harvest_num( require_type ) ) + "/" + std::to_string(require_num) , 
								glm::vec2( drawable_size.x - 400.0f, drawable_size.y - 75.0f - text_gap ), 0.4f );

			}

			iter++;
			text_gap += 20.0f;

		}

		//draw.draw_text( "Payment: " + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 105.0f - text_gap ), 0.6f );
	}//<-- gets drawn on deallocation
}

