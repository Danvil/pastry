#pragma once

#include <pastry/deferred/Component.hpp>
#include <pastry/deferred/SkyBox.hpp>
#include <Eigen/Dense>
#include <memory>

namespace pastry {
namespace deferred {

class Camera
:	public Component
{
private:
	float angle_deg_, near_, far_;
	Eigen::Vector3f eye_, center_, up_;

	Eigen::Matrix4f projection_;
	Eigen::Matrix4f view_;

public:
	Camera();

	void setProjection(float angle_deg, float near, float far);

	void setView(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up);

	const Eigen::Matrix4f& projection() const
	{ return projection_; }

	const Eigen::Matrix4f& view() const
	{ return view_; }

	void update();

private:
	void udpateMatrices();

};

}}
