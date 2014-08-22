#include <pastry/deferred/Light.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/SkyBox.hpp>

namespace pastry {
namespace deferred {

EnvironmentLight::EnvironmentLight()
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

	sp_ = pastry::load_program("assets/deferred/envlight");
	sp_.get_uniform<int>("texPosition").set(0);
	sp_.get_uniform<int>("texNormal").set(1);
	sp_.get_uniform<int>("texColor").set(2);
	sp_.get_uniform<int>("texMaterial").set(3);
	sp_.get_uniform<int>("gCubemapTexture").set(4);

	va_ = pastry::vertex_array(sp_, {
		{"quv", vbo, "uv"}
	});
	va_.bind();
}

void EnvironmentLight::render(const std::shared_ptr<Camera>& camera)
{
	pastry::texture_cube_map::activate_unit(4);
	camera->skybox()->bind();

	sp_.use();
	sp_.get_uniform<Eigen::Matrix3f>("cuberot").set(camera->skybox()->cubeRotate());

	va_.bind();
	mesh_.render();
}


PointLight::PointLight()
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

	sp_ = pastry::load_program("assets/deferred/pointlight");
	sp_.get_uniform<int>("texPosition").set(0);
	sp_.get_uniform<int>("texNormal").set(1);
	sp_.get_uniform<int>("texColor").set(2);
	sp_.get_uniform<int>("texMaterial").set(3);

	va_ = pastry::vertex_array(sp_, {
		{"quv", vbo, "uv"}
	});
	va_.bind();
}

void PointLight::render(const std::shared_ptr<Camera>& camera)
{
	sp_.use();

	Eigen::Vector4f lightpos4 = camera->view()*Eigen::Vector4f(light_pos_[0],light_pos_[1],light_pos_[2],1);
	Eigen::Vector3f lightpos(lightpos4[0],lightpos4[1],lightpos4[2]);
	sp_.get_uniform<Eigen::Vector3f>("lightpos").set(lightpos);
	sp_.get_uniform<Eigen::Vector3f>("lightcol").set(light_color_);
	sp_.get_uniform<float>("lightfalloff").set(falloff_);

	va_.bind();
	mesh_.render();
}


}}
