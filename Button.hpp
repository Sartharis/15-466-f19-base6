#pragma once

#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include <glm/glm.hpp>
#include <iostream>

/* A button, which can have a sprite and a string associated with it
 * If it has a sprite, draws the sprite.
 * Otherwise, draws the text.
 */
struct Button {

	/* show_text: additionally draw the text if the button has a sprite and is currently hovered. Nothing happens if it doesn't have a sprite (just draw the text as usual)
	 * none: nothing happens on hover
	 */
	enum HoverBehavior { show_text, darken_text, none };
	enum ScreenAnchor{ tl, tr, bl, br };

	Button (
			glm::vec2 screen_size = glm::vec2(960, 600),
			ScreenAnchor _screen_anchor = tl,
			glm::vec2 _rel_position = glm::vec2(0, 0),
			glm::vec2 _size = glm::vec2(20, 20),			
			const Sprite* _sprite = nullptr,
			glm::vec2 _sprite_anchor = glm::vec2(0, 0),
			float _sprite_scale = 1.0f,
			HoverBehavior _hover_behavior = Button::none,
			std::string _text = "",
			glm::vec2 _text_anchor = glm::vec2(0, 0),
			float _text_scale = 1.0f,
			std::function<void()> const &_on_click = nullptr,
			bool _hidden = false,
			glm::u8vec4 _text_tint = glm::u8vec4(255, 255, 255, 255)
			) :	hidden(_hidden),
					hover_behavior(_hover_behavior),
					rel_position(_rel_position),
					size(_size),
					screen_anchor(_screen_anchor),
					sprite(_sprite),
					sprite_anchor(_sprite_anchor),
					sprite_scale(_sprite_scale),
					text(_text),
					text_anchor(_text_anchor),
					text_scale(_text_scale),
					text_tint(_text_tint),
					on_click(_on_click) { update_position(screen_size); }

	// getters
	glm::vec2 get_position() const { return position; };
	bool get_hovered() const { return hovered; }
	glm::vec2 get_size() const { return size; }
	const Sprite* get_sprite() const { return sprite; }
	glm::vec2 get_sprite_anchor() const { return sprite_anchor; }
	std::string get_text() const { return text; }
	glm::vec2 get_text_anchor() const { return text_anchor; }

	bool hidden = false;

	// functions
	bool update_hover(glm::vec2 mouse_pos);
	void update_position(glm::vec2 new_screensize);
	void set_position(ScreenAnchor new_screen_anchor, glm::vec2 new_rel_pos, glm::vec2 screen_size);
	bool try_click(glm::vec2 mouse_pos); // if mouse_pos is on this button, call on_click() and return true.
	void draw_sprite(DrawSprites& draw_sprites);
	void draw_text(DrawSprites& draw_text);

private:
	// hover state
	bool hovered = false;
	HoverBehavior hover_behavior = Button::none;

	// shape & location
	glm::vec2 rel_position;
	glm::vec2 position = glm::vec2(0, 0);// updated on resize
	glm::vec2 size;
	ScreenAnchor screen_anchor;

	// sprite
	const Sprite* sprite = nullptr;
	glm::vec2 sprite_anchor = glm::vec2(0, 0);
	float sprite_scale = 1.0f;

	// text
	std::string text = "";
	glm::vec2 text_anchor = glm::vec2(0, 0);
	float text_scale = 1.0f;
	glm::u8vec4 text_tint = glm::u8vec4(255, 255, 255, 255);

	// on_click
	std::function<void()> const on_click = nullptr;
};

