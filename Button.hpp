#pragma once

#include "Sprite.hpp"
#include <glm/glm.hpp>
#include <iostream>

/* A button, which can have a sprite and a string associated with it
 * TODO: keep a list of buttons in PlantMode, and in handle_events call try_click for each of them?
 */
struct Button {
	Button (
			glm::vec2 _position = glm::vec2(0, 0),
			glm::vec2 _size = glm::vec2(20, 20),			
			const Sprite* _sprite = nullptr,
			glm::vec2 _sprite_anchor = glm::vec2(0, 0),
			std::string _text = "",
			glm::vec2 _text_anchor = glm::vec2(0, 0),
			void (*_on_click)() = nullptr
			) : position(_position),
					size(_size),
					sprite(_sprite),
					sprite_anchor(_sprite_anchor),
					text(_text),
					text_anchor(_text_anchor),
					on_click(_on_click) {}

	glm::vec2 position;
	glm::vec2 size;

	const Sprite* sprite = nullptr;
	glm::vec2 sprite_anchor = glm::vec2(0, 0);

	std::string text = "";
	glm::vec2 text_anchor = glm::vec2(0, 0);

	bool try_click(glm::vec2 mouse_pos); // if mouse_pos is on this button, call on_click() and return true.
	void draw();
	void (*on_click)() = nullptr;

};
