#include <pastry/deferred/Geometry.hpp>
#include <pastry/obj.hpp>

namespace pastry {
namespace deferred {

Geometry::Geometry(const std::string& fn_obj)
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
	mesh_.set_vertices(pastry::GetVertexData(pastry::LoadObjMesh(fn_obj)));

	sp_ = pastry::load_program("assets/deferred/render");

	va_ = pastry::vertex_array(sp_, {
		{"position", vbo, "pos"},
		{"texcoord", vbo, "uv"},
		{"normal", vbo, "normal"}
	});
	va_.bind();

}

void Geometry::render(const std::shared_ptr<Camera>& camera)
{
	sp_.use();
	sp_.get_uniform<Eigen::Matrix4f>("proj").set(camera->projection());
	sp_.get_uniform<Eigen::Matrix4f>("view").set(camera->view()*pose_);
	va_.bind();
	mesh_.render();
}

}}
