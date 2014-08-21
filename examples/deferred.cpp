
#include <pastry/pastry.hpp>
#include <pastry/gl.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

namespace obj
{
	struct VertexIndices
	{
		int v, uv, vn;
	};

	struct Face
	{
		VertexIndices a, b, c;
	};

	struct Mesh
	{
		std::string name;
		std::vector<Eigen::Vector3f> v;
		std::vector<Eigen::Vector2f> uv;
		std::vector<Eigen::Vector3f> vn;
		std::vector<Face> f;
	};

	VertexIndices ParseVertexIndices(const std::string& str)
	{
		static std::vector<std::string> tokens;
		boost::split(tokens, str, boost::is_any_of("/"));
		return VertexIndices{
			tokens[0].empty() ? 0 : boost::lexical_cast<int>(tokens[0]),
			tokens[1].empty() ? 0 : boost::lexical_cast<int>(tokens[1]),
			tokens[2].empty() ? 0 : boost::lexical_cast<int>(tokens[2])};
	}

	class Loader
	{
	public:
		Mesh Load(const std::string& fn)
		{
			auto it = cache_.find(fn);
			if(it == cache_.end()) {
				Mesh mesh = LoadImpl(fn);
				cache_[fn] = mesh;
				return mesh;
			}
			else {
				return it->second;
			}
		}

	private:
		Mesh LoadImpl(const std::string& fn) {
			std::ifstream ifs(fn);
			Mesh mesh;
			mesh.v.emplace_back(Eigen::Vector3f::Zero());
			mesh.uv.emplace_back(Eigen::Vector2f::Zero());
			mesh.vn.emplace_back(Eigen::Vector3f::Zero());
			std::string line;
			std::vector<std::string> tokens;
			std::vector<std::string> tokens2;
			while(std::getline(ifs, line)) {
				boost::split(tokens, line, boost::is_any_of(" "));
				const std::string& head = tokens[0];
				if(head == "#") {
					continue;
				}
				else if(head == "o") {
					mesh.name = tokens[1];
				}
				else if(head == "v") {
					mesh.v.emplace_back(
						boost::lexical_cast<float>(tokens[1]),
						boost::lexical_cast<float>(tokens[2]),
						boost::lexical_cast<float>(tokens[3]));
				}
				else if(head == "uv") {
					mesh.uv.emplace_back(
						boost::lexical_cast<float>(tokens[1]),
						boost::lexical_cast<float>(tokens[2]));
				}
				else if(head == "vn") {
					mesh.vn.emplace_back(
						boost::lexical_cast<float>(tokens[1]),
						boost::lexical_cast<float>(tokens[2]),
						boost::lexical_cast<float>(tokens[3]));
				}
				else if(head == "f") {
					mesh.f.push_back({
						ParseVertexIndices(tokens[1]),
						ParseVertexIndices(tokens[2]),
						ParseVertexIndices(tokens[3])});
				}
				else {
					std::cerr << "Unknown line in obj file starting with '" << head << "'" << std::endl;
				}
			}
			std::cout << "Mesh { "
				<< "name: " << mesh.name
				<< ", v: " << mesh.v.size()
				<< ", uv: " << mesh.uv.size()
				<< ", vn: " << mesh.vn.size()
				<< ", f: " << mesh.f.size()
				<< "}" << std::endl;
			return mesh;
		}

	private:
		std::map<std::string,Mesh> cache_;
	};

	Mesh Load(const std::string& fn)
	{
		static Loader s_loader;
		return s_loader.Load(fn);
	}

	struct Vertex
	{
		float vx, vy, vz;
		float u, v;
		float nx, ny, nz;
	};

	Vertex GetVertex(const Mesh& mesh, const VertexIndices& v)
	{
		return {
			mesh.v[v.v][0], mesh.v[v.v][1], mesh.v[v.v][2],
			mesh.uv[v.uv][0], mesh.uv[v.uv][1],
			mesh.vn[v.vn][0], mesh.vn[v.vn][1], mesh.vn[v.vn][2]
		};
	}

	std::vector<Vertex> GetData(const Mesh& mesh)
	{
		std::vector<Vertex> result;
		result.reserve(mesh.f.size() * 3 * 8);
		for(const auto& f : mesh.f) {
			result.push_back(GetVertex(mesh, f.a));
			result.push_back(GetVertex(mesh, f.c));
			result.push_back(GetVertex(mesh, f.b));
		}
		return result;
	}
}

Eigen::Matrix4f math_transform_3d(const Eigen::Vector3f& pos, float theta)
{
	float st = std::sin(theta);
	float ct = std::cos(theta);
	Eigen::Matrix4f m;
	m <<
		 ct, +st, 0, pos.x(),
		-st,  ct, 0, pos.y(),
		  0,   0, 1, pos.z(),
		  0,   0, 0, 1;
	return m;
}

struct Camera
{
	Eigen::Matrix4f projection;
	Eigen::Matrix4f view;
};

Camera g_camera;

