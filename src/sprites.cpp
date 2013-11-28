
#include <pastry/pastry.hpp>

namespace pastry
{
	namespace sprites
	{
		struct render_group
		{
			std::string sheet_tag;
			program spo;
			array_buffer vbo;
			vertex_array vao;
			texture tex;
			int w, h;

			void render(const std::vector<vertex>& v) {
				vbo.bind();
				vbo.update_data(v);
				texture::activate_unit(0);
				tex.bind();
				spo.use();
				vao.bind();
				glDrawArrays(GL_TRIANGLES, 0, v.size());
			}
		};

		struct sprite_manager
		{
			std::map<std::string, def_sheet> def_sheets_;
			std::map<std::string, def_sprite> def_sprites_;
			std::map<std::string, def_anim> def_anims_;

			std::map<std::string, std::string> sprite_to_sheet_;

			program spo;

			std::map<std::string, render_group> groups_;

			sprite_manager();
			
			void add_sheet(const def_sheet& sheet);

			void add_anim(const def_anim& anim);

			std::string find_frame(const def_anim& da, float t);
		};

		sprite_manager::sprite_manager()
		{
			std::string vert_src = PASTRY_GLSL(
				in vec2 position;
				in vec2 texcoord;
				out vec2 Texcoord;
				void main() {
					gl_Position = vec4(position, 0.0, 1.0f);
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

		void sprite_manager::add_sheet(const def_sheet& sheet)
		{
			def_sheets_[sheet.tag] = sheet;

			render_group g;

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
			g.tex.set_filter(GL_NEAREST);
			g.w = g.tex.width();
			g.h = g.tex.height();
			groups_[sheet.tag] = g;

			for(const auto& s : sheet.sprites) {
				def_sprites_[s.tag] = s;
				sprite_to_sheet_[s.tag] = sheet.tag;
			}
		}

		void sprite_manager::add_anim(const def_anim& anim)
		{
			def_anims_[anim.tag] = anim;
		}

		std::string sprite_manager::find_frame(const def_anim& da, float t)
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

		std::shared_ptr<sprite_manager> s_sprite_manager;

		void initialize()
		{
			s_sprite_manager = std::make_shared<sprite_manager>();
		}

		void add_sprite_sheet(const def_sheet& sheet)
		{
			s_sprite_manager->add_sheet(sheet);
		}

		void add_sprite_animation(const def_anim& anim)
		{
			s_sprite_manager->add_anim(anim);
		}

		sprite_group::sprite_group()
		{
		}
		
		sprite_group::~sprite_group()
		{

		}

		sprite_ptr sprite_group::add_sprite(const std::string& tag)
		{
			auto p = std::make_shared<sprite>();
			p->tag = tag;
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
				auto it = s_sprite_manager->def_anims_.find(tag);
				if(it != s_sprite_manager->def_anims_.end()) {
					// animation
					tag = s_sprite_manager->find_frame(it->second, s.t);
				}
				// find sprite defintion;
				auto it2 = s_sprite_manager->def_sprites_.find(tag);
				if(it2 == s_sprite_manager->def_sprites_.end()) {
					continue;
				}
				const def_sprite& st = it2->second;
				std::string sheet_tag = s_sprite_manager->sprite_to_sheet_[tag];
				std::vector<vertex>& vv = vertices_[sheet_tag];
				render_group& g = s_sprite_manager->groups_[sheet_tag];
				float u_div = 1.0f / static_cast<float>(g.w);
				float v_div = 1.0f / static_cast<float>(g.h);
				vv.push_back(vertex{s.x-s.sx, s.y+s.sy, (st.u_px         )*u_div, (st.v_px+st.sv_px)*v_div});
				vv.push_back(vertex{s.x+s.sx, s.y+s.sy, (st.u_px+st.su_px)*u_div, (st.v_px+st.sv_px)*v_div});
				vv.push_back(vertex{s.x+s.sx, s.y-s.sy, (st.u_px+st.su_px)*u_div, (st.v_px         )*v_div});
				vv.push_back(vertex{s.x-s.sx, s.y+s.sy, (st.u_px         )*u_div, (st.v_px+st.sv_px)*v_div});
				vv.push_back(vertex{s.x+s.sx, s.y-s.sy, (st.u_px+st.su_px)*u_div, (st.v_px         )*v_div});
				vv.push_back(vertex{s.x-s.sx, s.y-s.sy, (st.u_px         )*u_div, (st.v_px         )*v_div});
			}
		}

		void sprite_group::render()
		{
			for(auto& q : vertices_) {
				s_sprite_manager->groups_[q.first].render(q.second);
			}
		}

	}
}
