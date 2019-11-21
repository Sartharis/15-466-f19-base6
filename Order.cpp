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

std::vector< OrderType const* > main_orders;
std::vector< OrderType const* > all_orders;

// TODO: would ideally get rid of this along with the draw function (and write the draw code in PlantMode::draw directly)
Load< SpriteAtlas > font_atlas_tmp( LoadTagDefault, []() -> SpriteAtlas const* {
	return new SpriteAtlas( data_path( "trade-font" ) );
} );

Load< void > daily_order_load_from_file(LoadTagLate, []() {
	std::cout << "----daily order loaded:" << std::endl;
	std::string const &path = data_path("daily_order");
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
			std::cout <<"energy:"<< tmp_bonus_energy << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+5){
			std::cout << line.substr(12) << std::endl;
			PlantType const* tmp_bonus_plant_in = get_plant_type_by_name(line.substr(12));
			// std::cout << "tmp plant" << std::endl;
			OrderType const* tmp_daily_order = new OrderType(tmp_title, tmp_description,tmp_require_plants, tmp_bonus_energy,tmp_bonus_plant_in);
			// std::cout << "tmp order done" << std::endl;
			all_orders.push_back(tmp_daily_order);
			// std::cout << "main order done" << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+6){
			if(line == ">"){		
				new_order_count =0;
				new_order_count = 0;
	 			num_of_plants_type = 0;
				tmp_require_plants.erase(tmp_require_plants.begin(),tmp_require_plants.end());
			}
		}
		
	}
	
});

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
			std::cout <<"energy:"<< tmp_bonus_energy << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+5){
			std::cout << line.substr(12) << std::endl;
			PlantType const* tmp_bonus_plant_in = get_plant_type_by_name(line.substr(12));
			// std::cout << "tmp plant" << std::endl;
			OrderType const* tmp_main_order = new OrderType(tmp_title, tmp_description,tmp_require_plants, tmp_bonus_energy,tmp_bonus_plant_in);
			// std::cout << "tmp order done" << std::endl;
			main_orders.push_back(tmp_main_order);
			// std::cout << "main order done" << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+6){
			if(line == ">"){		
				new_order_count =0;
				new_order_count = 0;
	 			num_of_plants_type = 0;
				tmp_require_plants.erase(tmp_require_plants.begin(),tmp_require_plants.end());
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

void OrderType::draw_main_order(glm::uvec2 const &drawable_size, Inventory& inventory) const {

	//use alpha blending:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//don't use the depth test:
	glDisable(GL_DEPTH_TEST);

	{// draw sprite
		DrawSprites draw(*main_atlas, glm::vec2(0,0), drawable_size, drawable_size, DrawSprites::AlignSloppy);
		glm::vec2 ul = glm::vec2( drawable_size.x - 860.0f, drawable_size.y);
  	draw.draw(*order_background_sprite, ul, 0.45f);
	}//<-- gets drawn on deallocation
	

	{// draw text
		DrawSprites draw( neucha_font, glm::vec2( 0.0f, 0.0f ), drawable_size, drawable_size, DrawSprites::AlignSloppy );
		draw.draw_text("Main ORDER for $" + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x  - 800.0f, drawable_size.y - 25.0f), 0.6f, text_col);
		//draw.draw_text(get_description(), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 50.0f ), 0.6f );
		

		auto iter = require_plants.begin();
		float text_gap = 0.0f;

		while( iter != require_plants.end() ) {
			const PlantType* require_type = iter->first;
			if( require_type )
			{
				int require_num = iter->second;
				draw.draw_text( " - " + require_type->get_name() + ": " + std::to_string( inventory.get_harvest_num( require_type ) ) + "/" + std::to_string(require_num) , 
								glm::vec2( drawable_size.x - 800.0f, drawable_size.y - 75.0f - text_gap ), 0.4f, text_col );

			}

			iter++;
			text_gap += 20.0f;

		}
		draw.draw_text("Bonus Plant: " + get_bonus_plant()->get_name(), glm::vec2( drawable_size.x  - 800.0f, drawable_size.y - 85.0f - text_gap), 0.4f, text_col);
		
		//draw.draw_text( "Payment: " + std::to_string( get_bonus_cash() ), glm::vec2( drawable_size.x - 500.0f, drawable_size.y - 105.0f - text_gap ), 0.6f );
	}//<-- gets drawn on deallocation
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