class MeshObject
{
public:
	MeshObject(const std::string& fn_obj)
	: pose_(Eigen::Matrix4f::Identity())
	{
		mesh_ = pastry::single_mesh(GL_TRIANGLES);

		pastry::array_buffer vbo(
			{
				{"pos", GL_FLOAT, 3},
				{"uv", GL_FLOAT, 2},
				{"normal", GL_FLOAT, 3}
			},
			GL_STATIC_DRAW
		);
		mesh_.set_vertex_bo(vbo);

		obj::Mesh meshdata = obj::Load(fn_obj);
		mesh_.set_vertices(GetData(meshdata));

		sp_ = pastry::load_program("assets/deferred/render");

		va_ = pastry::vertex_array(sp_, {
			{"position", vbo, "pos"},
			{"texcoord", vbo, "uv"},
			{"normal", vbo, "normal"}
		});
		va_.bind();

	}

	void setPose(const Eigen::Matrix4f& pose)
	{ pose_ = pose; }

	void render()
	{
		sp_.use();
		sp_.get_uniform<Eigen::Matrix4f>("proj").set(g_camera.projection);
		sp_.get_uniform<Eigen::Matrix4f>("view").set(g_camera.view*pose_);
		va_.bind();
		mesh_.render();
	}

public:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

	Eigen::Matrix4f pose_;
};

class LightObject
{
public:
	LightObject()
	:	light_pos_(Eigen::Vector3f::Zero()),
		light_color_(Eigen::Vector3f::Ones()),
		falloff_(0.05f)
	{
		mesh_ = pastry::single_mesh(GL_TRIANGLES);

		pastry::array_buffer vbo(
			{ {"uv", GL_FLOAT, 2} },
			GL_STATIC_DRAW
		);
		mesh_.set_vertex_bo(vbo);

		std::vector<float> data = {
			0, 0,
			1, 0,
			1, 1,
			0, 0,
			1, 1,
			0, 1
		};
		mesh_.set_vertices(data);

		sp_ = pastry::load_program("assets/deferred/deferred");
		sp_.get_uniform<int>("texPosition").set(0);
		sp_.get_uniform<int>("texNormal").set(1);
		sp_.get_uniform<int>("texColor").set(2);

		va_ = pastry::vertex_array(sp_, {
			{"quv", vbo, "uv"}
		});
		va_.bind();
	}

	void setLightPosition(const Eigen::Vector3f& pos)
	{ light_pos_ = pos; }

	void setLightColor(const Eigen::Vector3f& color)
	{ light_color_ = color; }

	void setLightFalloff(float falloff)
	{ falloff_ = falloff; }

	void render()
	{
		sp_.use();

		Eigen::Vector4f lightpos4 = g_camera.view*Eigen::Vector4f(light_pos_[0],light_pos_[1],light_pos_[2],1);
		Eigen::Vector3f lightpos(lightpos4[0],lightpos4[1],lightpos4[2]);
		sp_.get_uniform<Eigen::Vector3f>("lightpos").set(lightpos);
		sp_.get_uniform<Eigen::Vector3f>("lightcol").set(light_color_);
		sp_.get_uniform<float>("lightfalloff").set(falloff_);

		va_.bind();
		mesh_.render();
	}

private:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

	Eigen::Vector3f light_pos_;
	Eigen::Vector3f light_color_;
	float falloff_;

};

class GBuffer
{
private:
	int width_, height_;
	pastry::texture_base tex_position, tex_normal, tex_color;
	pastry::texture_base tex_final;
	pastry::texture_base tex_depth;
	pastry::framebuffer fbo;

	int dbg_ = 0;

public:
	GBuffer()
	{
		pastry::fb_get_dimensions(width_, height_);
		fbo.bind();

		tex_position.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
		fbo.attach(GL_COLOR_ATTACHMENT0, tex_position);

		tex_normal.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
		fbo.attach(GL_COLOR_ATTACHMENT1, tex_normal);

		tex_color.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
		fbo.attach(GL_COLOR_ATTACHMENT2, tex_color);

		tex_final.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
		fbo.attach(GL_COLOR_ATTACHMENT3, tex_final);

		tex_depth.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		tex_depth.set_image_depth<float>(GL_DEPTH_COMPONENT32F, width_, height_);
		fbo.attach(GL_DEPTH_ATTACHMENT, tex_depth);

		fbo.unbind();
	}

	void startGeometryPass()
	{
		dbg_++;

		fbo.bind(pastry::framebuffer::target::WRITE);
		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, buffers);

		glEnable(GL_CULL_FACE);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void stopGeometryPass()
	{
		if(dbg_ == 50) {
			pastry::texture_save(tex_position, "/tmp/deferred_pos.png");
			pastry::texture_save(tex_normal, "/tmp/deferred_normal.png");
			pastry::texture_save(tex_color, "/tmp/deferred_color.png");
			pastry::texture_save(tex_depth, "/tmp/deferred_depth.png");
		}
	}

