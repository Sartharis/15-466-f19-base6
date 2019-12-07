#define _CRT_SECURE_NO_WARNINGS

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
#include "PlantMode.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <map>
#include <random>
#include <unordered_map>

std::vector< OrderType const* > main_orders;
std::vector< OrderType const* > daily_orders;

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
	}
	else if( plant_type_name == "spreader_source_plant" ){
		return spreader_source_plant;
	}
	else if( plant_type_name == "spreader_child_plant" ){
		return spreader_child_plant;
	}
	else if( plant_type_name == "teleporter_plant" ){
		return teleporter_plant;
	}
	else{
		return nullptr;
	}
}

// Randomly generate daily order with combination of different required plants
OrderType const* generate_random_daily_order(){
	std::map< PlantType const*, int > rand_require_plants;
	int rand_required_plants_type_num = rand()%3+1;
	int rand_bonus_energy = 0;
	for(int i=0;i<rand_required_plants_type_num;i++){
		int rand_required_num = rand()%5+1;
		rand_bonus_energy += rand()%125 + 55;
		int rand_plant_type = rand()%all_plants.size();
		rand_require_plants.insert(std::pair<PlantType const*, int>(all_plants[rand_plant_type],rand_required_num));
	}
	OrderType const* tmp_daily_order = new OrderType("random order", "Someone ordered these..",rand_require_plants, rand_bonus_energy,nullptr);
	return tmp_daily_order;
}

Load< void > daily_order_load_from_file(LoadTagLate, []() {
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
			// std::cout <<"title:"<< tmp_title << std::endl;
			new_order_count = 2;
		}else if(new_order_count==2){
			tmp_description = line.substr(12);
			// std::cout <<"description:"<< tmp_description << std::endl;
			new_order_count = 3;
		}else if(new_order_count==3){
			if(sscanf(line.c_str(),"number_of_required_plants_type:%d",&d)){
				num_of_plants_type = d;
			}
			// std::cout <<"num:"<< num_of_plants_type << std::endl;
			new_order_count = 4;
		}else if(new_order_count <= num_of_plants_type+3){
			int pos = (int)line.find(" ");
			std::string tmp_name = line.substr(0,pos);
			int plant_num = std::stoi(line.substr(pos+1));
			tmp_require_plants.insert( std::pair<PlantType const*, int>(get_plant_type_by_name(tmp_name), plant_num));
			// std::cout <<tmp_name<< " " << plant_num<< std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+4){
			if(sscanf(line.c_str(),"bonus_energy:%d",&d)){
				tmp_bonus_energy = d;
			}
			// std::cout <<"energy:"<< tmp_bonus_energy << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+5){
			// std::cout << line.substr(12) << std::endl;
			PlantType const* tmp_bonus_plant_in = get_plant_type_by_name(line.substr(12));
			// std::cout << "tmp plant" << std::endl;
			OrderType const* tmp_daily_order = new OrderType(tmp_title, tmp_description,tmp_require_plants, tmp_bonus_energy,tmp_bonus_plant_in);
			// std::cout << "tmp order done" << std::endl;
			daily_orders.push_back(tmp_daily_order);
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
    
	// This part is to randomly generate three daily order.
	for(int i=0;i<3;i++){
		daily_orders.push_back(generate_random_daily_order());
	}

	// validity check
	for (auto order : daily_orders) {
		assert(order);
		auto reqs = order->get_required_plants();
		for (auto req : reqs) {
			assert(req.first);
		}
	}

	std::cout << "----all daily orders loaded." << std::endl;
});

Load< void > main_order_load_from_file(LoadTagLate, []() {
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
			// std::cout <<"title:"<< tmp_title << std::endl;
			new_order_count = 2;
		}else if(new_order_count==2){
			tmp_description = line.substr(12);
			// std::cout <<"description:"<< tmp_description << std::endl;
			new_order_count = 3;
		}else if(new_order_count==3){
			if(sscanf(line.c_str(),"number_of_required_plants_type:%d",&d)){
				num_of_plants_type = d;
			}
			// std::cout <<"num:"<< num_of_plants_type << std::endl;
			new_order_count = 4;
		}else if(new_order_count <= num_of_plants_type+3){
			int pos = (int)line.find(" ");
			std::string tmp_name = line.substr(0,pos);
			int plant_num = std::stoi(line.substr(pos+1));
			tmp_require_plants.insert( std::pair<PlantType const*, int>(get_plant_type_by_name(tmp_name), plant_num));
			// std::cout <<tmp_name<< " " << plant_num<< std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+4){
			if(sscanf(line.c_str(),"bonus_energy:%d",&d)){
				tmp_bonus_energy = d;
			}
			// std::cout <<"energy:"<< tmp_bonus_energy << std::endl;
			new_order_count += 1;
		}else if(new_order_count == num_of_plants_type+5){
			// std::cout << line.substr(12) << std::endl;
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
	// validity check
	for (auto order : main_orders) {
		assert(order);
		auto reqs = order->get_required_plants();
		for (auto req : reqs) {
			assert(req.first);
		}
	}
	std::cout << "----all main orders loaded." << std::endl;
});

