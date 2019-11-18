#pragma once

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include <glm/glm.hpp>
#include <iostream>
#include <functional>

struct UIElem {

	enum Action { mouseDown, mouseUp, mouseEnter, mouseLeave, none };

	UIElem(
			UIElem* _parent, // hierarchy
			glm::vec2 _anchor, // transform
			glm::vec2 _position,
			glm::vec2 _size,
			Sprite const* _sprite, // content
			std::string _text,
			glm::vec2 _sprite_position,
			float _scale,
			bool _interactive = false, // optional
			bool _hidden = false
			) : sprite(_sprite),
					text(_text),
					sprite_position(_sprite_position),
					scale(_scale),
					anchor(_anchor),
					position(_position),
					size(_size),
					parent(_parent),
					interactive(_interactive),
					hidden(_hidden) {
						if (parent) parent->add_child(this);
					}

	~UIElem();

	// TODO: merge everything into one sprite sheet
	void draw(DrawSprites& draw_sprites, DrawSprites& draw_text);

	void on_resize(glm::vec2 new_screen_size);
	Action test_event(glm::vec2 mouse_pos, Action action);
	void set_position(glm::vec2 _position, glm::vec2 _anchor, glm::vec2 screen_size);
	void add_child(UIElem* child){ children.push_back(child); }

	void set_on_mouse_down(std::function<void()> fn){ on_mouse_down = fn; }
	void set_on_mouse_up(std::function<void()> fn){ on_mouse_up = fn; }
	void set_on_mouse_enter(std::function<void()> fn){ on_mouse_enter = fn; }
	void set_on_mouse_leave(std::function<void()> fn){ on_mouse_leave = fn; }

	void print_name(){ std::cout << text << std::endl; }
	void show(){ hidden = false; }
	void hide(){ hidden = true; }
	
	std::vector<UIElem*> children = std::vector<UIElem*>();

private:

	// content
	Sprite const* sprite = nullptr;
	std::string text = "";
	glm::vec2 sprite_position = glm::vec2(0,0);
	float scale = 1.0f;

	// transformation states
	glm::vec2 anchor = glm::vec2(0,0); // origin (rel to parent) from which position is specified
	glm::vec2 position = glm::vec2(0,0);
	glm::vec2 size = glm::vec2(0,0);
	// transformation states that get automatically updated on resize
	glm::vec2 absolute_position = glm::vec2(0,0);
	void update_absolute_position(glm::vec2 screen_size);

	// hierarchy
	UIElem* parent = nullptr;

	// interaction behaviors
	std::function<void()> on_mouse_down = nullptr;
	std::function<void()> on_mouse_up = nullptr;
	std::function<void()> on_mouse_enter = nullptr;
	std::function<void()> on_mouse_leave = nullptr;
	
	// internal states
	bool interactive = false;
	bool hidden = false;
	bool hovered = false;

	// helpers
	bool inside(glm::vec2 mouse_pos);
	
};
