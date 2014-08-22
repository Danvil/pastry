#pragma once

#include <pastry/deferred/Component.hpp>
#include <pastry/gl.hpp>
#include <string>
#include <memory>

namespace pastry {
namespace deferred {

class Camera;

class SkyBox
:	public Component
{
public:
	SkyBox();

	void setTexture(const std::string& fn_tex);

	void setGeometry(const std::string& fn_obj);

	void render();

	void bind()
	{ cm_.bind(); }

	Eigen::Matrix3f cubeRotate() const;

	texture_cube_map cm_;
	program sp_;
	single_mesh mesh_;
	vertex_array va_;

};

}}
