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

Load< SpriteAtlas > order_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *kret = new SpriteAtlas(data_path("orders"));
	orderbg_sprite = &kret->lookup("orderBg");
	std::map< PlantType const*, int > require_plants_in1;
	require_plants_in1[test_plant] = 4;
	std::map< PlantType const*, int > require_plants_in2;
	require_plants_in2[cactus_plant] = 2;
	std::map< PlantType const*, int > require_plants_in3;
	require_plants_in3[friend_plant] = 1;
	// require_plants_in3[test_plant] = 5;
	std::map< PlantType const*, int > require_plants_in4;
	require_plants_in4[vampire_plant] = 1;
	// order1 = new OrderType("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,nullptr);
	// order2 = new OrderType("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,nullptr);
	order1 = new OrderType("Here is your order", "Please Sell me four ferns",require_plants_in1,300,nullptr);
	order2 = new OrderType("Here is your order", "Please Help me find two cactus",require_plants_in2,350,nullptr);
	order3 = new OrderType("Here is your order", "Please Help me find one friend plant",require_plants_in3,450,nullptr);	
	order4 = new OrderType("Here is your order", "Please Help me find one sapsucker plant",require_plants_in4,300,nullptr);	
	return kret;
});

// TODO: would ideally get rid of this along with the draw function (and write the draw code in PlantMode::draw directly)
Load< SpriteAtlas > font_atlas_tmp( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

void OrderType::draw(glm::uvec2 const &drawable_size) const {

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{// draw sprite
		DrawSprites draw(*order_atlas, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);
		glm::vec2 ul = glm::vec2(view_min.x+350.0f, view_max.y);
  		draw.draw(*orderbg_sprite, ul);
	}//<-- gets drawn on deallocation
	

	{// draw text
		// DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		// draw.draw_text( selectedPlant->get_name() + " (" + std::to_string(inventory.get_seeds_num(selectedPlant)) +") :", glm::vec2( 20.0f, drawable_size.y - 20.0f ), 0.8f);
		
		DrawSprites draw(neucha_font, text_min, text_max, drawable_size, DrawSprites::AlignPixelPerfect);
		// draw.draw_text( "LALAL ", glm::vec2( drawable_size.x - 160.0f, drawable_size.y - 40.0f ), 2.0f );
		draw.draw_text(get_title(), glm::vec2(view_min.x+90.0f, 256.0f), 0.2f, glm::u8vec4(0xff,0xff,0xff,0xff));
		draw.draw_text(get_description(), glm::vec2(view_min.x+75.0f, 245.0f), 0.2f, glm::u8vec4(0xff,0xff,0xff,0xff));
		draw.draw_text("Bonus energy: "+std::to_string(get_bonus_cash()),glm::vec2(view_min.x+75.0f, 235.0f), 0.2f, glm::u8vec4(0xff,0xff,0xff,0xff));
	}//<-- gets drawn on deallocation

}

