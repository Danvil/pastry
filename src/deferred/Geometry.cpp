#include <pastry/deferred/Geometry.hpp>
#include <pastry/obj.hpp>

namespace pastry {
namespace deferred {

Geometry::Geometry()
:	pose_(Eigen::Matrix4f::Identity()),
	material_(0.5f, 0.0f, 0.5f)
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

	sp_ = pastry::load_program("assets/deferred/render");

	va_ = pastry::vertex_array(sp_, {
		{"position", vbo, "pos"},
		{"texcoord", vbo, "uv"},
		{"normal", vbo, "normal"}
	});
	va_.bind();

}

void Geometry::load(const std::string& fn_obj)
{
	mesh_.set_vertices(pastry::GetVertexData(pastry::LoadObjMesh(fn_obj)));
}

void Geometry::render()
{
	sp_.use();
	sp_.get_uniform<Eigen::Matrix4f>("proj").set(mainCamera()->projection());
	sp_.get_uniform<Eigen::Matrix4f>("view").set(mainCamera()->view()*pose_);
	sp_.get_uniform<Eigen::Vector3f>("material").set(material_);
	va_.bind();
	mesh_.render();
}

}}
