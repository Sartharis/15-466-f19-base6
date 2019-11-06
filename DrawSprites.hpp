#pragma once

#include "Sprite.hpp"
#include <glm/glm.hpp>
#include <vector>

/* Struct that contains additional information for rendering text:
 * 	xadvance for each char,
 * 	kerning pairs
 */
struct Font {

	typedef std::unordered_map< std::string, std::unordered_map< std::string, int > > KerningMap;
	typedef std::unordered_map< std::string, int > AdvanceMap;

	Font( SpriteAtlas const* _atlas, 
				KerningMap _kerning_map,
				AdvanceMap _advance_map ) : 
		atlas(_atlas),
		kerning_map(_kerning_map),
		advance_map(_advance_map) {}

	SpriteAtlas const* atlas;
	KerningMap kerning_map;
	AdvanceMap advance_map;

};
extern Font const* neucha_font;

/*
 * Helper class for drawing a bunch of sprites from a single atlas.
 * Usage:
 *	{
 *		DrawSprites draw_sprites(atlas, screen_min, screen_max, DrawSprites::PixelPerfect);
 *		draw_sprites.draw(
 *
 */
struct DrawSprites {
	enum AlignMode {
		AlignPixelPerfect,
		AlignSloppy
	};
	//Starts accumulating sprites to draw on allocation:
	DrawSprites(SpriteAtlas const &atlas, glm::vec2 const &view_min, glm::vec2 const &view_max, glm::uvec2 const &drawable_size, AlignMode mode = AlignSloppy);

	//another constructor for drawing text
	DrawSprites(Font const *_font, glm::vec2 const &view_min, glm::vec2 const &view_max, glm::uvec2 const &drawable_size, AlignMode mode = AlignSloppy);

	//Add more sprites to draw:
	void draw(Sprite const &sprite, glm::vec2 const &center, float scale = 1.0f, glm::u8vec4 const &tint = glm::u8vec4(0xff, 0xff, 0xff, 0xff));

	//Add text to draw:
	void draw_text(std::string const &name, glm::vec2 const &anchor, float scale = 1.0f, glm::u8vec4 const &tint = glm::u8vec4(0xff, 0xff, 0xff, 0xff), glm::vec2 *anchor_out = nullptr);

	//Measure text:
	void get_text_extents(std::string const &name, glm::vec2 const &anchor, float scale, glm::vec2 *min, glm::vec2 *max);



	//Actually draws the sprites on deallocation:
	~DrawSprites();

	//--- internals ---
	Font const* font = nullptr;
	SpriteAtlas const &atlas;
	glm::vec2 view_min, view_max;
	glm::uvec2 drawable_size;
	AlignMode mode;
	glm::mat4 to_clip;
	float get_xadvance(std::string const& char_a, std::string const* char_b); // returns xadvance at scale 1

	struct Vertex {
		Vertex(glm::vec2 const &Position_, glm::vec2 const &TexCoord_, glm::u8vec4 const &Color_) : Position(Position_), TexCoord(TexCoord_), Color(Color_) { }
		glm::vec2 Position;
		glm::vec2 TexCoord;
		glm::u8vec4 Color;
	};
	std::vector< Vertex > attribs;
};
