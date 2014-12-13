#include <pastry/gl.hpp>
#include <pastry/pastry.hpp>
#include <pastry/sprites.hpp>

namespace pastry
{
	struct sprite_render_group
	{
		std::string sheet_tag;
		program spo;
		array_buffer vbo;
		vertex_array vao;
		texture_2d tex;
		int w, h;

		void render(const std::vector<detail::sprite_vertex>& v) {
			spo.use();
			texture_2d::activate_unit(0);
			tex.bind();
			vbo.bind();
			vbo.update_data(v);
			vao.bind();
			glDrawArrays(GL_TRIANGLES, 0, v.size());
		}
	};

	struct sprite_manager
	{
		std::map<std::string, detail::def_sheet> def_sheets_;
		std::map<std::string, detail::def_sprite> def_sprites_;
		std::map<std::string, detail::def_anim> def_anims_;

		std::map<std::string, std::string> sprite_to_sheet_;

		program spo;

		std::map<std::string, sprite_render_group> groups_;

		sprite_manager();
		
		void add_sheet(const detail::def_sheet& sheet, bool filter_nearest);

		void add_anim(const detail::def_anim& anim);

		std::string find_frame(const detail::def_anim& da, float t);
	};

	sprite_manager::sprite_manager()
	{
		std::string vert_src = PASTRY_GLSL(
			in vec2 position;
			in vec2 texcoord;
			out vec2 Texcoord;
			uniform mat4 proj;
			void main() {
				gl_Position = proj*vec4(position, 0.0, 1.0f);
				Texcoord = texcoord;
			}
		);
		std::string frag_src = PASTRY_GLSL(
			in vec2 Texcoord;
			uniform sampler2D tex;
			out vec4 outColor;
			void main() {
				outColor = texture(tex, Texcoord);
			}
		);
		spo = program(vert_src, frag_src);
		spo.get_uniform<int>("tex").set(0);
	}

	void sprite_manager::add_sheet(const detail::def_sheet& sheet, bool filter_nearest)
	{
		def_sheets_[sheet.tag] = sheet;

		sprite_render_group g;

		g.sheet_tag = sheet.tag;

		g.spo = spo;

		g.vbo = array_buffer({
			{"xy", GL_FLOAT, 2},
			{"uv", GL_FLOAT, 2}
		});
		g.vbo.init_data(GL_DYNAMIC_DRAW);

		g.vao = vertex_array(spo, {
			{"position", g.vbo, "xy"},
			{"texcoord", g.vbo, "uv"}
		});

		g.tex = sheet.tex;
		if(filter_nearest)
			g.tex.set_filter(GL_NEAREST);
		else
			g.tex.set_filter(GL_LINEAR);
		g.w = g.tex.width();
		g.h = g.tex.height();
		groups_[sheet.tag] = g;

		for(const auto& s : sheet.sprites) {
			def_sprites_[s.tag] = s;
			sprite_to_sheet_[s.tag] = sheet.tag;
		}
	}

	void sprite_manager::add_anim(const detail::def_anim& anim)
	{
		def_anims_[anim.tag] = anim;
	}

	std::string sprite_manager::find_frame(const detail::def_anim& da, float t)
	{
		// FIXME complete bullshit
		float tsum = 0.0f;
		for(const auto& x : da.frames) {
			tsum += x.dt;
		}
		float p = std::fmod(t, tsum);
		tsum = 0.0f;
		for(const auto& x : da.frames) {
			tsum += x.dt;
			if(p < tsum) {
				return x.sprite_tag;
			}
		}
		// this should not happen
		std::cerr << "ERROR: This error does not happen." << std::endl;
		return "";
	}

	std::shared_ptr<sprite_manager> g_sprite_manager;

	void sprites_initialize()
	{
		g_sprite_manager = std::make_shared<sprite_manager>();
	}

	void sprites_add_sheet(const detail::def_sheet& sheet, bool filter_nearest)
	{
		g_sprite_manager->add_sheet(sheet, filter_nearest);
	}

	void sprites_add_animation(const detail::def_anim& anim)
	{
		g_sprite_manager->add_anim(anim);
	}

	sprite_ptr sprite_group::add_sprite(const std::string& tag)
	{
		auto p = std::make_shared<sprite>();
		p->tag = tag;
		p->x = 0.0f;
		p->y = 0.0f;
		p->sx = 1.0f;
		p->sy = 1.0f;
		p->t = 0.0f;
		sprites_.push_back(p);
		return p;
	}

	void sprite_group::remove_sprite(const sprite_ptr& s)
	{
		auto it = std::find(sprites_.begin(), sprites_.end(), s);
		if(it != sprites_.end()) {
			sprites_.erase(it);
		}
	}

	void sprite_group::update(float t, float dt)
	{
		for(auto& q : vertices_) {
			q.second.clear();
		}
		for(const std::shared_ptr<sprite>& sp : sprites_) {
			sprite& s = *sp;
			// animation
			s.t += dt;
			std::string tag = s.tag;
			auto it = g_sprite_manager->def_anims_.find(tag);
			if(it != g_sprite_manager->def_anims_.end()) {
				// animation
				tag = g_sprite_manager->find_frame(it->second, s.t);
			}
			// find sprite defintion;
			auto it2 = g_sprite_manager->def_sprites_.find(tag);
			if(it2 == g_sprite_manager->def_sprites_.end()) {
				continue;
			}
			const detail::def_sprite& st = it2->second;
			std::string sheet_tag = g_sprite_manager->sprite_to_sheet_[tag];
			std::vector<detail::sprite_vertex>& vv = vertices_[sheet_tag];
			sprite_render_group& g = g_sprite_manager->groups_[sheet_tag];
			float u_div = 1.0f / static_cast<float>(g.w);
			float v_div = 1.0f / static_cast<float>(g.h);
			float sx = s.sx*st.su_px;
			float sy = s.sy*st.sv_px;
			vv.push_back({s.x-sx, s.y+sy, (st.u_px         )*u_div, (st.v_px+st.sv_px)*v_div});
			vv.push_back({s.x+sx, s.y+sy, (st.u_px+st.su_px)*u_div, (st.v_px+st.sv_px)*v_div});
			vv.push_back({s.x+sx, s.y-sy, (st.u_px+st.su_px)*u_div, (st.v_px         )*v_div});
			vv.push_back({s.x-sx, s.y+sy, (st.u_px         )*u_div, (st.v_px+st.sv_px)*v_div});
			vv.push_back({s.x+sx, s.y-sy, (st.u_px+st.su_px)*u_div, (st.v_px         )*v_div});
			vv.push_back({s.x-sx, s.y-sy, (st.u_px         )*u_div, (st.v_px         )*v_div});
		}
	}

	void sprite_group::render()
	{
		// update projection matrix
		int w, h;
		fb_get_dimensions(w,h);
		Eigen::Matrix4f proj = pastry::math_orthogonal_projection(w, h, -1.0f, +1.0f);
		g_sprite_manager->spo.get_uniform<Eigen::Matrix4f>("proj").set(proj);
		// transparency
		{	auto state = capability{GL_BLEND,true};
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			// render sprites
			for(auto& q : vertices_) {
				g_sprite_manager->groups_[q.first].render(q.second);
			}
		}
	}
}
