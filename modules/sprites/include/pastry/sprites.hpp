#pragma once

#include <pastry/gl.hpp>
#include <pastry/pastry.hpp>
#include <map>
#include <string>
#include <vector>
#include <memory>

namespace pastry {

namespace detail
{
	struct def_sprite
	{
		std::string tag;
		float u_px, v_px;
		float su_px, sv_px;
	};

	struct def_sheet
	{
		std::string tag;
		texture_2d tex;
		std::vector<def_sprite> sprites;
	};

	struct def_anim_frame
	{
		std::string sprite_tag;
		float dt;
	};

	struct def_anim
	{
		std::string tag;
		std::vector<def_anim_frame> frames;
	};

	struct sprite_vertex
	{
		float x, y;
		float u, v;
	};
}

struct sprite
{
	/** The tag under which the sprite definition is found */
	std::string tag;
	/** Sprite position */
	float x, y;
	/** Sprite scale factors */
	float sx, sy;
	/** internal */
	float t;
};

PASTRY_DEFINE_PTR(sprite)

/** Initializes sprite system */
void sprites_initialize();

/** Adds a sprite sheet */
void sprites_add_sheet(const detail::def_sheet& sheet, bool filter_nearest=true);

/** Adds a sprite animation */
void sprites_add_animation(const detail::def_anim& anim);

/** A group of sprites */
class sprite_group
: public renderling
{
public:
	/** Creates a new sprite with given tag */
	sprite_ptr add_sprite(const std::string& tag);
	/** Removes the specified sprite */
	void remove_sprite(const sprite_ptr& s);
	/** Updates sprite animations preparse for rendering */
	void update(float t, float dt);
	/** Renders all sprites */
	void render();
private:
	std::vector<sprite_ptr> sprites_;
	std::map<std::string, std::vector<detail::sprite_vertex>> vertices_;
};

}
