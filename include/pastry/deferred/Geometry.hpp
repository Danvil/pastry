#pragma once

#include <pastry/deferred/Camera.hpp>
#include <pastry/gl.hpp>
#include <Eigen/Dense>
#include <string>

namespace pastry {
namespace deferred {

class Geometry
{
public:
	Geometry(const std::string& fn_obj);

	void setPose(const Eigen::Matrix4f& pose)
	{ pose_ = pose; }

	void setRoughness(float rough)
	{ material_[0] = rough; }

	void render(const std::shared_ptr<Camera>& camera);

public:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

	Eigen::Matrix4f pose_;
	Eigen::Vector3f material_; // roughness, metallic, specular)
};

}}
