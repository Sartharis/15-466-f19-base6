#include "PlantMode.hpp"
#include "data_path.hpp"
#include <cstddef>

Load< Sound::Sample > shop_open_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "ShopOpen.wav" ) );
 } );

Load< Sound::Sample > shop_close_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "ShopClose.wav" ) );
} );

Load< Sound::Sample > magic_book_toggle_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "BOOK_Turn_Page_01_mono.wav" ) );
} );

Load< Sound::Sample > magic_book_flip_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "BOOK_Turn_Page_04_mono.wav" ) );
 } );

Load< Sound::Sample > magic_book_purchase_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "COINS_Rattle_01_mono.wav" ) );
} );

Load< Sound::Sample > order_completed_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "OrderComplete.wav" ) );
} );

struct {
	Sprite const* banner = nullptr;
	struct {
		Sprite const* background = nullptr;
		Sprite const* hand = nullptr;
		Sprite const* watering_can = nullptr;
		Sprite const* shovel = nullptr;
		Sprite const* healer = nullptr;
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
		Sprite const* lock = nullptr;
		Sprite const* background = nullptr;
		Sprite const* prev = nullptr;
		Sprite const* next = nullptr;
	} magicbook;
	struct {
		Sprite const* rolledup = nullptr;
		Sprite const* expanded = nullptr;
	} order;
	Sprite const* close = nullptr;
	Sprite const* coins = nullptr;
	Sprite const* instructions_icon = nullptr;
	Sprite const* win = nullptr;
	Sprite const* lose = nullptr;
} ui_sprites;

Load< void > more_ui_sprites(LoadTagDefault, []() {
	SpriteAtlas const *ret = new SpriteAtlas(data_path("solidarity"));
	// banner
	ui_sprites.banner = &ret->lookup("banner");
	// tools
	ui_sprites.tools.background = &ret->lookup("toolsBackground");
	ui_sprites.tools.hand = &ret->lookup("hand");
	ui_sprites.tools.watering_can = &ret->lookup("wateringCan");
	ui_sprites.tools.shovel = &ret->lookup("shovel");
	ui_sprites.tools.healer = &ret->lookup("healer");
	// storage
	ui_sprites.storage.icon = &ret->lookup("seedBagClosed");
	ui_sprites.storage.background = &ret->lookup("seedMenuBackground");
	ui_sprites.storage.seeds_tab = &ret->lookup("seedBagOpen");
	ui_sprites.storage.harvest_tab = &ret->lookup("harvestBasket");
	// magicbook
	ui_sprites.magicbook.icon = &ret->lookup("magicbookIcon");
	ui_sprites.magicbook.background = &ret->lookup("magicbookBackground");
	ui_sprites.magicbook.prev = &ret->lookup("pagePrev");
	ui_sprites.magicbook.next = &ret->lookup("pageNext");
	ui_sprites.magicbook.lock = &ret->lookup( "plantLocked" );
	// order
	ui_sprites.order.rolledup = &ret->lookup("orderRolledup");
	ui_sprites.order.expanded = &ret->lookup("orderExpanded");
	// other
	ui_sprites.coins = &ret->lookup("coins");
	ui_sprites.close = &ret->lookup("magicbookClose");
	ui_sprites.instructions_icon = &ret->lookup("helpIcon");
	ui_sprites.win = &ret->lookup( "youWin" );
	ui_sprites.lose = &ret->lookup( "youLose" );
});

