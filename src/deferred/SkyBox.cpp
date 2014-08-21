#include <pastry/deferred/SkyBox.hpp>
#include <pastry/obj.hpp>
#include <pastry/pastry.hpp>

namespace pastry {
namespace deferred {

SkyBox::SkyBox(const std::string& fn)
{
	texture_2d tex = texture_load(fn);
	std::vector<unsigned char> data = tex.get_image<unsigned char>();
	unsigned width = tex.width();
	unsigned height = tex.height();
	std::cout << "skybox " << width << "x" << height << ", size=" << data.size() << std::endl;
	cm_.create();

	for(unsigned k=0; k<6; k++) {
		std::vector<unsigned char> sub(width/6*height*3);
		for(unsigned i=0; i<height; i++) {
			const unsigned char* src = data.data() + i*3*width + k*3*width/6;
			std::copy(src, src + 3*width/6, sub.data() + i*3*width/6);
		}
		cm_.set_image<unsigned char, 3>(cm_.cube_map_type(k), GL_RGB8, width/6, height, sub.data());
	}

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
	mesh_.set_vertices(pastry::GetVertexData(pastry::LoadObjMesh("assets/box.obj"), 5.0f, true));

	sp_ = pastry::load_program("assets/skybox");
	sp_.get_uniform<int>("gCubemapTexture").set(0);

	va_ = pastry::vertex_array(sp_, {
		{"Position", vbo, "pos"}
	});
	va_.bind();
}

void SkyBox::render(const std::shared_ptr<pastry::deferred::Camera>& camera)
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	pastry::texture_cube_map::activate_unit(0);
	cm_.bind();

	sp_.use();
	Eigen::Affine3f pose = Eigen::Translation3f(Eigen::Vector3f{0,0,0})
		* Eigen::AngleAxisf(-1.570796327f, Eigen::Vector3f{1,0,0});
	sp_.get_uniform<Eigen::Matrix4f>("gWVP").set(camera->projection*camera->view*pose.matrix());

	va_.bind();
	mesh_.render();

	glCullFace(GL_BACK);
}

}}
