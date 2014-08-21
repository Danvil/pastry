
#include <pastry/pastry.hpp>
#include <pastry/gl.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

	Mesh Load(const std::string& fn)
	{
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

class MeshObject
: public pastry::renderling
{
public:
	MeshObject()
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

		obj::Mesh meshdata = obj::Load("assets/suzanne.obj");
		mesh_.set_vertices(GetData(meshdata));

		sp_ = pastry::load_program("assets/deferred/render");

		auto projmat = pastry::math_perspective_projection(90.0f/180.0f*3.1415f, 1.0f, 10.0f);
		//std::cout << projmat << std::endl;
		sp_.get_uniform<Eigen::Matrix4f>("proj").set(projmat);

		auto viewmat = pastry::lookAt({2,4,3},{0,0,0},{0,0,-1}).transpose();
		//std::cout << viewmat << std::endl;
		sp_.get_uniform<Eigen::Matrix4f>("view").set(viewmat);

		va_ = pastry::vertex_array(sp_, {
			{"position", vbo, "pos"},
			{"texcoord", vbo, "uv"},
			{"normal", vbo, "normal"}
		});
		va_.bind();

	}

	void update(float t, float dt)
	{


	}

	void render()
	{
		sp_.use();
		va_.bind();
		mesh_.render();
	}

public:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;
};

class GBuffer
{
private:
	int width_, height_;
	pastry::texture_base tex_position, tex_normal, tex_color;
	// pastry::texture_base tex_depth;
	pastry::framebuffer fbo;

	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

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

		// tex_depth.create(GL_LINEAR,GL_CLAMP_TO_EDGE);
		// tex_depth.set_image<float, 1>(GL_DEPTH_COMPONENT32F, width_, height_);
		// fbo.attach(GL_DEPTH_ATTACHMENT, tex_depth);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
		glDrawBuffers(3, buffers);

		fbo.unbind();

		// screen quad for rendering

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

		auto viewmat = pastry::lookAt({2,4,3},{0,0,0},{0,0,-1}).transpose();
		//std::cout << viewmat << std::endl;
		sp_.get_uniform<Eigen::Matrix4f>("view").set(viewmat);

		va_ = pastry::vertex_array(sp_, {
			{"quv", vbo, "uv"}
		});
		va_.bind();

	}

	void startGeometryPass()
	{
		fbo.bind(pastry::framebuffer::target::WRITE);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
			tex_color.set_image<unsigned char, 3>(GL_RGB8, width_, height_);
			// tex_depth.bind();
			// tex_depth.set_image<float, 1>(GL_DEPTH_COMPONENT32F, width_, height_);
			fbo.unbind();
		}
	}

	void runlightPass()
	{
		static unsigned i=0;
		i++;

		//fbo.bind(pastry::framebuffer::target::READ);
		//fbo.unbind(pastry::framebuffer::target::WRITE);
		fbo.unbind();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
//		glClearColor(1.0, 1.0, 0.0, 1.0);
//		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		pastry::texture_base::activate_unit(0);
		tex_position.bind();		
		pastry::texture_base::activate_unit(1);
		tex_normal.bind();
		pastry::texture_base::activate_unit(2);
		tex_color.bind();

		if(i == 100) {
			pastry::texture_save(tex_position, "/tmp/deferred_pos.png");
			pastry::texture_save(tex_normal, "/tmp/deferred_normal.png");
			pastry::texture_save(tex_color, "/tmp/deferred_color.png");
		}
		
		sp_.use();
		va_.bind();
		mesh_.render();
	}
};

class DeferredRenderer
: public pastry::renderling
{
public:
	DeferredRenderer()
	{
		std::cout << "Create G-Buffer" << std::endl;
	}

	void update(float t, float dt)
	{

	}

	void render()
	{
		gbuff.startGeometryPass();

		mesh.render();

		gbuff.runlightPass();
	}

	GBuffer gbuff;
	MeshObject mesh;
};

int main(void)
{
	std::cout << "Starting engine" << std::endl;

	pastry::initialize();

	auto dr = std::make_shared<DeferredRenderer>();
	pastry::scene_add(dr);

	std::cout << "Running main loop" << std::endl;

	pastry::run();

	std::cout << "Graceful quit" << std::endl;

	return 0;
}
