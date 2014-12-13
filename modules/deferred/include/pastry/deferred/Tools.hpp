#pragma once

#include <algorithm>
#include <memory>

namespace pastry {
namespace deferred {

template<typename C>
bool IsOfType(const std::shared_ptr<Component>& c)
{
	return std::dynamic_pointer_cast<C>(c).get() != nullptr;
}

template<typename T>
void UniqueAdd(std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& x)
{
	if(!x) return;
	auto it = std::find(v.begin(), v.end(), x);
	if(it == v.end()) {
		v.push_back(x);
	}
}

template<typename T>
void UniqueRemove(std::vector<std::shared_ptr<T>>& v, const std::shared_ptr<T>& x)
{
	if(!x) return;
	auto it = std::find(v.begin(), v.end(), x);
	if(it != v.end()) {
		v.erase(it);
	}
}

}}
