#pragma once

#include <pastry/deferred/GBuffer.hpp>
#include <pastry/deferred/Camera.hpp>
#include <pastry/deferred/Light.hpp>
#include <pastry/deferred/Geometry.hpp>
#include <pastry/pastry.hpp>
#include <pastry/gl.hpp>

namespace pastry {
namespace deferred {

class DeferredRenderer
: public pastry::renderling
{
public:
	DeferredRenderer();

	void setCamera(const std::shared_ptr<pastry::deferred::Camera>& camera);

	void add(const std::shared_ptr<pastry::deferred::Geometry>& mesh);

	void add(const std::shared_ptr<pastry::deferred::Light>& light);

	void update(float t, float dt);

	void render();

	pastry::deferred::GBuffer gbuff_;

	std::shared_ptr<pastry::deferred::Camera> camera_;
	
	std::vector<std::shared_ptr<pastry::deferred::Geometry>> geometry_;
	std::vector<std::shared_ptr<pastry::deferred::Light>> lights_;
};

}}
