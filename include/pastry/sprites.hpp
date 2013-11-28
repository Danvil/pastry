#ifndef INCLUDED_PASTRY_SPRITES_HPP
#define INCLUDED_PASTRY_SPRITES_HPP

#include "pastry_gl.hpp"
#include <map>
#include <vector>
#include <string>

namespace pastry
{
	namespace sprites
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
			std::string path;
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

		struct sprite
		{
			std::string tag;
			float x, y;
			float sx, sy;
			float t;
		};

		struct vertex
		{
			float x, y;
			float u, v;
		};

		typedef std::shared_ptr<sprite> sprite_ptr;

		void add_sprite_sheet(const def_sheet& sheet);

		void add_sprite_animation(const def_anim& anim);

		class sprite_group
		: public renderling
		{
		public:
			sprite_group();
			~sprite_group();
			sprite_ptr add_sprite(const std::string& tag);
			void remove_sprite(const sprite_ptr& s);
			void update(float t, float dt);
			void render();
		private:
			std::vector<sprite_ptr> sprites_;
			std::map<std::string, std::vector<vertex>> vertices_;
		};

	}
}

#endif
