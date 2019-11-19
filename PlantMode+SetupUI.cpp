#include "PlantMode.hpp"
#include "data_path.hpp"

Load< Sound::Sample > shop_open_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "ShopOpen.wav" ) );
 } );

Load< Sound::Sample > shop_close_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "ShopClose.wav" ) );
} );

struct {
	struct {
		Sprite const* background = nullptr;
		Sprite const* hand = nullptr;
		Sprite const* watering_can = nullptr;
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
	struct {
		Sprite const* icon = nullptr;
		Sprite const* background = nullptr;
	} magicbook;
	Sprite const* close = nullptr;
	Sprite const* coins = nullptr;
} sprites;

Load< void > more_ui_sprites(LoadTagDefault, []() {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("solidarity"));
	// tools
	sprites.tools.background = &ret->lookup("toolsBackground");
	sprites.tools.hand = &ret->lookup("hand");
	sprites.tools.watering_can = &ret->lookup("wateringCan");
	sprites.tools.shovel = &ret->lookup("shovel");
	sprites.tools.fertilizer = &ret->lookup("fertilizer");
	// storage
	sprites.storage.icon = &ret->lookup("seedBagClosed");
	sprites.storage.background = &ret->lookup("seedMenuBackground");
	sprites.storage.seeds_tab = &ret->lookup("seedBagOpen");
	sprites.storage.harvest_tab = &ret->lookup("harvestBasket");
	// magicbook
	sprites.magicbook.icon = &ret->lookup("magicbookIcon");
	sprites.magicbook.background = &ret->lookup("magicbookBackground");
	// other
	sprites.coins = &ret->lookup("coins");
	sprites.close = &ret->lookup("magicbookClose");
});

