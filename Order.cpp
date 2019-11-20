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

OrderType const* order1 = nullptr;
OrderType const* order2 = nullptr;
OrderType const* order3 = nullptr;
OrderType const* order4 = nullptr;
OrderType const* order5 = nullptr;
OrderType const* order6 = nullptr;
OrderType const* order7 = nullptr;
std::list<OrderType const*> main_orders;

Load< void > order_atlas(LoadTagLate, []() {
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
});

// TODO: would ideally get rid of this along with the draw function (and write the draw code in PlantMode::draw directly)
Load< SpriteAtlas > font_atlas_tmp( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

Load< void > main_order_load_from_file(LoadTagLate, []() {
	std::cout << "----main order loaded:" << std::endl;
	std::string const &path = data_path("main_order");
	std::ifstream infile(path);
	std::string line;
	std::string tmp_title;
	std::string tmp_description;
	std::map< PlantType const*, int > tmp_require_plants;
	int tmp_bonus_energy=0;
	
	int new_order_count = 0;
	int num_of_plants_type = 0;
    
	while(std::getline(infile,line)){
		int d = 0;
		if(new_order_count == 0){
			if(line == "<"){		
				new_order_count =1;
			}
		} else if(new_order_count == 1){
			tmp_title = line.substr(6); 
			std::cout <<"title:"<< tmp_title << std::endl;
			new_order_count = 2;
		}else if(new_order_count==2){
			tmp_description = line.substr(12);
			std::cout <<"description:"<< tmp_description << std::endl;
			new_order_count = 3;
		}else if(new_order_count==3){
			if(sscanf(line.c_str(),"number_of_required_plants_type:%d",&d)){
				num_of_plants_type = d;
			}
			std::cout <<"num:"<< num_of_plants_type << std::endl;
			new_order_count = 4;
		}else if(new_order_count <= num_of_plants_type+3){
			int pos = line.find(" ");
			std::string tmp_name = line.substr(0,pos);
			int plant_num = std::stoi(line.substr(pos+1));
			tmp_require_plants.insert( std::pair<PlantType const*, int>(get_plant_type_by_name(tmp_name), plant_num));
			std::cout <<tmp_name<< " " << plant_num<< std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+4){
			if(sscanf(line.c_str(),"bonus_energy:%d",&d)){
				tmp_bonus_energy = d;
			}
			std::cout << tmp_bonus_energy << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+5){
			std::cout << line.substr(12) << std::endl;
			PlantType const* tmp_bonus_plant_in = get_plant_type_by_name(line.substr(12));
			OrderType const* tmp_main_order = new OrderType(tmp_title, tmp_description,tmp_require_plants, tmp_bonus_energy,tmp_bonus_plant_in);
			main_orders.push_back(tmp_main_order);
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+6){
			if(line == ">"){		
				new_order_count =0;
				new_order_count = 0;
	 			num_of_plants_type = 0;
			}
		}
		
	}
	
});


PlantType const* get_plant_type_by_name(std::string plant_type_name){
	if(plant_type_name=="test_plant"){
		return test_plant;
	}else if(plant_type_name=="friend_plant"){
		return friend_plant;
	}else if(plant_type_name=="vampire_plant"){
		return vampire_plant;
	}else if(plant_type_name=="cactus_plant"){
		return cactus_plant;
	}else if(plant_type_name=="fireflower_plant"){
		return fireflower_plant;
	}else if(plant_type_name=="corpseeater_plant"){
		return corpseeater_plant;
	}else{
		return nullptr;
	}
}


void OrderType::draw(glm::uvec2 const &drawable_size, Inventory& inventory) const {

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{// draw sprite
		DrawSprites draw(*main_atlas, glm::vec2(0,0), drawable_size, drawable_size, DrawSprites::AlignSloppy);
		glm::vec2 ul = glm::vec2( drawable_size.x - 460.0f, drawable_size.y);
  	draw.draw(*order_background_sprite, ul, 0.45f);
	}//<-- gets drawn on deallocation
	

	{// draw text
		DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text("ORDER for $" + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x  - 400.0f, drawable_size.y - 25.0f), 0.6f, text_col);
		//draw.draw_text(get_description(), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 50.0f ), 0.6f );
		

		auto iter = require_plants.begin();
		float text_gap = 0.0f;

		while( iter != require_plants.end() ) {
			const PlantType* require_type = iter->first;
			if( require_type )
			{
				int require_num = iter->second;
				draw.draw_text( " - " + require_type->get_name() + ": " + std::to_string( inventory.get_harvest_num( require_type ) ) + "/" + std::to_string(require_num) , 
								glm::vec2( drawable_size.x - 400.0f, drawable_size.y - 75.0f - text_gap ), 0.4f, text_col );

			}

			iter++;
			text_gap += 20.0f;

		}

		//draw.draw_text( "Payment: " + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 105.0f - text_gap ), 0.6f );
	}//<-- gets drawn on deallocation
}

