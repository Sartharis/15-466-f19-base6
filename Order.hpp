#pragma once
#include "Plant.hpp"
#include "Button.hpp"

struct Inventory;

/* Contains info on the order*/
struct OrderType
{
	OrderType(std::string title_in = "Default Order Title",
			std::string description_in = "Default Order Description",
			const std::map<PlantType const*, int> require_plants_in = {},
			int bonus_cash_in = 100,
			PlantType const* bonus_plant_in = test_plant):
		title(title_in),
		description(description_in),
		require_plants(require_plants_in),
		bonus_cash(bonus_cash_in),
		bonus_plant(bonus_plant_in){};

	std::string get_title() const { return title; };
	std::string get_description() const { return description; };
	std::map<PlantType const*, int> get_required_plants() const {return require_plants;};
	int get_bonus_cash() const { return bonus_cash; };
	PlantType const* get_bonus_plant() const {return bonus_plant;};

	void draw(glm::uvec2 const &drawable_size, Inventory& inventory ) const;
	glm::vec2 view_min = glm::vec2(0,0);
	glm::vec2 view_max = glm::vec2(1000, 800);
	glm::vec2 text_min = glm::vec2(0,0);
	glm::vec2 text_max = glm::vec2(250, 220);

private:
	std::string title = "Default Order Title";
	std::string description = "Default Order Description";
	std::map<PlantType const*, int> require_plants = {};
	int bonus_cash = 100;
	PlantType const* bonus_plant = test_plant;
	glm::u8vec4 text_col = glm::u8vec4(92, 76, 53, 255);
};

extern OrderType const* order1;
extern OrderType const* order2;
extern OrderType const* order3;
extern OrderType const* order4;
extern OrderType const* order5;
extern OrderType const* order6;
extern OrderType const* order7;
