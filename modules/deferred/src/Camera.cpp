#include <pastry/deferred/Camera.hpp>
#include <pastry/pastry.hpp>

namespace pastry {
namespace deferred {

Camera::Camera()
:	angle_deg_(90.0f), near_(1.0f), far_(100.0f),
	eye_(1,2,3), center_(0,0,0), up_(0,0,1)
{
	udpateMatrices();
}

void Camera::setProjection(float angle_deg, float near, float far)
{
	angle_deg_ = angle_deg;
	near_ = near;
	far_ = far;
}

void Camera::setView(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up)
{
	eye_ = eye;
	center_ = center;
	up_ = up;
}

void Camera::update()
{
	udpateMatrices();
}

void Camera::udpateMatrices()
{
	projection_ = pastry::math_perspective_projection(angle_deg_/180.0f*3.1415f, pastry::fb_get_aspect(), near_, far_);
	view_ = pastry::lookAt(eye_, center_, up_);
}

}}