	void update()
	{
		if(pastry::fb_has_changed()) {
			pastry::fb_get_dimensions(width_, height_);
			fbo.bind();
			tex_position.bind();
			tex_position.set_image<float, 3>(GL_RGB32F, width_, height_);
			tex_normal.bind();
			tex_normal.set_image<float, 3>(GL_RGB32F, width_, height_);
			tex_color.bind();
			tex_color.set_image<float, 3>(GL_RGB32F, width_, height_);
			tex_final.bind();
			tex_final.set_image<float, 3>(GL_RGB32F, width_, height_);
			tex_depth.bind();
			tex_depth.set_image_depth<float>(GL_DEPTH_COMPONENT32F, width_, height_);
			fbo.unbind();
		}
	}

	void startLightPass()
	{
		fbo.bind();
		glDrawBuffer(GL_COLOR_ATTACHMENT3);

		glDisable(GL_CULL_FACE);

		glDepthMask(GL_FALSE);
		glDisable(GL_DEPTH_TEST);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		pastry::texture_base::activate_unit(0);
		tex_position.bind();		
		pastry::texture_base::activate_unit(1);
		tex_normal.bind();
		pastry::texture_base::activate_unit(2);
		tex_color.bind();

	}

	void stopLightPass()
	{
		glDisable(GL_BLEND);

		if(dbg_ == 50) {
			pastry::texture_save(tex_final, "/tmp/deferred_final.png");
		}
	}

	void finalPass()
	{
		fbo.unbind();
		fbo.bind(pastry::framebuffer::target::READ);
		glReadBuffer(GL_COLOR_ATTACHMENT3);
		glBlitFramebuffer(0, 0, width_, height_,
		                  0, 0, width_, height_, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
};

class DeferredRenderer
: public pastry::renderling
{
public:
	DeferredRenderer()
	{ }

	void add(const std::shared_ptr<MeshObject>& mesh)
	{ geometry_.push_back(mesh); }

	void add(const std::shared_ptr<LightObject>& light)
	{ lights_.push_back(light); }

	void update(float t, float dt)
	{

	}

	void render()
	{
		gbuff_.startGeometryPass();
		for(const auto& v : geometry_) {
			v->render();
		}
		gbuff_.stopGeometryPass();

		gbuff_.startLightPass();
		for(const auto& v : lights_) {
			v->render();
		}
		gbuff_.stopLightPass();

		gbuff_.finalPass();
	}

	GBuffer gbuff_;

	std::vector<std::shared_ptr<MeshObject>> geometry_;
	std::vector<std::shared_ptr<LightObject>> lights_;
};

Eigen::Vector3f HSL(float hue, float sat, float light)
{
	float c = (1.0f - std::abs(2.0f*light - 1.0f)) * sat;
	while(hue < 0) hue += 1;
	while(hue >= 1) hue -= 1;
	float h = hue * 6.0f;
	float x = c * (1.0f - std::abs(std::fmod(h, 2.0f) - 1.0f));
	float r,g,b;
	     if(h < 1) { r = c; g = x; b = 0; }
	else if(h < 2) { r = x; g = c; b = 0; }
	else if(h < 3) { r = 0; g = c; b = x; }
	else if(h < 4) { r = 0; g = x; b = c; }
	else if(h < 5) { r = x; g = 0; b = c; }
	else           { r = c; g = 0; b = x; }
	float m = light - 0.5f * c;
	return {r+m, g+m, b+m};
}

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	auto dr = std::make_shared<DeferredRenderer>();
	pastry::scene_add(dr);

	// camera
	{
		g_camera.projection = pastry::math_perspective_projection(90.0f/180.0f*3.1415f, 1.0f, 100.0f);
		g_camera.view = pastry::lookAt(3*Eigen::Vector3f{2,4,3},{0,0,0},{0,0,-1});
	}

	constexpr float SPACE = 3.0f;

	// geometry
	{
		constexpr int R = 3;
		for(int x=-R; x<=+R; x++) {
			for(int y=-R; y<=+R; y++) {
				auto geom = std::make_shared<MeshObject>("assets/suzanne.obj");
				Eigen::Affine3f pose = Eigen::Translation3f(Eigen::Vector3f(SPACE*x,SPACE*y,0))
					* Eigen::AngleAxisf(0.0f,Eigen::Vector3f{0,0,1});
				geom->setPose(pose.matrix());
				dr->add(geom);				
			}
		}
	}

	// lights
	// {
	// 	auto light = std::make_shared<LightObject>();
	// 	light->setLightPosition({+3,3,4});
	// 	dr->add(light);
	// }
	// {
	// 	auto light = std::make_shared<LightObject>();
	// 	light->setLightPosition({-4,1,4});
	// 	light->setLightColor({1,0,0});
	// 	dr->add(light);
	// }
	{
		constexpr int R = 3;
		for(int x=-R; x<=+R; x++) {
			for(int y=-R; y<=+R; y++) {
				auto light = std::make_shared<LightObject>();
				light->setLightPosition({SPACE*x,SPACE*y,1.5});
				light->setLightColor(HSL(std::atan2(y,x)/6.2831853f,1.0f,0.5f));
				light->setLightFalloff(1.65f);
				dr->add(light);
			}
		}
	}

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
