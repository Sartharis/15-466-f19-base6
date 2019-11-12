#include "Button.hpp"
#include <iostream>

bool Button::try_click(glm::vec2 mouse_pos) {
	// early return if no on_click function attached
	if (hidden || on_click == nullptr) return false;

	if (mouse_pos.x >= position.x
			&& mouse_pos.x < position.x + size.x
			&& mouse_pos.y >= position.y
			&& mouse_pos.y < position.y + size.y) {
		on_click();
		return true;
	} else {
		return false;
	}
}

void Button::update_position(glm::vec2 screen_size) {
	switch (screen_anchor) {
	case tl:
		position = rel_position;
		break;
	case tr:
		position = glm::vec2(screen_size.x, 0) + rel_position;
		break;
	case bl:
		position = glm::vec2(0, screen_size.y) + rel_position;
		break;
	case br:
		position = screen_size + rel_position;
		break;
	}
}

void Button::update_hover(glm::vec2 mouse_pos) {
	if( hidden ) {
		hovered = false;
		return;
	}
	if (mouse_pos.x >= position.x
			&& mouse_pos.x < position.x + size.x
			&& mouse_pos.y >= position.y
			&& mouse_pos.y < position.y + size.y) {
		hovered = true;
	} else {
		hovered = false;
	}
}

void Button::set_position(ScreenAnchor new_screen_anchor, glm::vec2 new_rel_position, glm::vec2 screen_size) {
	screen_anchor = new_screen_anchor;
	rel_position = new_rel_position;
	update_position(screen_size);
}

void Button::draw_sprite(DrawSprites& draw_sprites) {
	if( hidden ) return;
	glm::vec2 drawable_size = draw_sprites.drawable_size;
	// draw sprite
	if (sprite) {
		glm::vec2 sprite_anchor = position + get_sprite_anchor();
		glm::vec2 sprite_draw_anchor = glm::vec2( sprite_anchor.x, drawable_size.y - sprite_anchor.y );
		draw_sprites.draw( *sprite, sprite_draw_anchor, sprite_scale );
	}
}

void Button::draw_text(DrawSprites& draw_text) {
	if( hidden || hover_behavior == Button::none ) return;
	glm::vec2 drawable_size = draw_text.drawable_size;
	// draw text
	if ( hover_behavior == Button::show_text && hovered && text.length() > 0 ) {
		glm::vec2 text_anchor = position + get_text_anchor();
		glm::vec2 text_draw_anchor = glm::vec2( text_anchor.x, drawable_size.y - text_anchor.y );
		draw_text.draw_text( text, text_draw_anchor, text_scale, text_tint );

	} else if( hover_behavior == Button::darken_text && text.length() > 0 ) {
		glm::vec2 text_anchor = position + get_text_anchor();
		glm::vec2 text_draw_anchor = glm::vec2( text_anchor.x, drawable_size.y - text_anchor.y );
		float amt = 0.55f;
		glm::u8vec4 color = hovered ? glm::u8vec4( text_tint.r*amt, text_tint.g*amt, text_tint.b*amt, 255 ) : text_tint;
		draw_text.draw_text( text, text_draw_anchor, text_scale, color );
	}
}
