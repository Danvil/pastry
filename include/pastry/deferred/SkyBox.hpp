#pragma once

#include <pastry/deferred/Camera.hpp>
#include <pastry/gl.hpp>

namespace pastry {
namespace deferred {

class SkyBox
{
public:
	SkyBox(const std::string& fn);

	void render(const std::shared_ptr<pastry::deferred::Camera>& camera);

	void bind()
	{ cm_.bind(); }

	texture_cube_map cm_;
	program sp_;
	single_mesh mesh_;
	vertex_array va_;

};

}}