void PlantMode::setup_UI() {
	// root
	UI.root = new UIElem(nullptr);
	UI.root_pause = new UIElem( nullptr );
	UI.root_title = new UIElem( nullptr );

	//--------------- win / lose ----------------------
	UI.lose_screen = new UIElem(
		UI.root,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -400, -200 ), // pos
		glm::vec2( 0, 0 ), // size
		ui_sprites.lose, "You ran out of money. Pause and press R to restart.",
		glm::vec2( 0, 0 ), // sprite pos
		1.0f, false, true );

	UIElem* lose_text = new UIElem(
		UI.lose_screen,
		glm::vec2( 0, 0 ), // anchor
		glm::vec2( -70.0f, 400.0f ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "You ran out of money. Pause and press R to restart.",
		glm::vec2( 0, 0 ), // sprite pos
		0.8f, false, false);
	(void)lose_text;

	UI.win_screen = new UIElem(
		UI.root,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -400, -200 ), // pos
		glm::vec2( 0, 0 ), // size
		ui_sprites.win, "You fulfilled all of the orders! Congratulations!",
		glm::vec2( 0, 0 ), // sprite pos
		1.0f, false, true );

	UIElem* win_text = new UIElem(
		UI.win_screen,
		glm::vec2( 0, 0 ), // anchor
		glm::vec2( -20.0f, 400.0f ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "You fulfilled all of the orders! Congratulations!",
		glm::vec2( 0, 0 ), // sprite pos
		0.8f, false, false);
	(void)win_text;


	//----------------- instructions  -----------------
	UIElem* instructions_button = new UIElem(
		UI.root,
		glm::vec2( 0.0f, 0.0f ), // anchor
		glm::vec2( 20, 20 ), // pos
		glm::vec2( 40, 40 ), // size
		ui_sprites.instructions_icon, "open help",
		glm::vec2( 0, 0 ), // sprite pos
		0.2f,true );

	instructions_button->set_on_mouse_down( [this](){
		paused = true;
	} );

	UIElem* close_instructions_button = new UIElem(
		UI.root_pause,
		glm::vec2( 0.0f, 0.0f ), // anchor
		glm::vec2( 20, 20 ), // pos
		glm::vec2( 40, 40 ), // size
		ui_sprites.instructions_icon, "close help",
		glm::vec2( 0, 0 ), // sprite pos
		0.2f, true );

	close_instructions_button->set_on_mouse_down( [this](){
		paused = false;
	} );

	//---------------- pause ------------------
	UIElem* pause_text = new UIElem(
		UI.root_pause,
		glm::vec2( 0.5f, 0.5f), // anchor
		glm::vec2( -150, -300 ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "PAUSED",
		glm::vec2( 0, 0 ), // sprite pos
		1.5f );
	(void)pause_text;

	UIElem* how_to_play_text = new UIElem(
		UI.root_pause,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -300, -200 ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "HOW TO PLAY: Your goal is to unlock all of the plants. To do this, fulfill the orders on the right side of the screen by harvesting different plants.",
		glm::vec2( 0, 0 ), // sprite pos
		0.5f );
	how_to_play_text->set_max_text_width( 600.0f );

	UIElem* controls_text = new UIElem(
		UI.root_pause,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -300, -100.0f ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "Move camera with WASD and interact with the left mouse button. Press f for fullscreen.",
		glm::vec2( 0, 0 ), // sprite pos
		0.5f );
	controls_text->set_max_text_width( 600.0f );

	UIElem* seed_buy_text = new UIElem(
		UI.root_pause,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -300, 0.0f ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "To buy seeds for planting, use the magic book in the bottom right corner. Some seeds are locked until you unlock them by fulfilling orders.",
		glm::vec2( 0, 0 ), // sprite pos
		0.5f );
	seed_buy_text->set_max_text_width( 600.0f );

	UIElem* seed_use_text = new UIElem(
		UI.root_pause,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -300, 100 ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "Your bought seeds will be available in the storage tab (left of the magic book in bottom right corner).",
		glm::vec2( 0, 0 ), // sprite pos
		0.5f );
	seed_use_text->set_max_text_width( 600.0f );

    //---------------- title ------------------
	UIElem* title_text = new UIElem(
		UI.root_title,
		glm::vec2( 0.0f, 0.0), // anchor
		glm::vec2( 0, 0 ), // pos
		glm::vec2( 0, 0 ), // size
		ui_sprites.banner, "HARVEST ISLAND",
		glm::vec2( 0, 0 ), // sprite pos
		1.0f );
	(void)title_text;

	// start button
	UIElem* start = new UIElem(
		UI.root_title,
		glm::vec2( 0.5f, 0.5f), // anchor
		glm::vec2( -150, 150 ), // pos
		glm::vec2( 300, 100 ), // size
		nullptr, "START GAME",
		glm::vec2( 0, 0 ), // sprite pos
		1.0f, true );
		start->make_interactive();
		start->set_on_mouse_down( [this](){
			paused = false;
			title = false;
		} );
	start->set_max_text_width( 600.0f );

	UIElem* credit_text = new UIElem(
		UI.root_title,
		glm::vec2( 0.5f, 0.5f ), // anchor
		glm::vec2( -300, 50 ), // pos
		glm::vec2( 0, 0 ), // size
		nullptr, "A game by: Rain Du, Lexi Luo, Jan Orlowski, Margot Stewart",
		glm::vec2( 0, 0 ), // sprite pos
		0.5f );
	credit_text->set_max_text_width( 600.0f );

	//---------------- money ------------------
	UIElem* money_icon = new UIElem(
		UI.root,
		glm::vec2(1, 0), // anchor
		glm::vec2(-160, 20), // pos
		glm::vec2(0, 0), // size
		ui_sprites.coins, "coins",
		glm::vec2(0, 0), // sprite pos
		0.4f);
	UI.coins_text = new UIElem(money_icon);
	UI.coins_text->set_scale(0.68f);
	UI.coins_text->set_text("");
	UI.coins_text->set_position(glm::vec2(76, 6), glm::vec2(0, 0));

	//---------------- toolbar ------------------

	// toolbar background
	UIElem* toolbar_bg = new UIElem(
		UI.root, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(0, 0), // position
		glm::vec2(0, 0), // size
		ui_sprites.tools.background, // sprite
		"tools background", // text
		glm::vec2(-270,0), // sprite pos
		0.4f); // sprite scale

	// default hand
	UI.toolbar.glove = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(60, -84), // position
		glm::vec2(64, 64), // size
		ui_sprites.tools.hand, // sprite
		"glove", // text
		glm::vec2(0, 0), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.glove->set_on_mouse_enter( [this](){
		for( auto c : UI.toolbar.glove->children ) c->show();
		set_current_tool_tooltip( default_hand );
	 } );
	UI.toolbar.glove->set_on_mouse_leave( [this](){
		for( auto c : UI.toolbar.glove->children ) c->hide();
		set_current_tool_tooltip( current_tool );
	} );
	UI.toolbar.glove->set_on_mouse_down([this](){
		set_current_tool( default_hand );
	});
	UI.toolbar.glove->set_hotkey(SDLK_1);
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
		ui_sprites.tools.watering_can, // sprite
		"watering can", // text
		glm::vec2(32, 32), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.watering_can->set_on_mouse_enter( [this](){
		for( auto c : UI.toolbar.watering_can->children ) c->show();
		set_current_tool_tooltip( watering_can );
	 } );
	UI.toolbar.watering_can->set_on_mouse_leave( [this](){
		for( auto c : UI.toolbar.watering_can->children ) c->hide();
		set_current_tool_tooltip( current_tool );
	} );
	UI.toolbar.watering_can->set_on_mouse_down([this](){
		if( current_tool == watering_can ) set_current_tool( default_hand );
		else set_current_tool( watering_can );
	});
	UI.toolbar.watering_can->set_hotkey(SDLK_2);
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
		ui_sprites.tools.healer, // sprite
		"healer", // text
		glm::vec2(7, 20), // sprite pos
		0.34f, // sprite scale
		true); 
	UI.toolbar.fertilizer->set_on_mouse_enter( [this](){
		for( auto c : UI.toolbar.fertilizer->children ) c->show();
		set_current_tool_tooltip( fertilizer );
	} );
	UI.toolbar.fertilizer->set_on_mouse_leave( [this](){
		for( auto c : UI.toolbar.fertilizer->children ) c->hide();
		set_current_tool_tooltip( current_tool );
	} );
	UI.toolbar.fertilizer->set_on_mouse_down([this](){
		if( current_tool == fertilizer ) set_current_tool( default_hand );
		else set_current_tool( fertilizer );
	});
	UI.toolbar.fertilizer->set_hotkey(SDLK_3);
	new UIElem(
		UI.toolbar.fertilizer,
		glm::vec2(0, 0), // anchor
		glm::vec2(12, 65), // pos
		glm::vec2(0, 0),
		nullptr,
		"healer",
		glm::vec2(0, 0),
		0.4f, false, true); // ..interactive, hidden

	// shovel
	UI.toolbar.shovel = new UIElem(
		toolbar_bg, // parent
		glm::vec2(0, 1), // anchor
		glm::vec2(315, -87), // position
		glm::vec2(64, 64), // size
		ui_sprites.tools.shovel, // sprite
		"shovel", // text
		glm::vec2(32, 32), // sprite pos
		0.3f, // sprite scale
		true); 
	UI.toolbar.shovel->set_on_mouse_enter([this](){
		for (auto c : UI.toolbar.shovel->children) c->show();
		set_current_tool_tooltip( shovel );
	});
	UI.toolbar.shovel->set_on_mouse_leave([this](){
		for (auto c : UI.toolbar.shovel->children) c->hide();
		set_current_tool_tooltip( current_tool );
	});
	UI.toolbar.shovel->set_on_mouse_down([this](){
		if( current_tool == shovel ) set_current_tool( default_hand );
		else set_current_tool( shovel );
	});
	UI.toolbar.shovel->set_hotkey(SDLK_4);
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
		glm::vec2(-565, -44), // position
		glm::vec2(416, 380), // size
		ui_sprites.storage.background,
		"storage background",
		glm::vec2(0, 0), // sprite anchor
		0.5f, true);
	storage_bg->set_z_index(2);

	storage_close = new UIElem(
		storage_bg,
		glm::vec2(0, 0),
		glm::vec2(375, 0),
		glm::vec2(40, 40),
		ui_sprites.close, "close storage",
		glm::vec2(20, 20),
		0.3f, true, true, false);

	storage_icon = new UIElem(
		storage_bg,
		glm::vec2(1, 0), // anchor
		glm::vec2(-50, -30), // position
		glm::vec2(64, 64), //size
		ui_sprites.storage.icon, // sprite
		"storage icon",
		glm::vec2(32, 32),
		0.3f, true, false, false);
	// storage_icon->set_z_index(3);
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
		glm::vec2(215, -24), // pos
		glm::vec2(64, 64),
		ui_sprites.storage.seeds_tab,
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

	UIElem* seed_name_text = new UIElem(
		seed_tab,
		glm::vec2( 0, 0 ), // anchor
		glm::vec2( -200, -60 ), // pos
		glm::vec2( 0, 0 ),
		nullptr,
		"",
		glm::vec2( 0, 0 ),
		0.8f, false, false );

	UIElem* seed_description_text = new UIElem(
		seed_tab,
		glm::vec2( 0, 0 ), // anchor
		glm::vec2( -200, -20 ), // pos
		glm::vec2( 0, 0 ),
		nullptr,
		"",
		glm::vec2( 0, 0 ),
		0.4f, false, false );

	// a dummy node just so that all its children are items to be laid out.
	seed_tab_items = new UIElem(storage_bg);

	harvest_tab = new UIElem(
		storage_bg,
		glm::vec2(0, 0), // anchor
		glm::vec2(295, -24), // pos
		glm::vec2(64, 64),
		ui_sprites.storage.harvest_tab,
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
	storage_icon->set_on_mouse_down([storage_icon, storage_close, seed_tab, harvest_tab, storage_bg](){
		Sound::play( *shop_open_sound, 0.0f, 1.0f );
		storage_icon->hide();
		storage_close->show();
		seed_tab->show();
		harvest_tab->show();
		storage_bg->set_position(
				storage_bg->get_position() + glm::vec2(0, -290),
				storage_bg->get_anchor(), 0.2f);
	});

	storage_close->set_on_mouse_down([seed_tab, harvest_tab, storage_icon, storage_close, storage_bg](){
		Sound::play( *shop_close_sound, 0.0f, 1.0f );
		seed_tab->hide();
		harvest_tab->hide();
		storage_close->hide();
		storage_icon->show();
		storage_bg->set_position(
				storage_bg->get_position() + glm::vec2(0, 290),
				storage_bg->get_anchor(), 0.2f);
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
					storage_bg->get_anchor(), 0.2f);
		} else {
			UI.storage_current_tab = 0;
			seed_tab->set_z_index(0);
			harvest_tab->set_z_index(-1);
			seed_tab_items->show();
			harvest_tab_items->hide();
		}
	});

	seed_tab_items->set_layout_children_fn([seed_tab_items](){
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
					glm::vec2(40, 47) + glm::vec2(col * 93.5f, row * 89), glm::vec2(0, 0));
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
					storage_bg->get_anchor(), 0.2f);
		} else {
			UI.storage_current_tab = 1;
			seed_tab->set_z_index(-1);
			harvest_tab->set_z_index(0);
			seed_tab_items->hide();
			harvest_tab_items->show();
		}
	});

	harvest_tab_items->set_layout_children_fn([harvest_tab_items](){
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
					glm::vec2(40, 47) + glm::vec2(col * 93.5f, row * 89), glm::vec2(0, 0));
		}
	});

	auto add_plant_buttons = [this, seed_tab_items, harvest_tab_items, seed_name_text, seed_description_text](PlantType const* plant) {
		UIElem* seed_icon = nullptr;
		UIElem* harvest_icon = nullptr;
		plant->make_menu_items(&selectedPlant, &current_tool, &seed_icon, &harvest_icon);
		assert(seed_icon); assert(harvest_icon);
		seed_icon->set_on_mouse_enter([plant, seed_name_text, seed_description_text](){
			seed_name_text->set_text( plant->get_name() + " :" );
			seed_description_text->set_text( plant->get_description() );
		});
		seed_icon->set_on_mouse_leave([this, seed_name_text, seed_description_text](){
			if( current_tool == seed )
			{
				seed_name_text->set_text( selectedPlant->get_name() + " :" );
				seed_description_text->set_text( selectedPlant->get_description() );
			}
			else
			{
				seed_name_text->set_text( "" );
				seed_description_text->set_text( "" );
			}
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

	for( auto plant : all_plants )
	{
		add_plant_buttons( plant );
	}
	
	//---------------- magic book ------------------
	UIElem* magicbook_icon = new UIElem(
		UI.root,
		glm::vec2(1, 1), // anchor
		glm::vec2(-110, -80), // pos
		glm::vec2(64, 64), // size
		ui_sprites.magicbook.icon,
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
	magicbook_icon->set_hotkey(SDLK_TAB);

	UIElem* magicbook_bg = new UIElem(
		UI.root,
		glm::vec2(0.5f, 0.5f), // anchor
		glm::vec2(-459, -310), // pos
		glm::vec2(914, 628), // size
		ui_sprites.magicbook.background, "magicbook background",
		glm::vec2(0, 0),
		0.9f, true, true);
	magicbook_icon->set_on_mouse_down([magicbook_bg](){
		if (magicbook_bg->get_hidden()) 
		{ 
			Sound::play( *magic_book_toggle_sound, 0.0f, 1.0f ); 
			magicbook_bg->show(); 
		}
		else
		{
			Sound::play( *magic_book_toggle_sound, 0.0f, 1.0f );
			magicbook_bg->hide();
		}
	});

	UIElem* magicbook_close_btn = new UIElem(
		magicbook_bg,
		glm::vec2(0, 0), // anchor
		glm::vec2(830, 30), // pos
		glm::vec2(40, 40), // size
		ui_sprites.close, "close magicbook",
		glm::vec2(20, 20),
		0.35f, true, false, false);
	magicbook_close_btn->set_on_mouse_down([magicbook_bg](){
		Sound::play( *magic_book_toggle_sound, 0.0f, 1.0f );
		magicbook_bg->hide();
	});
	magicbook_close_btn->set_hotkey(SDLK_TAB);

	UIElem* all_choices = new UIElem(magicbook_bg);
	all_choices->set_layout_children_fn([this, all_choices](){
		for (int i=0; i<all_choices->children.size(); i++) {
			all_choices->children[i]->hide();
			int page = i / 2;
			int row = i % 2;
			if (page == UI.magicbook_page || page == UI.magicbook_page + 1) {
				all_choices->children[i]->show();
				page = page % 2;
				glm::vec2 pos = glm::vec2(135, 70) + glm::vec2(page * 400, row * 255);
				all_choices->children[i]->set_position(pos, glm::vec2(0, 0));
			}
		}	
	});

	UIElem* magicbook_prev_page = new UIElem(
		magicbook_bg,
		glm::vec2(0, 0), //anchor
		glm::vec2(78, 528), //pos
		glm::vec2(64, 64), //size
		ui_sprites.magicbook.prev, "magic book prev page",
		glm::vec2(2, 2),
		1.0f, true);
	magicbook_prev_page->set_on_mouse_down([this, all_choices](){
		if (UI.magicbook_page >= 2 ) {
			Sound::play( *magic_book_flip_sound, 0.0f, 1.0f );
			UI.magicbook_page -= 2;
			all_choices->layout_children();
		}
	});
	magicbook_prev_page->set_hotkey(SDLK_q);
	
	UIElem* magicbook_next_page = new UIElem(
		magicbook_bg,
		glm::vec2(0, 0), //anchor
		glm::vec2(820, 528), //pos
		glm::vec2(64, 64), //size
		ui_sprites.magicbook.next, "magic book next page",
		glm::vec2(2, 2),
		1.0f, true);
	magicbook_next_page->set_on_mouse_down([this, all_choices](){
		int max_pages = (int)std::ceil(all_choices->children.size() / 2.0f) - 1;
		if (UI.magicbook_page + 2 <= max_pages) {
			Sound::play( *magic_book_flip_sound, 0.0f, 1.0f );
			UI.magicbook_page += 2;
			all_choices->layout_children();
		}
	});
	magicbook_next_page->set_hotkey(SDLK_e);

	// magicbook buy choices
	auto add_buy_choice = [this, all_choices]( PlantType const* plant ) {
		UIElem* entry = new UIElem(all_choices); // will get automatically laid out anyway
		plant_to_magicbook_entry.insert( std::make_pair( plant, entry ) ); //used for unlocking plants

		UIElem* lock = new UIElem( entry ); 
		lock->set_sprite( ui_sprites.magicbook.lock );
		lock->set_scale( 0.5f );
		lock->set_position( glm::vec2( 0, 0 ), glm::vec2( 0, 0 ) );

		// icon
		UIElem* icon = new UIElem(entry);
		icon->set_sprite(plant->get_harvest_sprite());
		icon->set_scale(0.5f);
		icon->hide();

		// seed
		UIElem* seed = new UIElem(entry);
		seed->set_sprite(plant->get_seed_sprite());
		seed->set_scale(0.25f);
		seed->set_position(glm::vec2(0, 60), glm::vec2(0, 0));
		seed->hide();

		// price
		UIElem* price = new UIElem(entry);
		price->set_text("$" + std::to_string( plant->get_cost() ));
		price->set_scale(0.6f);
		price->set_position(glm::vec2(40, 0), glm::vec2(0, 0));
		price->set_tint(text_tint);
		price->hide();

		// name
		UIElem* name = new UIElem(entry);
		name->set_position(glm::vec2(40, 40), glm::vec2(0, 0));
		name->set_size(glm::vec2(300, 40));
		name->set_text(plant->get_name() + " Seed");
		name->set_tint(text_tint);
		name->set_scale(0.6f);
		name->hide();

		// buy button
		UIElem* buy = new UIElem( entry );
		buy->make_interactive();
		buy->set_position( glm::vec2( 100, 0 ), glm::vec2( 0, 0 ) );
		buy->set_size( glm::vec2( 55, 40 ) );
		buy->set_text( "BUY" );
		buy->set_tint( text_tint );
		buy->set_scale( 0.6f );
		buy->set_on_mouse_down( [this, plant](){
			if( num_coins >= plant->get_cost() ){
				Sound::play( *magic_book_purchase_sound, 0.0f, 1.0f );
				change_num_coins( -plant->get_cost() );
				inventory.change_seeds_num( plant, 1 );
			}
		} );
		buy->set_on_mouse_enter( [buy, this](){
			buy->set_tint( text_highlight_tint );
		} );
		buy->set_on_mouse_leave( [buy, this](){
			buy->set_tint( text_tint );
		} );
		buy->hide();

		// description
		UIElem* description = new UIElem(entry);
		description->set_text(plant->get_description());
		description->set_scale(0.45f);
		description->set_position(glm::vec2(0, 85), glm::vec2(0, 0));
		description->set_tint(text_tint);
		description->set_max_text_width(300.0f);
		description->hide();
	};

	for( auto plant : all_plants )
	{
		if( plant != spreader_child_plant )
		{
			add_buy_choice( plant );
		}
	}

	all_choices->layout_children();
	
	//---------------- orders ------------------
	
	// main order container
	UIElem* order1 = new UIElem(
		UI.root,
		glm::vec2(1, 0), // anchor
		glm::vec2(0, 90), // pos
		glm::vec2(0, 0), // size
		nullptr, "", glm::vec2(0, 0), 1.0f);

	// daily order container
	UIElem* order2 = new UIElem(
		UI.root,
		glm::vec2(1, 0), //anchor
		glm::vec2(0, 290), //pos
		glm::vec2(0, 0), //size
		nullptr, "", glm::vec2(0, 0), 1.0f);

	float order_w1 = 189.0f;
	float order_w2 = 394.0f;
	float order_h = 178.0f;

	// MAIN ORDER ---------
	
	UIElem* order1_rolledup = new UIElem(
		order1,
		glm::vec2(0, 0), //anchor
		glm::vec2(-order_w1, 0), //pos
		glm::vec2(order_w1, order_h), //size
		ui_sprites.order.rolledup, "",
		glm::vec2(order_w1, -14), //sprite pos
		0.45f, true, false, false);

	UIElem* order1_expanded = new UIElem(
		order1,
		glm::vec2(0, 0), //anchor
		glm::vec2(-order_w2, 0), //pos
		glm::vec2(order_w2, order_h), //size
		ui_sprites.order.expanded, "",
		glm::vec2(order_w2, -14), //sprite pos
		0.45f, true, true, false);

	order1_rolledup->set_on_mouse_enter([order1_rolledup, order1_expanded](){
		order1_rolledup->hide();
		order1_expanded->show();
	});
	order1_expanded->set_on_mouse_leave([order1_rolledup, order1_expanded](){
		order1_rolledup->show();
		order1_expanded->hide();
	});

	UI.main_order.description = new UIElem(
		order1_expanded,
		glm::vec2(0, 0), //anchor
		glm::vec2(20, 10), //pos
		glm::vec2(0, 0), //size doesn't matter
		nullptr, "default description",
		glm::vec2(0, 0), 0.4f);
	UI.main_order.description->set_tint(text_tint);
	UI.main_order.description->set_max_text_width(220.0f);

	UIElem* complete_btn = new UIElem(
		order1_expanded,
		glm::vec2(0, 1), //anchor
		glm::vec2(68, -38), //pos
		glm::vec2(82, 22), //size
		nullptr, "COMPLETE",
		glm::vec2(0, 0), 0.4f, true);
	complete_btn->set_tint(text_tint);
	complete_btn->set_on_mouse_enter([this, complete_btn](){ complete_btn->set_tint(text_highlight_tint); });
	complete_btn->set_on_mouse_leave([this, complete_btn](){ complete_btn->set_tint(text_tint); });
	complete_btn->set_on_mouse_down([this](){
		// std::cout << "Submit Main Order Button Click!" << std::endl;
		std::map< PlantType const*, int > require_plants = current_main_order->get_required_plants();
		bool orderFinished = true;
		std::map<PlantType const*, int>::iterator iter = require_plants.begin();
		while( iter != require_plants.end() ) {
			PlantType const* require_type = iter->first;
			int needed_num = iter->second;
			if( inventory.get_harvest_num(require_type) < needed_num ){
				orderFinished = false;
				break;
			}
			iter++;
		}
		std::cout << orderFinished << std::endl;
		if( orderFinished == true ){
			iter = require_plants.begin();
			while( iter != require_plants.end() ) {
				PlantType const* require_type = iter->first;
				int needed_num = iter->second;
				inventory.change_harvest_num(require_type, -needed_num);
				iter++;
			}

			if( current_main_order->get_bonus_plant() ) unlock_plant( current_main_order->get_bonus_plant() );

			Sound::play( *order_completed_sound, 0.0f, 1.0f );
			change_num_coins( current_main_order->get_bonus_cash() );
			current_main_order_idx += 1;
			if( current_main_order_idx >= main_orders.size() ){
				current_main_order_idx =  (int)main_orders.size()-1;
				UI.win_screen->show();
			}
			else
			{
				// std::cout << "main_order_idx " << current_main_order_idx << std::endl;
				set_main_order( current_main_order_idx );
			}
		}
	});

	UI.main_order.unlock_plant = new UIElem(
		order1,
		glm::vec2(1, 0), //anchor
		glm::vec2(-148, 18), //pos
		glm::vec2(0, 0), //size doesn't matter
		nullptr, "default unlock plant text",
		glm::vec2(0, 0), 0.44f);
	UI.main_order.unlock_plant->set_tint(text_tint);
	UI.main_order.unlock_plant->set_max_text_width(180.0f);

	UI.main_order.requirements = new UIElem(
		order1,
		glm::vec2(1, 0), //anchor
		glm::vec2(-142, 68), //pos
		glm::vec2(0, 0), //size
		nullptr, "",
		glm::vec2(0, 0), 1.0f);
	UI.main_order.requirements->set_layout_children_fn([this](){
		UIElem* reqs = UI.main_order.requirements;
		DrawSprites draw_text( neucha_font, glm::vec2(0, 0), screen_size, screen_size, DrawSprites::AlignSloppy );
		glm::vec2 tmin, size;
		glm::vec2 moving_anchor = glm::vec2(0, 0);
		for (int i=0; i<reqs->children.size(); i++) {
			// make sure this part is in sync with where the requirement labels are created (in set_main_order())
			reqs->children[i]->set_position(moving_anchor, glm::vec2(0, 0));
			draw_text.get_text_extents(reqs->children[i]->get_text(), glm::vec2(0, 0), 0.36f, &tmin, &size, 160.0f);
			moving_anchor.y += size.y + 4.0f;
		}
	});
	
	// DAILY ORDER ---------
	
	UIElem* order2_rolledup = new UIElem(
		order2,
		glm::vec2(0, 0), //anchor
		glm::vec2(-order_w1, 0), //pos
		glm::vec2(order_w1, order_h), //size
		ui_sprites.order.rolledup, "",
		glm::vec2(order_w1, -14), //sprite pos
		0.45f, true, false, false);

	UIElem* order2_expanded = new UIElem(
		order2,
		glm::vec2(0, 0), //anchor
		glm::vec2(-order_w2, 0), //pos
		glm::vec2(order_w2, order_h), //size
		ui_sprites.order.expanded, "",
		glm::vec2(order_w2, -14), //sprite pos
		0.45f, true, true, false);

	order2_rolledup->set_on_mouse_enter([order2_rolledup, order2_expanded](){
		order2_rolledup->hide();
		order2_expanded->show();
	});
	order2_expanded->set_on_mouse_leave([order2_rolledup, order2_expanded](){
		order2_rolledup->show();
		order2_expanded->hide();
	});

	UI.daily_order.description = new UIElem(
		order2_expanded,
		glm::vec2(0, 0), //anchor
		glm::vec2(20, 10), //pos
		glm::vec2(0, 0), //size doesn't matter
		nullptr, "default description",
		glm::vec2(0, 0), 0.4f);
	UI.daily_order.description->set_tint(text_tint);
	UI.daily_order.description->set_max_text_width(224.0f);

	complete_btn = new UIElem(
		order2_expanded,
		glm::vec2(0, 1), //anchor
		glm::vec2(24, -38), //pos
		glm::vec2(82, 22), //size
		nullptr, "COMPLETE",
		glm::vec2(0, 0), 0.4f, true);
	complete_btn->set_tint(text_tint);
	complete_btn->set_on_mouse_enter([this, complete_btn](){ complete_btn->set_tint(text_highlight_tint); });
	complete_btn->set_on_mouse_leave([this, complete_btn](){ complete_btn->set_tint(text_tint); });
	complete_btn->set_on_mouse_down([this](){
		// std::cout << "Submit Button Click!" << std::endl;
		std::map< PlantType const*, int > require_plants = current_daily_order->get_required_plants();
		bool orderFinished = true;
		std::map<PlantType const*, int>::iterator iter = require_plants.begin();
		while( iter != require_plants.end() ) {
			PlantType const* require_type = iter->first;
			int needed_num = iter->second;
			if( inventory.get_harvest_num(require_type) < needed_num ){
				orderFinished = false;
				break;
			}
			iter++;
		}
		std::cout << orderFinished << std::endl;
		if( orderFinished == true ){
			iter = require_plants.begin();
			while( iter != require_plants.end() ) {
				PlantType const* require_type = iter->first;
				int needed_num = iter->second;
				inventory.change_harvest_num(require_type, -needed_num);
				iter++;
			}
			Sound::play( *order_completed_sound, 0.0f, 1.0f );
			change_num_coins( current_daily_order->get_bonus_cash() );
			current_daily_order_idx += 1;
			if( current_daily_order_idx >= daily_orders.size() ){
				current_daily_order_idx = 0;
			}
			set_daily_order(current_daily_order_idx);
		}
	});

	/*UIElem* cancel_btn = new UIElem(
		order2_expanded,
		glm::vec2(0, 1), //anchor
		glm::vec2(120, -38), //pos
		glm::vec2(66, 22), //size
		nullptr, "CANCEL",
		glm::vec2(0, 0), 0.4f, true);
	cancel_btn->set_tint(text_tint);
	cancel_btn->set_on_mouse_enter([this, cancel_btn](){ cancel_btn->set_tint(text_highlight_tint); });
	cancel_btn->set_on_mouse_leave([this, cancel_btn](){ cancel_btn->set_tint(text_tint); });
	cancel_btn->set_on_mouse_down([this](){
		if(cancel_order_state==true){
			current_daily_order_idx += 1;
			if(current_daily_order_idx >= daily_orders.size()){
				current_daily_order_idx = 0;
			}
			current_daily_order = daily_orders[current_daily_order_idx];
			cancel_order_freeze_time = 10;
		}else{
			std::cout << "Cannot cancel this order!" << std::endl;
		}				
	});*/

	UI.daily_order.reward = new UIElem(
		order2,
		glm::vec2(1, 0), //anchor
		glm::vec2(-128, 24), //pos
		glm::vec2(0, 0), //size doesn't matter
		nullptr, "default reward text",
		glm::vec2(0, 0), 0.44f);
	UI.daily_order.reward->set_tint(text_tint);
	UI.daily_order.reward->set_max_text_width(180.0f);

	UI.daily_order.requirements = new UIElem(
		order2,
		glm::vec2(1, 0), //anchor
		glm::vec2(-142, 58), //pos
		glm::vec2(0, 0), //size
		nullptr, "",
		glm::vec2(0, 0), 1.0f);
	UI.daily_order.requirements->set_layout_children_fn([this](){
		UIElem* reqs = UI.daily_order.requirements;
		DrawSprites draw_text( neucha_font, glm::vec2(0, 0), screen_size, screen_size, DrawSprites::AlignSloppy );
		glm::vec2 tmin, size;
		glm::vec2 moving_anchor = glm::vec2(0, 0);
		for (int i=0; i<reqs->children.size(); i++) {
			// make sure this part is in sync with where the requirement labels are created (in set_daily_order())
			reqs->children[i]->set_position(moving_anchor, glm::vec2(0, 0));
			draw_text.get_text_extents(reqs->children[i]->get_text(), glm::vec2(0, 0), 0.36f, &tmin, &size, 160.0f);
			moving_anchor.y += size.y + 4.0f;
		}
	});
}
