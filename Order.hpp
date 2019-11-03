#include "Plant.hpp"
#include "PlantMode.hpp"

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
    std::map<PlantType const*, int> get_require_plants() const {return require_plants;};
    int get_bonus_cash() const { return bonus_cash; };
    PlantType const* get_bonus_plant() const {return bonus_plant;};



    
    private:
        std::string title = "Default Order Title";
        std::string description = "Default Order Description";
        std::map<PlantType const*, int> require_plants = {};
        int bonus_cash = 100;
        PlantType const* bonus_plant = test_plant;
};

extern PlantType const* test_plant;
extern PlantType const* friend_plant;
extern PlantType const* vampire_plant;
extern PlantType const* carrot_plant;
extern PlantType const* cactus_plant;
extern PlantType const* fireflower_plant;

OrderType const* order1;
OrderType const* order2;
extern OrderType const* current_order;

void draw();
glm::vec2 view_min = glm::vec2(0,0);
glm::vec2 view_max = glm::vec2(50, 44);