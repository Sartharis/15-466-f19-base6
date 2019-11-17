#include "UIElem.hpp"

UIElem::~UIElem(){
	for (int i=0; i<children.size(); i++) {
		delete children[i];
	}
}

void UIElem::on_resize(glm::vec2 new_screen_size){
	update_absolute_position(new_screen_size);
	for (int i=0; i<children.size(); i++) {
		children[i]->on_resize(new_screen_size);
	}
}

UIElem::Action UIElem::test_event(glm::vec2 mouse_pos, Action action){
	for (int i=0; i<children.size(); i++) {
		Action child_result = children[i]->test_event(mouse_pos, action);
		if (child_result != none) return child_result;
	}

	if (hidden || !interactive) return none;

	switch (action) {
	case mouseDown:
		if (inside(mouse_pos)) {
			if (on_mouse_down) on_mouse_down();
			return mouseDown;
		}
		break;

	case mouseUp:
		if (inside(mouse_pos)) {
			if (on_mouse_up) on_mouse_up();
			return mouseUp;
		}
		break;

	case mouseEnter:
		if (!hovered && inside(mouse_pos)) {
			if (on_mouse_enter) on_mouse_enter();
			hovered = true;
			return mouseEnter;
		}
		break;

	case mouseLeave:
		if (hovered && !inside(mouse_pos)) {
			if (on_mouse_leave) on_mouse_leave();
			hovered = false;
			return mouseLeave;
		}
		break;

	case none:
		return none;
		break;
	}
	return none;
}

bool UIElem::inside(glm::vec2 mouse_pos){
	if (mouse_pos.x >= absolute_position.x
			&& mouse_pos.x < absolute_position.x + size.x
			&& mouse_pos.y >= absolute_position.y
			&& mouse_pos.y < absolute_position.y + size.y) {
		return true;
	} else {
		return false;
	}
}

void UIElem::set_position(glm::vec2 _position, glm::vec2 _anchor, glm::vec2 screen_size){
	anchor = _anchor;
	position = _position;
	update_absolute_position(screen_size);
}

// NOTE: requires parent to be updated before children
void UIElem::update_absolute_position(glm::vec2 screen_size){
	if (!parent) {
		size = screen_size;
		absolute_position = glm::vec2(0, 0);
	} else {
		absolute_position.x = parent->absolute_position.x + anchor.x * parent->size.x + position.x;
		absolute_position.y = parent->absolute_position.y + anchor.y * parent->size.y + position.y;
	}
}

void UIElem::draw(DrawSprites& draw_sprites, DrawSprites& draw_text){
	if (hidden) return;
	// draw this
	glm::vec2 drawable_size = draw_sprites.drawable_size;
	glm::vec2 draw_sprite_anchor = absolute_position + sprite_position;
	draw_sprite_anchor.y = drawable_size.y - draw_sprite_anchor.y;
	if (sprite) {
		draw_sprites.draw(*sprite, draw_sprite_anchor, scale);
	} else {
		assert(draw_text.font);
		draw_text.draw_text(text, draw_sprite_anchor, scale);
	}
	
	for (int i=0; i<children.size(); i++) {
		children[i]->draw(draw_sprites, draw_text);
	}
}
