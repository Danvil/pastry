#pragma once

#include <pastry/deferred/Forward.hpp>
#include <pastry/deferred/GBuffer.hpp>
#include <pastry/pastry.hpp>
#include <memory>

namespace pastry {
namespace deferred {

class DeferredRenderer
: public pastry::renderling
{
public:
	DeferredRenderer();

	void setScene(const std::shared_ptr<Scene>& scene);

	void update(float t, float dt);

	void render();

private:
	GBuffer gbuff_;

	std::shared_ptr<Scene> scene_;
};

}}
