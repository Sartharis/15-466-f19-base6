#pragma once

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include <glm/glm.hpp>
#include <iostream>

/* A button, which can have a sprite and a string associated with it
 */
struct Button {
	Button (
			glm::vec2 _position = glm::vec2(0, 0),
			glm::vec2 _size = glm::vec2(20, 20),			
			const Sprite* _sprite = nullptr,
			glm::vec2 _sprite_anchor = glm::vec2(0, 0),
			std::string _text = "",
			glm::vec2 _text_anchor = glm::vec2(0, 0),
			std::function<void()> const &_on_click = nullptr
			) : position(_position),
					size(_size),
					sprite(_sprite),
					sprite_anchor(_sprite_anchor),
					text(_text),
					text_anchor(_text_anchor),
					on_click(_on_click) {}

	// getters
	bool is_hidden() const { return hidden; }
	glm::vec2 get_position() const { return position; }
	glm::vec2 get_size() const { return size; }
	const Sprite* get_sprite() const { return sprite; }
	glm::vec2 get_sprite_anchor() const { return sprite_anchor; }
	std::string get_text() const { return text; }
	glm::vec2 get_text_anchor() const { return text_anchor; }

	// functions
	bool try_click(glm::vec2 mouse_pos); // if mouse_pos is on this button, call on_click() and return true.
	void draw(DrawSprites& draw_sprites, DrawSprites& draw_text);

private:
	// whether button is hidden
	bool hidden = false;
	// shape & location
	glm::vec2 position;
	glm::vec2 size;
	// sprite & text
	const Sprite* sprite = nullptr;
	glm::vec2 sprite_anchor = glm::vec2(0, 0);
	std::string text = "";
	glm::vec2 text_anchor = glm::vec2(0, 0);
	// on_click
	std::function<void()> const on_click = nullptr;
};

