#pragma once

#include <pastry/deferred/Component.hpp>

namespace pastry {
namespace deferred {

class Script
:	public Component
{
public:
	virtual void initialize() {}
	virtual void update(float t, float dt) {}
};

}}
