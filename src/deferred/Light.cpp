#include <pastry/deferred/Light.hpp>

namespace pastry {
namespace deferred {

Light::Light()
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

void Light::render(const std::shared_ptr<Camera>& camera)
{
	sp_.use();

	Eigen::Vector4f lightpos4 = camera->view*Eigen::Vector4f(light_pos_[0],light_pos_[1],light_pos_[2],1);
	Eigen::Vector3f lightpos(lightpos4[0],lightpos4[1],lightpos4[2]);
	sp_.get_uniform<Eigen::Vector3f>("lightpos").set(lightpos);
	sp_.get_uniform<Eigen::Vector3f>("lightcol").set(light_color_);
	sp_.get_uniform<float>("lightfalloff").set(falloff_);

	va_.bind();
	mesh_.render();
}



}}