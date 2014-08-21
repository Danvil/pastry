#pragma once

#include <pastry/deferred/Camera.hpp>
#include <pastry/gl.hpp>
#include <Eigen/Dense>

namespace pastry {
namespace deferred {

class Light
{
public:
	Light();

	void setLightPosition(const Eigen::Vector3f& pos)
	{ light_pos_ = pos; }

	void setLightColor(const Eigen::Vector3f& color)
	{ light_color_ = color; }

	void setLightFalloff(float falloff)
	{ falloff_ = falloff; }

	void render(const std::shared_ptr<Camera>& camera);

private:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

	Eigen::Vector3f light_pos_;
	Eigen::Vector3f light_color_;
	float falloff_;

};

}}
