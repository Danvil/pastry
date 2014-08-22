#pragma once

#include <pastry/gl.hpp>
#include <memory>

namespace pastry {
namespace deferred {

class Camera;

class SkyBox
{
public:
	SkyBox(const std::string& fn);

	void render(const std::shared_ptr<pastry::deferred::Camera>& camera);

	void bind()
	{ cm_.bind(); }

	Eigen::Matrix3f cubeRotate() const;

	texture_cube_map cm_;
	program sp_;
	single_mesh mesh_;
	vertex_array va_;

};

}}
