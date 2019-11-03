//orderBg
#include "Load.hpp"
#include "Mesh.hpp"
#include "Scene.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"
#include <cstddef>
#include <algorithm>
#include <iostream>


Sprite const *orderbg_sprite = nullptr;
order1 = nullptr;
order2 = nullptr;
current_order = nullptr;
Load< SpriteAtlas > order_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	SpriteAtlas const *kret = new SpriteAtlas(data_path("orders"));
	orderbg_sprite =  &kret->lookup("orderBg");
    std::map< std::PlantType const*, std::int > require_plants_in1;
	require_plants_in1[test_plant] = 4;
    order1 = new Order("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. 
    Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,none);
	// order2 = new Order("Welcome to solidarity, here is your first order", "Hi, I am your negibour Jacy. 
    // Recently I am refurbishing my house. Could you please help me find the follwing items?",require_plants_in1,300,none);
	return kret;
});

void Order::draw() {
    //clear the color buffer:
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);
    DrawSprites draw(*order_atlas, view_min, view_max, drawable_size, DrawSprites::AlignPixelPerfect);
	glm::vec2 ul = glm::vec2(view_min.x, view_max.y);
    draw.draw(*orderbg_sprite, ul);
    draw.draw_text(current_order->get_title(), glm::vec2(x, 2.0f), 1.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
    draw.draw_text(current_order->get_description(), glm::vec2(x, 4.0f), 1.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
    

}

