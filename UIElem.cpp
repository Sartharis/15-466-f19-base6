#include "UIElem.hpp"
#include "data_path.hpp"
#include <algorithm>

Load< Sound::Sample > button_hover_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "UI_Click_Cut_mono.wav" ) );
} );

Load< Sound::Sample > button_click_sound( LoadTagDefault, []() -> Sound::Sample const* {
	return new Sound::Sample( data_path( "Click.wav" ) );
} );

UIElem::~UIElem(){
	for (int i=0; i<children.size(); i++) {
		delete children[i];
	}
}

// TODO: should take z-index into consideration here as well
bool UIElem::test_event_mouse(glm::vec2 mouse_pos, Action action){
	if (hidden || get_in_animation()) return false;
	for (int i=0; i<children.size(); i++) {
		if (children[i]->test_event_mouse(mouse_pos, action)) return true;
	}

	if (!interactive) return false;
	switch (action) {
	case mouseDown:
		if (inside(mouse_pos)) {
			if (default_sound) Sound::play( *button_click_sound, 0.0f, 1.0f );
			if (on_mouse_down) {
				on_mouse_down();
			}
			return true;
		}
		break;

	case mouseUp:
		/*
		if (inside(mouse_pos)) {
			if (on_mouse_up) {
				if (default_sound) Sound::play( *button_click_sound, 0.0f, 1.0f );
				on_mouse_up();
			}
			return true;
		} */
		break;

	case mouseEnter:
		if (!hovered && inside(mouse_pos)) {
			if (default_sound) Sound::play( *button_hover_sound, 0.0f, 1.0f );
			if (on_mouse_enter) {
				on_mouse_enter();
			}
			hovered = true;
			return true;
		}
		break;

	case mouseLeave:
		if (hovered && !inside(mouse_pos)) {
			if (on_mouse_leave) on_mouse_leave();
			hovered = false;
			return true;
		}
		break;

	case none:
		return false;
		break;
	}
	return false;
}

bool UIElem::test_event_keyboard(SDL_Keycode key_in) {
	if (hidden || get_in_animation()) return false;
	for (int i=0; i<children.size(); i++) {
		if (children[i]->test_event_keyboard(key_in)) return true;
	}
	if (!interactive) return false;
	if (key_in == hotkey && on_mouse_down) {
		on_mouse_down();
		return true;
	}
	return false;
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

void UIElem::set_position(glm::vec2 _position, glm::vec2 _anchor, float _animation_duration){
	anchor = _anchor;
	// if anim duration is small, there's no point animating. just jump to target.
	if (_animation_duration >= 0.001f) {
		animation_duration = _animation_duration;
		start_position = position;
		target_position = _position;
		timeout = _animation_duration;
	} else {
		position = _position;
	}
	update_absolute_position();
}

// NOTE: requires parent to be updated before children
void UIElem::update_absolute_position(){
	if (!parent) {
		absolute_position = glm::vec2(0, 0);
	} else {
		absolute_position.x = parent->absolute_position.x + anchor.x * parent->size.x + position.x;
		absolute_position.y = parent->absolute_position.y + anchor.y * parent->size.y + position.y;
	}
	for (int i=0; i<children.size(); i++) {
		children[i]->update_absolute_position();
	}
}

void UIElem::set_parent(UIElem* _parent){ 
	// remove this from old parent's children list
	if (parent) {
		int index = 0;
		for (int i=0; i<parent->children.size(); i++) {
			if (parent->children[i] == this) { index = i; break; }
		}
		assert(index != parent->children.size());
		parent->children.erase(parent->children.begin() + index, parent->children.begin() + index + 1);
		parent->layout_children();
	}
	// set new parent
	parent = _parent; 
	if (_parent) _parent->add_child(this); 
}

void UIElem::layout_children(){
	if (layout_children_fn) layout_children_fn(); 
	for (auto c : children) c->layout_children();
}

bool UIElem::get_hidden_from_hierarchy() {
	if (!parent) return hidden;
	return parent->get_hidden_from_hierarchy() || hidden;
}

int UIElem::get_absolute_z_index() {
	if (!parent) return z_index;
	return parent->get_absolute_z_index() + z_index;
}

void UIElem::clear_children() {
	for (int i=0; i<children.size(); i++) {
		delete children[i];
	}
	children = {};
}

glm::vec2 ease_out(glm::vec2 start, glm::vec2 end, float t) {
	// a list of easing functions: https://gist.github.com/gre/1650294
	float ease_param = 1.0f - std::pow( 1.0f - t, 3 ); // cubic
	return start + (end - start) * ease_param;
}

void UIElem::update(float elapsed) {

	if (get_in_animation()) {
		timeout = std::max(0.0f, timeout - elapsed);
		float progress = 1.0f - timeout / animation_duration;
		position = ease_out(start_position, target_position, progress);
		update_absolute_position();
	}
	
	// update children
	for( int i=0; i<children.size(); i++) {
		children[i]->update(elapsed);
	}

}

void UIElem::draw(DrawSprites& draw_sprites, DrawSprites& draw_text){
	if (hidden) return;
	// draw this
	glm::vec2 drawable_size = draw_sprites.drawable_size;
	glm::vec2 draw_sprite_anchor = absolute_position + sprite_position;
	draw_sprite_anchor.y = drawable_size.y - draw_sprite_anchor.y;
	if (sprite) {
		draw_sprites.draw(*sprite, draw_sprite_anchor, scale, tint);
	} else {
		assert(draw_text.font);
		draw_text.draw_text(text, draw_sprite_anchor, scale, tint, max_text_width);
	}
	// draw children
	for (int i=0; i<children.size(); i++) {
		children[i]->draw(draw_sprites, draw_text);
	}
}

void UIElem::draw_self(DrawSprites& draw_sprites, DrawSprites& draw_text) {
	if (get_hidden_from_hierarchy()) return;
	// draw this
	glm::vec2 drawable_size = draw_sprites.drawable_size;
	glm::vec2 draw_sprite_anchor = absolute_position + sprite_position;
	draw_sprite_anchor.y = drawable_size.y - draw_sprite_anchor.y;
	if (sprite) {
		draw_sprites.draw(*sprite, draw_sprite_anchor, scale, tint);
	} else {
		assert(draw_text.font);
		draw_text.draw_text(text, draw_sprite_anchor, scale, tint, max_text_width);
	}
}

void UIElem::gather(std::vector<UIElem*> &list) {
	list.push_back(this);
	for (int i=0; i<children.size(); i++) {
		children[i]->gather(list);
	}
}
