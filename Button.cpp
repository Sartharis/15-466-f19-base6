#include "Button.hpp"
#include <iostream>

bool Button::try_click(glm::vec2 mouse_pos) {
	// early return if no on_click function attached
	if (on_click == nullptr) return false;

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

void Button::draw(DrawSprites& draw_sprites, DrawSprites& draw_text) {
	glm::vec2 drawable_size = draw_sprites.drawable_size;

	// draw sprite
	if (sprite) {
		glm::vec2 sprite_anchor = get_position() + get_sprite_anchor();
		glm::vec2 sprite_draw_anchor = glm::vec2( sprite_anchor.x, drawable_size.y - sprite_anchor.y );
		draw_sprites.draw( *sprite, sprite_draw_anchor );
	}

	// TODO: draw text
	if (text.length() > 0) {
		glm::vec2 text_anchor = get_position() + get_text_anchor();
		glm::vec2 text_draw_anchor = glm::vec2( text_anchor.x, drawable_size.y - text_anchor.y );
		draw_text.draw_text( text, text_draw_anchor );
	}

}
