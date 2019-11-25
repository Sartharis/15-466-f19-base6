#pragma once

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "Load.hpp"
#include "Sound.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <functional>

/* currently sizeof(UIElem) gives 336. The most expensive parts are the function pointers
 * don't think we'll actually need to optimize out every bit of memory
 * but if needed (or I get bored) could also store all the bools into bits of a flag,
 * store char* instead of std::string,
 * use short for positions and sizes,
 * and use __fp16 as low-precision floats for things like anchor and scale.
 */
struct UIElem {

	enum Action { mouseDown, mouseUp, mouseEnter, mouseLeave, none };

	UIElem(
			UIElem* _parent, // hierarchy
			glm::vec2 _anchor = glm::vec2(0, 0), // transform
			glm::vec2 _position = glm::vec2(0, 0),
			glm::vec2 _size = glm::vec2(0, 0),
			Sprite const* _sprite = nullptr, // content
			std::string _text = "",
			glm::vec2 _sprite_position = glm::vec2(0, 0),
			float _scale = 1.0f,
			bool _interactive = false, // optionals
			bool _hidden = false,
			bool _default_sound = true
			) : text(_text),
					sprite_position(_sprite_position),
					sprite(_sprite),
					scale(_scale),
					anchor(_anchor),
					position(_position),
					size(_size),
					parent(_parent),
					interactive(_interactive),
					hidden(_hidden),
					default_sound(_default_sound) {
						if (parent) parent->add_child(this);
					}

	~UIElem();

	static bool z_index_comp_fn(UIElem* e1, UIElem* e2) {
		return e1->get_absolute_z_index() < e2->get_absolute_z_index();
	} 

	void update(float elapsed);
	void draw(DrawSprites& draw_sprites, DrawSprites& draw_text);
	void draw_self(DrawSprites& draw_sprites, DrawSprites& draw_text); // draw this elem only (not its children)
	void gather(std::vector<UIElem*> &list);

	Action test_event(glm::vec2 mouse_pos, Action action);

	void set_position(glm::vec2 _position, glm::vec2 _anchor, float _animation_duration = 0.0f);
	void set_size(glm::vec2 _size) { size = _size; }
	void set_scale(float _scale) { scale = _scale; }
	void set_parent(UIElem* _parent); 
	void set_text(std::string _text){ text = _text; }
	void set_z_index(int _z_index){ z_index = _z_index; }
	void set_tint(glm::u8vec4 _tint){ tint = _tint; }
	void set_max_text_width(float _w){ max_text_width = _w; }

	glm::vec2 get_position(){ return position; }
	glm::vec2 get_anchor(){ return anchor; }
	Sprite const* get_sprite(){ return sprite; }
	std::string get_text(){ return text; }
	bool get_hidden(){ return hidden; }
	UIElem* get_parent(){ return parent; }
	bool get_in_animation(){ return timeout > 0.0f; }

	void update_absolute_position();
	void add_child(UIElem* child){ children.push_back(child); }
	void clear_children();
	void layout_children();

	void set_on_mouse_down(std::function<void()> fn){ on_mouse_down = fn; }
	// void set_on_mouse_up(std::function<void()> fn){ on_mouse_up = fn; }
	void set_on_mouse_enter(std::function<void()> fn){ on_mouse_enter = fn; }
	void set_on_mouse_leave(std::function<void()> fn){ on_mouse_leave = fn; }
	void set_layout_children_fn(std::function<void()> fn){ layout_children_fn = fn; }
	void show(){ hidden = false; }
	void hide(){ hidden = true; }
	
	std::vector<UIElem*> children = std::vector<UIElem*>();

private:

	// content
	std::string text = "";
	glm::vec2 sprite_position = glm::vec2(0,0);
	Sprite const* sprite = nullptr;
	float scale = 1.0f;
	glm::u8vec4 tint = glm::vec4(255, 255, 255, 255);

	// transformation states
	glm::vec2 anchor = glm::vec2(0,0); // origin (rel to parent) from which position is specified
	glm::vec2 position = glm::vec2(0,0);
	glm::vec2 size = glm::vec2(0,0);
	// transformation states that get automatically updated on resize
	glm::vec2 absolute_position = glm::vec2(0,0);

	// hierarchy
	UIElem* parent = nullptr;

	// interaction behaviors
	std::function<void()> on_mouse_down = nullptr;
	// std::function<void()> on_mouse_up = nullptr; // storing one of these is expensive.. not used so disabled for now
	std::function<void()> on_mouse_enter = nullptr;
	std::function<void()> on_mouse_leave = nullptr;
	std::function<void()> layout_children_fn = nullptr;
	
	// animation related
	glm::vec2 target_position = glm::vec2(0,0); // good so far
	glm::vec2 start_position = glm::vec2(0,0);
	float animation_duration = 0.0f;

	// internal states
	int z_index = 0; // relative to parent. Low z-index elements get drawn first (show at bottom)
	float timeout = 0.0f;
	bool interactive = false;
	bool hidden = false;
	bool default_sound = true;

	bool hovered = false;

	// others
	float max_text_width = 10000.0f;

	// helpers
	int get_absolute_z_index();
	bool inside(glm::vec2 mouse_pos);
	bool get_hidden_from_hierarchy();
};

extern Load< Sound::Sample > button_hover_sound;
extern Load< Sound::Sample > button_click_sound;