void PlantMode::setup_UI() {
	// root
	UI.root = new UIElem(
		nullptr, // parent
		glm::vec2(0, 0), // anchor
		glm::vec2(200, 100), // position
		glm::vec2(200, 150), // size
		nullptr, // sprite
		"", // text
		glm::vec2(0,0), // sprite pos
		0.0f); // sprite scale

	//---------------- money ------------------
	UIElem* money_icon = new UIElem(
		UI.root,
		glm::vec2(1, 0), // anchor
		glm::vec2(-160, 20), // pos
		glm::vec2(0, 0), // size
		sprites.coins, "coins",
		glm::vec2(0, 0), // sprite pos
		0.4f);
	UI.coins_text = new UIElem(money_icon);
	UI.coins_text->set_scale(0.68f);
	UI.coins_text->set_text("hoofda");
	UI.coins_text->set_position(glm::vec2(76, 6), glm::vec2(0, 0), screen_size);

	//---------------- toolbar ------------------

	// toolbar background
	UIElem* toolbar_bg = new UIElem(
		UI.root, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(0, 0), // position
		glm::vec2(0, 0), // size
		sprites.tools.background, // sprite
		"tools background", // text
		glm::vec2(-270,0), // sprite pos
		0.4f); // sprite scale

	// default hand
	UI.toolbar.glove = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(60, -84), // position
		glm::vec2(64, 64), // size
		sprites.tools.hand, // sprite
		"glove", // text
		glm::vec2(0, 0), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.glove->set_on_mouse_enter([this](){
		for (auto c : UI.toolbar.glove->children) c->show();
	});
	UI.toolbar.glove->set_on_mouse_leave([this](){
		for (auto c : UI.toolbar.glove->children) c->hide();
	});
	UI.toolbar.glove->set_on_mouse_down([this](){
		set_current_tool( default_hand );
	});
	new UIElem(
		UI.toolbar.glove,
		glm::vec2(0, 0), // anchor
		glm::vec2(0, 48), // pos
		glm::vec2(0, 0),
		nullptr,
		"glove",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	// watering can
	UI.toolbar.watering_can = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(142, -79), // position
		glm::vec2(64, 64), // size
		sprites.tools.watering_can, // sprite
		"watering can", // text
		glm::vec2(32, 32), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.watering_can->set_on_mouse_enter([this](){
		for (auto c : UI.toolbar.watering_can->children) c->show();
	});
	UI.toolbar.watering_can->set_on_mouse_leave([this](){
		for (auto c : UI.toolbar.watering_can->children) c->hide();
	});
	UI.toolbar.watering_can->set_on_mouse_down([this](){
		if( current_tool == watering_can ) set_current_tool( default_hand );
		else set_current_tool( watering_can );
	});
	new UIElem(
		UI.toolbar.watering_can,
		glm::vec2(0, 0), // anchor
		glm::vec2(-4, 44), // pos
		glm::vec2(0, 0),
		nullptr,
		"watering can",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	// fertilizer
	UI.toolbar.fertilizer = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(230, -94), // position
		glm::vec2(64, 64), // size
		sprites.tools.fertilizer, // sprite
		"fertilizer", // text
		glm::vec2(32, 32), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.fertilizer->set_on_mouse_enter([this](){
		for (auto c : UI.toolbar.fertilizer->children) c->show();
	});
	UI.toolbar.fertilizer->set_on_mouse_leave([this](){
		for (auto c : UI.toolbar.fertilizer->children) c->hide();
	});
	UI.toolbar.fertilizer->set_on_mouse_down([this](){
		if( current_tool == fertilizer ) set_current_tool( default_hand );
		else set_current_tool( fertilizer );
	});
	new UIElem(
		UI.toolbar.fertilizer,
		glm::vec2(0, 0), // anchor
		glm::vec2(0, 65), // pos
		glm::vec2(0, 0),
		nullptr,
		"fertilizer",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	// shovel
	UI.toolbar.shovel = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(315, -87), // position
		glm::vec2(64, 64), // size
		sprites.tools.shovel, // sprite
		"shovel", // text
		glm::vec2(32, 32), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.shovel->set_on_mouse_enter([this](){
		for (auto c : UI.toolbar.shovel->children) c->show();
	});
	UI.toolbar.shovel->set_on_mouse_leave([this](){
		for (auto c : UI.toolbar.shovel->children) c->hide();
	});
	UI.toolbar.shovel->set_on_mouse_down([this](){
		if( current_tool == shovel ) set_current_tool( default_hand );
		else set_current_tool( shovel );
	});
	new UIElem(
		UI.toolbar.shovel,
		glm::vec2(0, 0), // anchor
		glm::vec2(0, 46), // pos
		glm::vec2(0, 0),
		nullptr,
		"shovel",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden
	
	//---------------- storage ------------------
	UIElem* storage_bg = nullptr;
	UIElem* storage_icon = nullptr;
	UIElem* storage_close = nullptr;
	UIElem* seed_tab = nullptr;
	UIElem* harvest_tab = nullptr;
	UIElem* seed_tab_items = nullptr;
	UIElem* harvest_tab_items = nullptr;

	storage_bg = new UIElem(
		UI.root,
		glm::vec2(1, 1), // anchor
		glm::vec2(-565, 306), // position
		glm::vec2(0, 0), // size
		sprites.storage.background,
		"storage background",
		glm::vec2(0, 0), // sprite anchor
		0.5f);
	storage_bg->set_z_index(2);

	storage_close = new UIElem(
		storage_bg,
		glm::vec2(0, 0),
		glm::vec2(375, -350),
		glm::vec2(40, 40),
		sprites.close, "close storage",
		glm::vec2(20, 20),
		0.3f, true, true, false);

	storage_icon = new UIElem(
		UI.root,
		glm::vec2(1, 1), // anchor
		glm::vec2(-190, -80), // position
		glm::vec2(64, 64), //size
		sprites.storage.icon, // sprite
		"storage icon",
		glm::vec2(32, 32),
		0.3f, true, false, false);
	storage_icon->set_z_index(3);
	UIElem* storage_icon_text = new UIElem(
		storage_icon,
		glm::vec2(0, 0),
		glm::vec2(0, -20),
		glm::vec2(0, 0),
		nullptr, "storage",
		glm::vec2(0, 0),
		0.4f, false, true);

	seed_tab = new UIElem(
		storage_bg,
		glm::vec2(0, 0), // anchor
		glm::vec2(215, -374), // pos
		glm::vec2(64, 64),
		sprites.storage.seeds_tab,
		"seeds tab",
		glm::vec2(32, 32),
		0.6f, true, true, false);
	UIElem* seeds_tab_text = new UIElem(
		seed_tab,
		glm::vec2(0, 0), // anchor
		glm::vec2(5, 36), // pos
		glm::vec2(0, 0),
		nullptr,
		"seeds",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	// a dummy node just so that all its children are items to be laid out.
	seed_tab_items = new UIElem(storage_bg);

	harvest_tab = new UIElem(
		storage_bg,
		glm::vec2(0, 0), // anchor
		glm::vec2(295, -374), // pos
		glm::vec2(64, 64),
		sprites.storage.harvest_tab,
		"harvest tab",
		glm::vec2(32, 32),
		0.5f, true, true, false);
	harvest_tab->set_z_index(-1);
	UIElem* harvest_tab_text = new UIElem(
		harvest_tab,
		glm::vec2(0, 0), // anchor
		glm::vec2(12, 40), // pos
		glm::vec2(0, 0),
		nullptr,
		"harvest",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	harvest_tab_items = new UIElem(storage_bg);
	harvest_tab_items->hide();

	//---- set events ----

	storage_icon->set_on_mouse_enter([storage_icon_text](){
		Sound::play( *button_hover_sound, 0.0f, 1.0f );
		storage_icon_text->show();
	});
	storage_icon->set_on_mouse_leave([storage_icon_text](){
		storage_icon_text->hide();
	});
	storage_icon->set_on_mouse_down([this, storage_icon, storage_close, seed_tab, harvest_tab, storage_bg](){
		Sound::play( *shop_open_sound, 0.0f, 1.0f );
		storage_icon->hide();
		storage_close->show();
		seed_tab->show();
		harvest_tab->show();
		storage_bg->set_position(
				storage_bg->get_position() + glm::vec2(0, -290),
				storage_bg->get_anchor(),
				screen_size);
	});

	storage_close->set_on_mouse_down([this, seed_tab, harvest_tab, storage_icon, storage_close, storage_bg](){
		Sound::play( *shop_close_sound, 0.0f, 1.0f );
		seed_tab->hide();
		harvest_tab->hide();
		storage_close->hide();
		storage_icon->show();
		storage_bg->set_position(
				storage_bg->get_position() + glm::vec2(0, 290),
				storage_bg->get_anchor(),
				screen_size);
	});

	seed_tab->set_on_mouse_enter([seeds_tab_text](){
		Sound::play( *button_hover_sound, 0.0f, 1.0f );
		seeds_tab_text->show();
	});
	seed_tab->set_on_mouse_leave([seeds_tab_text](){
		seeds_tab_text->hide();
	});
	seed_tab->set_on_mouse_down([this, seed_tab, seed_tab_items, harvest_tab, harvest_tab_items, storage_icon, storage_close, storage_bg](){
		if (UI.storage_current_tab == 0) {
			Sound::play( *shop_close_sound, 0.0f, 1.0f );
			seed_tab->hide();
			harvest_tab->hide();
			storage_icon->show();
			storage_close->hide();
			storage_bg->set_position(
					storage_bg->get_position() + glm::vec2(0, 290),
					storage_bg->get_anchor(),
					screen_size);
		} else {
			UI.storage_current_tab = 0;
			seed_tab->set_z_index(0);
			harvest_tab->set_z_index(-1);
			seed_tab_items->show();
			harvest_tab_items->hide();
		}
	});

	seed_tab_items->set_layout_children_fn([this, seed_tab_items](){
		auto children = seed_tab_items->children;
		auto children_to_show = std::vector<UIElem*>();
		for (auto c : children) {
			if (c->get_text().size() > 0) {
				c->show();
				children_to_show.push_back(c);
			} else {
				c->hide();
			}
		}
		for (int i=0; i<children_to_show.size(); i++) {
			int row = i / 4;
			int col = i % 4;
			children_to_show[i]->set_position(
					glm::vec2(40, -303) + glm::vec2(col * 93.5f, row * 89), glm::vec2(0, 0), screen_size);
		}
	});
	
	harvest_tab->set_on_mouse_enter([harvest_tab_text](){
		Sound::play( *button_hover_sound, 0.0f, 1.0f );
		harvest_tab_text->show();
	});
	harvest_tab->set_on_mouse_leave([harvest_tab_text](){
		harvest_tab_text->hide();
	});
	harvest_tab->set_on_mouse_down([this, seed_tab, seed_tab_items, harvest_tab, harvest_tab_items, storage_icon, storage_close, storage_bg](){
		if (UI.storage_current_tab == 1) {
			Sound::play( *shop_close_sound, 0.0f, 1.0f );
			seed_tab->hide();
			harvest_tab->hide();
			storage_icon->show();
			storage_close->hide();
			storage_bg->set_position(
					storage_bg->get_position() + glm::vec2(0, 290),
					storage_bg->get_anchor(),
					screen_size);
		} else {
			UI.storage_current_tab = 1;
			seed_tab->set_z_index(-1);
			harvest_tab->set_z_index(0);
			seed_tab_items->hide();
			harvest_tab_items->show();
		}
	});

	harvest_tab_items->set_layout_children_fn([this, harvest_tab_items](){
		auto children = harvest_tab_items->children;
		auto children_to_show = std::vector<UIElem*>();
		for (auto c : children) {
			if (c->get_text().size() > 0) {
				c->show();
				children_to_show.push_back(c);
			} else {
				c->hide();
			}
		}
		for (int i=0; i<children_to_show.size(); i++) {
			int row = i / 4;
			int col = i % 4;
			children_to_show[i]->set_position(
					glm::vec2(40, -303) + glm::vec2(col * 93.5f, row * 89), glm::vec2(0, 0), screen_size);
		}
	});

	auto add_plant_buttons = [this, seed_tab_items, harvest_tab_items](PlantType const* plant) {
		UIElem* seed_icon = nullptr;
		UIElem* harvest_icon = nullptr;
		plant->make_menu_items(screen_size, &selectedPlant, &current_tool, &seed_icon, &harvest_icon);
		assert(seed_icon); assert(harvest_icon);
		seed_icon->set_on_mouse_enter([this, plant](){
			tool_name = plant->get_name() + " x" + std::to_string( inventory.get_seeds_num( plant ) ) + " :";
			tool_description = plant->get_description();
		});
		seed_icon->set_on_mouse_leave([this](){
			set_current_tool(current_tool);
		});

		// the small number on top left of seed / harvest
		UIElem* seed = new UIElem( // will get automatically laid out anyway
			seed_tab_items,
			glm::vec2(0, 0), // anchor
			glm::vec2(0, 0), // pos
			glm::vec2(0, 0), // size
			nullptr, "hi", glm::vec2(0, 0), 0.35f);
		UIElem* harvest = new UIElem( // will get automatically laid out anyway
			harvest_tab_items,
			glm::vec2(0, 0), // anchor
			glm::vec2(0, 0), // pos
			glm::vec2(0, 0), // size
			nullptr, "ho", glm::vec2(0, 0), 0.35f);
		seed_icon->set_parent(seed);
		harvest_icon->set_parent(harvest);
		inventory.set_seed_item(plant, seed);
		inventory.set_harvest_item(plant, harvest);
	};
	add_plant_buttons( test_plant );
	add_plant_buttons( friend_plant );
	add_plant_buttons( vampire_plant );
	add_plant_buttons( cactus_plant );
	add_plant_buttons( fireflower_plant );
	add_plant_buttons( corpseeater_plant );
	
	//---------------- magic book ------------------
	UIElem* magicbook_icon = new UIElem(
		UI.root,
		glm::vec2(1, 1), // anchor
		glm::vec2(-110, -80), // pos
		glm::vec2(64, 64), // size
		sprites.magicbook.icon,
		"magic book icon",
		glm::vec2(32, 32),
		0.3f, true);
	UIElem* magicbook_text = new UIElem(
		magicbook_icon,
		glm::vec2(0, 0), // anchor
		glm::vec2(0, -20), // pos
		glm::vec2(0, 0),// size
		nullptr, "magic book",
		glm::vec2(0, 0), // sprite pos
		0.4f, false, true);
	magicbook_icon->set_on_mouse_enter([magicbook_text](){
		magicbook_text->show();
	});
	magicbook_icon->set_on_mouse_leave([magicbook_text](){
		magicbook_text->hide();
	});

	UIElem* magicbook_bg = new UIElem(
		UI.root,
		glm::vec2(0.5f, 0.5f), // anchor
		glm::vec2(-459, -310), // pos
		glm::vec2(0, 0), // size
		sprites.magicbook.background, "magicbook background",
		glm::vec2(0, 0),
		0.9f, false, true);
	magicbook_icon->set_on_mouse_down([magicbook_bg](){
		if (magicbook_bg->get_hidden()) { magicbook_bg->show(); }
		else magicbook_bg->hide();
	});

	UIElem* magicbook_close_btn = new UIElem(
		magicbook_bg,
		glm::vec2(0, 0), // anchor
		glm::vec2(830, 30), // pos
		glm::vec2(40, 40), // size
		sprites.close, "close magicbook",
		glm::vec2(20, 20),
		0.35f, true, false, false);
	magicbook_close_btn->set_on_mouse_down([magicbook_bg](){
		magicbook_bg->hide();
	});

	UIElem* all_choices = new UIElem(magicbook_bg);
	all_choices->set_layout_children_fn([this, all_choices](){
		for (int i=0; i<all_choices->children.size(); i++) {
			int page = i / 4;
			int row = i % 4;
			glm::vec2 pos = glm::vec2(95, 70) + glm::vec2(page * 425, row * 60);
			all_choices->children[i]->set_position(pos, glm::vec2(0, 0), screen_size);
		}	
	});
	
	// magicbook buy choices
	auto add_buy_choice = [this, all_choices]( PlantType const* plant ) {
		std::string text = plant->get_name() + " seed - $" + std::to_string( plant->get_cost() );
		UIElem* entry = new UIElem( // will get laid out automatically anyway.
			all_choices,
			glm::vec2(0, 0), // anchor
			glm::vec2(0, 0), // pos
			glm::vec2(270, 32), // size
			nullptr, text,
			glm::vec2(6.0f, 1.0f), // text anchor
			0.54f, true);
		entry->set_on_mouse_down([this, plant](){
			if( num_coins >= plant->get_cost() ){
			change_num_coins( -plant->get_cost() );
				inventory.change_seeds_num( plant, 1 );
			}
		});
		entry->set_tint(glm::u8vec4(92, 76, 53, 255));
		entry->set_on_mouse_enter([entry](){
			entry->set_tint(glm::u8vec4(145, 127, 100, 255));
		});
		entry->set_on_mouse_leave([entry](){
			entry->set_tint(glm::u8vec4(92, 76, 53, 255));
		});
	};
	add_buy_choice( test_plant );
	add_buy_choice( friend_plant );
	add_buy_choice( cactus_plant );
	add_buy_choice( vampire_plant );
	add_buy_choice( fireflower_plant );
	add_buy_choice( corpseeater_plant );

	all_choices->layout_children();

}
