#include "Load.hpp"
// #include "Mesh.hpp"
// #include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Order.hpp"
#include <cstddef>
#include <algorithm>
#include <iostream>

Sprite const *orderbg_sprite = nullptr;
OrderType const* order1 = nullptr;
OrderType const* order2 = nullptr;

Load< SpriteAtlas > order_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *kret = new SpriteAtlas(data_path("orders"));
	orderbg_sprite = &kret->lookup("orderBg");
    std::map< PlantType const*, int > require_plants_in1;
	require_plants_in1[test_plant] = 4;
    order1 = new OrderType("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,nullptr);
		order2 = new OrderType("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,nullptr);
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
		glm::vec2 ul = glm::vec2(view_min.x, view_max.y);
  	draw.draw(*orderbg_sprite, ul);
	}//<-- gets drawn on deallocation
	

	{// draw text
		DrawSprites draw(*font_atlas_tmp, text_min, text_max, drawable_size, DrawSprites::AlignPixelPerfect);
		// draw.draw_text( "LALAL ", glm::vec2( drawable_size.x - 160.0f, drawable_size.y - 40.0f ), 2.0f );
		draw.draw_text(get_title(), glm::vec2(view_min.x-80.0f, 210.0f), 0.7f, glm::u8vec4(0xff,0xff,0xff,0xff));
		draw.draw_text(get_description(), glm::vec2(view_min.x-80, 180.0f), 0.6f, glm::u8vec4(0xff,0xff,0xff,0xff));
	}//<-- gets drawn on deallocation
}

