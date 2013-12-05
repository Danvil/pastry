
#include <pastry/pastry.hpp>
#include <random>
#include <memory>
#include <string>
#include <vector>

std::default_random_engine s_rnd;

class kitten
{
public:
	static constexpr float SIZE = 0.07f;
	std::shared_ptr<pastry::sprite> s;
	float gx, gy;
	kitten(float x, float y, const std::shared_ptr<pastry::sprite>& ns) {
		s = ns;
		s->x = x;
		s->y = y;
		s->sx = SIZE;
		s->sy = SIZE;
		gx = x;
		gy = y;
	}
	void update(float t, float dt) {
		std::uniform_real_distribution<float> uniform_dist(0, 512.0f);
 		float dx = s->x - gx;
		float dy = s->y - gy;
		float dist = std::sqrt(dx*dx + dy*dy);
		if(dist < 0.01f) {
			gx = uniform_dist(s_rnd);
			gy = uniform_dist(s_rnd);
		}
		s->x += (gx - s->x)*dt*0.34f;
		s->y += (gy - s->y)*dt*0.34f;
	}
};

class kitten_manager : public pastry::renderling
{
	pastry::sprite_group sprites_;
	std::vector<kitten> kittens_;
public:
	static constexpr int NUM = 100;
	kitten_manager() {
		pastry::sprites_add_sheet({
			"kitten",
			pastry::texture_load("assets/kitten.jpg"),
			{
				{"kitten", 0, 0, 1024, 786}
			}
		}, false);
		// create data
		for(int i=0; i<NUM; i++) {
			std::shared_ptr<pastry::sprite> s = sprites_.add_sprite("kitten");
			float phi = 2.0f * 3.1415f * (float)(i) / (float)(NUM);
			kitten k{256.0f+100.0f*std::cos(phi), 256.0f+100.0f*std::sin(phi), s};
			kittens_.push_back(k);
		}
	}
	void update(float t, float dt) {
		for(kitten& k : kittens_) {
			k.update(t,dt);
		}
		sprites_.update(t,dt);
	}
	void render() {
		sprites_.render();
	}
};

class post_processor : public pastry::renderling
{
private:
	pastry::program spo;
	pastry::array_buffer vbo;
	pastry::vertex_array vao;
	pastry::texture tex;
	pastry::renderbuffer rbo;
	pastry::framebuffer fbo;
	int width_, height_;
public:
	post_processor() {
		vbo = pastry::array_buffer({
			{"pos", GL_FLOAT, 2}
		});
//		vbo.init_data(std::vector<float>{1,1.0f}, GL_STATIC_DRAW);
		vbo.init_data(std::vector<float>{0.0f,0.0f}, GL_STATIC_DRAW);

		spo = pastry::load_program("assets/post");
		spo.get_uniform<int>("tex").set(0);

		vao = pastry::vertex_array(spo, {
			{"pos", vbo}//,
			//{"dummy", vbo, "pos"}
		});

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

	}
	void update(float t, float dt) {
		if(pastry::fb_has_changed()) {
			pastry::fb_get_dimensions(width_, height_);
			tex.bind();
			tex.image_2d_rgba_ub(width_, height_, 0);
			rbo.bind();
			rbo.storage(GL_DEPTH_COMPONENT16, width_, height_);
		}
		fbo.bind();
		fbo.unbind();
	}
	void render() {
		fbo.unbind();
		spo.use();
		tex.bind();
		glDrawArrays(GL_POINTS, 0, 1);
		fbo.bind();
	}
};

int main()
{
	pastry::initialize();

//	pastry::scene_add(std::make_shared<kitten_manager>());

	pastry::scene_add(std::make_shared<post_processor>());

	pastry::run();

	return 0;
}
