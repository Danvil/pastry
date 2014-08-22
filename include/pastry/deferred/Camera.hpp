#pragma once

#include <pastry/deferred/SkyBox.hpp>
#include <Eigen/Dense>
#include <memory>

namespace pastry {
namespace deferred {

class Camera
{
private:
	float angle_deg_, near_, far_;
	Eigen::Vector3f eye_, center_, up_;

	Eigen::Matrix4f projection_;
	Eigen::Matrix4f view_;

	std::shared_ptr<SkyBox> skybox_;

public:
	Camera();

	void setProjection(float angle_deg, float near, float far);

	void setView(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up);

	void setSkybox(const std::shared_ptr<SkyBox>& skybox)
	{ skybox_ = skybox; }

	const Eigen::Matrix4f& projection() const
	{ return projection_; }

	const Eigen::Matrix4f& view() const
	{ return view_; }

	const std::shared_ptr<SkyBox>& skybox() const
	{ return skybox_; }

	void update();

private:
	void udpateMatrices();

};

}}
