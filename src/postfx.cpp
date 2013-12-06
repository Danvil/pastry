#include <pastry/pastry.hpp>

namespace pastry {
namespace postfx {	

struct buffer
{
	int width_, height_;
	pastry::texture tex;
	pastry::renderbuffer rbo;
	pastry::framebuffer fbo;

	void create() {
		pastry::fb_get_dimensions(width_, height_);

		tex.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex.image_2d_rgba_ub(width_, height_, 0);

		rbo.id_create();
		rbo.bind();
		rbo.storage(GL_DEPTH_COMPONENT16, width_, height_);

		fbo.id_create();
		fbo.bind();
		fbo.attach(GL_COLOR_ATTACHMENT0, tex);
		fbo.attach(GL_DEPTH_ATTACHMENT, rbo);
		fbo.unbind();
	}

	void bind() {
		fbo.bind();
	}

	void update() {
		if(pastry::fb_has_changed()) {
			pastry::fb_get_dimensions(width_, height_);
			fbo.bind();
			tex.bind();
			tex.image_2d_rgba_ub(width_, height_, 0);
			rbo.bind();
			rbo.storage(GL_DEPTH_COMPONENT16, width_, height_);
			fbo.unbind();
		}
	}
};

struct effect
{
	pastry::program spo;
	pastry::array_buffer vbo;
	pastry::vertex_array vao;

	uniform<Eigen::Vector2f> u_dim;

	post_effect_ptr sfx_;

	effect() {}

	effect(const post_effect_ptr& sfx) {
		sfx_ = sfx;

		vbo = pastry::array_buffer({
			{"pos", GL_FLOAT, 2}
		});
		vbo.init_data(std::vector<float>{0.0f,0.0f}, GL_STATIC_DRAW);

		std::string src_vert = PASTRY_GLSL(
			in vec2 pos;
			void main() {
				gl_Position = vec4(pos, 0, 1);
			}
		);
		std::string src_geom = PASTRY_GLSL(
			layout(points) in;
			layout(triangle_strip) out;
			layout(max_vertices = 4) out;
			out vec2 postfx_fuv;
			void main() {
				float R = 1.0f;
				gl_Position = gl_in[0].gl_Position + vec4(-R,-R,0,0);
				postfx_fuv = vec2(0,0);
				EmitVertex();	
				gl_Position = gl_in[0].gl_Position + vec4(-R,+R,0,0);
				postfx_fuv = vec2(0,1);
				EmitVertex();	
				gl_Position = gl_in[0].gl_Position + vec4(+R,-R,0,0);
				postfx_fuv = vec2(1,0);
				EmitVertex();
				gl_Position = gl_in[0].gl_Position + vec4(+R,+R,0,0);
				postfx_fuv = vec2(1,1);
				EmitVertex();	
				EndPrimitive();
			}
		);
		std::string src_frag = PASTRY_GLSL(
			uniform sampler2D postfx_tex;
			uniform vec2 postfx_dim;
			in vec2 postfx_fuv;
			out vec4 postfx_outColor;
			vec4 sfx_read_fb(vec2 uv) {
				return texture(postfx_tex, uv);
			}
			vec4 sfx(vec2 uv);
			void main() {
				postfx_outColor = sfx(postfx_fuv);
			}
		);
		src_frag += "\n" + sfx_->source();

		//spo = pastry::load_program("assets/post"); // FIXME
		spo = pastry::program(src_vert, src_geom, src_frag);
		spo.get_uniform<int>("postfx_tex").set(0);
		u_dim = spo.get_uniform<Eigen::Vector2f>("postfx_dim");

		vao = pastry::vertex_array(spo, {{"pos", vbo}});
	}

	void update(float t, float dt) {
		sfx_->update(t, dt, spo);
	}

	void render(const texture& tex, unsigned w, unsigned h) {
		// render quad with texture
		spo.use();
		if(u_dim.is_valid())
			u_dim.set({w,h});
		pastry::texture::activate_unit(0);
		tex.bind();
		vbo.bind();
		vao.bind();
		glDrawArrays(GL_POINTS, 0, 1);
	}
};

class manager : public pastry::renderling
{
	buffer buff_a_, buff_b_;
	std::vector<effect> effects_;
	std::vector<post_effect_ptr> remove_sheduled_;

private:
	void remove_impl(const post_effect_ptr& p) {
		auto it = effects_.begin();
		while(it != effects_.end()) {
			it = std::find_if(it, effects_.end(),
				[&p](const effect& e) {
					return e.sfx_ == p;
				});
			if(it != effects_.end()) {
				effects_.erase(it);
			}
		}
	}

public:
	manager() {
		buff_a_.create();
		buff_b_.create();
	}

	void add(const effect& e) {
		effects_.push_back(e);
	}

	void remove(const post_effect_ptr& p) {
		remove_sheduled_.push_back(p);
	}

	void update(float t, float dt) {
		// remove sheduled
		for(const auto& p : remove_sheduled_) {
			remove_impl(p);
		}
		remove_sheduled_.clear();
		// skip if no effects
		if(effects_.empty()) {
			return;
		}
		// update effects
		for(effect& e : effects_) {
			e.update(t, dt);
		}
		// render to A
		if(effects_.size() > 1) {
			buff_b_.bind();
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
		buff_a_.bind();
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//std::cout << "Render ->A" << std::endl;
	}

	void render() {
		if(effects_.empty()) {
			return;
		}
		// render from A to B to A ... to screen
		bool source_is_a = true;
		for(int i=0; i<effects_.size()-1; ++i) {
			if(source_is_a) {
				buff_b_.bind();
			}
			else {
				buff_a_.bind();
			}
			//std::cout << "Render " << (source_is_a ? "A->B" : "B->A") << std::endl;
			effects_[i].render(source_is_a ? buff_a_.tex : buff_b_.tex, buff_a_.width_, buff_a_.height_);
			source_is_a = !source_is_a;
		}
		// now render to screen
		framebuffer::unbind();
		//std::cout << "Render " << (source_is_a ? "A->X" : "B->X") << std::endl;
		effects_.back().render(source_is_a ? buff_a_.tex : buff_b_.tex, buff_a_.width_, buff_a_.height_);
	}
};

}

std::shared_ptr<postfx::manager> g_postfx_manager;

void postfx_init()
{
	g_postfx_manager = std::make_shared<postfx::manager>();
	scene_add(g_postfx_manager, std::numeric_limits<int>::max());
}

class post_effect_stateless : public post_effect
{
public:
	std::string source_;
	func_postfx_update on_update_;
public:
	std::string source() const {
		return source_;
	}
	void update(float t, float dt, const program& spo) {
		if(on_update_)
			on_update_(t, dt, spo);
	}
};


post_effect_ptr postfx_add(const std::string& source)
{
	auto p = std::make_shared<post_effect_stateless>();
	p->source_ = source;
	postfx_add(p);
	return p;
}

post_effect_ptr postfx_add(const std::string& source, func_postfx_update f)
{
	auto p = std::make_shared<post_effect_stateless>();
	p->source_ = source;
	p->on_update_ = f;
	postfx_add(p);
	return p;
}

void postfx_add(const post_effect_ptr& p)
{
	g_postfx_manager->add(postfx::effect{p});
}

void postfx_remove(const post_effect_ptr& p)
{
	g_postfx_manager->remove(p);
}

}
