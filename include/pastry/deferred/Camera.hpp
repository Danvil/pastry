#pragma once

#include <Eigen/Dense>

namespace pastry {
namespace deferred {

struct Camera
{
	Eigen::Matrix4f projection;
	Eigen::Matrix4f view;
};

}}
