#include "UIElem.hpp"
#include "data_path.hpp"

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

UIElem::Action UIElem::test_event(glm::vec2 mouse_pos, Action action){
	if (hidden) return none;
	for (int i=0; i<children.size(); i++) {
		Action child_result = children[i]->test_event(mouse_pos, action);
		if (child_result != none) {
			return child_result;
		}
	}

	if (!interactive) return none;
	switch (action) {
	case mouseDown:
		if (inside(mouse_pos)) {
			if (default_sound) Sound::play( *button_click_sound, 0.0f, 1.0f );
			if (on_mouse_down) {
				on_mouse_down();
			}
			return mouseDown;
		}
		break;

	case mouseUp:
		if (inside(mouse_pos)) {
			if (on_mouse_up) {
				if (default_sound) Sound::play( *button_click_sound, 0.0f, 1.0f );
				on_mouse_up();
			}
			return mouseUp;
		}
		break;

	case mouseEnter:
		if (!hovered && inside(mouse_pos)) {
			if (default_sound) Sound::play( *button_hover_sound, 0.0f, 1.0f );
			if (on_mouse_enter) {
				on_mouse_enter();
			}
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
void UIElem::update_absolute_position(glm::vec2 new_screen_size){
	if (!parent) {
		size = new_screen_size;
		absolute_position = glm::vec2(0, 0);
	} else {
		absolute_position.x = parent->absolute_position.x + anchor.x * parent->size.x + position.x;
		absolute_position.y = parent->absolute_position.y + anchor.y * parent->size.y + position.y;
	}
	for (int i=0; i<children.size(); i++) {
		children[i]->update_absolute_position(new_screen_size);
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
		draw_text.draw_text(text, draw_sprite_anchor, scale, tint);
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
		draw_text.draw_text(text, draw_sprite_anchor, scale, tint);
	}
}

void UIElem::gather(std::vector<UIElem*> &list) {
	list.push_back(this);
	for (int i=0; i<children.size(); i++) {
		children[i]->gather(list);
	}
}
