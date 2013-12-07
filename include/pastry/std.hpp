#ifndef INCLUDED_PASTRY_DEF_HPP
#define INCLUDED_PASTRY_DEF_HPP

#include "gl.hpp"

namespace pastry {

	/** A group of instances sharing the same mesh data
	 */
	template<
		typename Mesh,
		typename InstanceData
	>
	class instance_group
	{
	private:
		program spo_;
		array_buffer vbo_mesh_;
		element_array_buffer ebo_mesh_;
		array_buffer vbo_inst_;
		vertex_array vao_;

		Mesh mesh_;
		bool is_mesh_changed_;

		std::vector<InstanceData> instances_;
		bool is_instances_changed_;
		
	public:
		instance_group() {}

		instance_group(
			const program& spo,
			std::initializer_list<detail::layout_item> layout_vertex,
			std::initializer_list<std::array<std::string,2>> mapping_vertex,
			std::initializer_list<detail::layout_item> layout_instance,
			std::initializer_list<std::array<std::string,2>> mapping_instance
		)
		: spo_(spo),
		  vbo_mesh_(layout_vertex),
		  ebo_mesh_(create_yes),
		  vbo_inst_(layout_instance),
		  vao_(create_yes),
		  is_instances_changed_(false),
		  is_mesh_changed_(false)
		{
			std::vector<detail::mapping> vao_m;
			for(const auto& x : mapping_vertex) {
				vao_m.push_back({ x[0], vbo_mesh_, x[1]});
			}
			for(const auto& x : mapping_instance) {
				vao_m.push_back({ x[0], vbo_inst_, x[1], 1});
			}
			vao_.set_layout(spo, vao_m.begin(), vao_m.end());
		}

		const program& get_program() const { return spo_; }

		void set_instances(const std::vector<InstanceData>& v) {
			instances_ = v;
			is_instances_changed_ = true;
		}

		void set_mesh(const Mesh& m) {
			mesh_ = m;
			is_mesh_changed_ = true;
		}

		void render() {
			if(is_instances_changed_) {
				vbo_inst_.update_data(instances_);
			}
			if(is_mesh_changed_) {
				vbo_mesh_.update_data(mesh_.vertices);
				ebo_mesh_.update_data(mesh_.indices);
			}
			else {
				ebo_mesh_.bind();
			}
			spo_.use();
			vao_.bind();
			mesh_.draw_elements_instanced(instances_.size());
		} 
	};

}

#endif
