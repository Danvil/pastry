#include <pastry/deferred/SkyBox.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/obj.hpp>
#include <pastry/pastry.hpp>
#include <slimage/image.hpp>
#include <slimage/algorithm.hpp>

namespace pastry {
namespace deferred {

slimage::Image3ub Smooth(const slimage::Image3ub& v)
{
	slimage::Image3ub result(v.width()/2, v.height()/2);
	for(unsigned y=0; y<result.height(); y++) {
		for(unsigned x=0; x<result.width(); x++) {
			slimage::Pixel3ub px0 = v(2*x  ,2*y  );
			slimage::Pixel3ub px1 = v(2*x+1,2*y  );
			slimage::Pixel3ub px2 = v(2*x  ,2*y+1);
			slimage::Pixel3ub px3 = v(2*x+1,2*y+1);
			auto dst = result(x,y);
			for(unsigned k=0; k<3; k++) {
				dst[k] = static_cast<unsigned char>(
					(static_cast<unsigned>(px0[k]) + static_cast<unsigned>(px1[k]) + static_cast<unsigned>(px2[k]) + static_cast<unsigned>(px3[k])) / 4);
			}
		}
	}
	return result;
}

SkyBox::SkyBox(const std::string& fn_tex, const std::string& fn_obj)
{
	texture_2d tex = texture_load(fn_tex);
	std::vector<unsigned char> data = tex.get_image<unsigned char>();
	unsigned width = tex.width();
	unsigned height = tex.height();
	std::cout << "skybox " << width << "x" << height << ", size=" << data.size() << std::endl;
	
	slimage::Image3ub big(width, height);
	std::copy(data.begin(), data.end(), big.pixel_pointer());

	cm_.create();
	cm_.set_filter(GL_LINEAR_MIPMAP_LINEAR);
	for(unsigned k=0; k<6; k++) {
		std::cout << "skybox side " << k << std::endl;
		auto sub = slimage::SubImage(big, k*width/6, 0, width/6, height);
		for(unsigned mm=0; (1<<mm) <= width && (1<<mm) <= height; mm++) {
			std::cout << "skybox mipmap " << mm << ": " << sub.width() << "x" << sub.height() << std::endl;
			cm_.set_image_mm<unsigned char, 3>(cm_.cube_map_type(k), GL_RGB8, mm, sub.width(), sub.height(), sub.pixel_pointer());
			sub = Smooth(sub);
		}
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
	mesh_.set_vertices(pastry::GetVertexData(pastry::LoadObjMesh(fn_obj), 15.0f, true));

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
	sp_.get_uniform<Eigen::Matrix4f>("gWVP").set(camera->projection()*camera->view()*pose.matrix());

	va_.bind();
	mesh_.render();

	glCullFace(GL_BACK);
}

Eigen::Matrix3f SkyBox::cubeRotate() const
{
	Eigen::Affine3f pose = Eigen::Translation3f(Eigen::Vector3f{0,0,0})
		* Eigen::AngleAxisf(-1.570796327f, Eigen::Vector3f{1,0,0});
	return pose.matrix().block<3,3>(0,0);
}


}}
