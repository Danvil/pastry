#pragma once

#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/SkyBox.hpp>
#include <pastry/gl.hpp>
#include <Eigen/Dense>

namespace pastry {
namespace deferred {

class Light
{
public:
	virtual ~Light()
	{}

	virtual void render(const std::shared_ptr<Camera>& camera) = 0;

};

class EnvironmentLight
: public Light
{
public:
	EnvironmentLight();

	void setSkybox(const std::shared_ptr<SkyBox>& skybox)
	{ skybox_ = skybox; }

	void render(const std::shared_ptr<Camera>& camera);

private:
	pastry::program sp_;
	pastry::single_mesh mesh_;
	pastry::vertex_array va_;

	std::shared_ptr<SkyBox> skybox_;

};

class PointLight
: public Light
{
public:
	PointLight();

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
