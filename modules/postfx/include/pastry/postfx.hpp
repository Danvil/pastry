#pragma once

#include <pastry/pastry.hpp>

namespace pastry {

class post_effect
{
public:
	virtual std::string source() const { return "vec4 sfx(vec2 uv) { return sfx_read_fb(uv); }"; }
	virtual void update(float t, float dt, const program& spo) {}
};

PASTRY_DEFINE_PTR(post_effect)

void postfx_initialize();

post_effect_ptr postfx_add(const std::string& source);

typedef std::function<void(float,float,const pastry::program& spo)> func_postfx_update;

post_effect_ptr postfx_add(const std::string& source, func_postfx_update f);

void postfx_add(const post_effect_ptr& p);

void postfx_remove(const post_effect_ptr& p);

}
