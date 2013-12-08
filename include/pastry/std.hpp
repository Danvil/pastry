#ifndef INCLUDED_PASTRY_DEF_HPP
#define INCLUDED_PASTRY_DEF_HPP

#include "gl.hpp"

namespace pastry {

	struct instance_group_mapping
	{
		std::vector<detail::layout_item> layout_vertex;
		std::vector<std::array<std::string,2>> mapping_vertex;
		std::vector<detail::layout_item> layout_instance;
		std::vector<std::array<std::string,2>> mapping_instance;
	};

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
			const instance_group_mapping& igm
		)
		: spo_(spo),
		  vbo_mesh_(create_yes),
		  ebo_mesh_(create_yes),
		  vbo_inst_(create_yes),
		  vao_(create_yes),
		  is_mesh_changed_(false),
		  is_instances_changed_(false)
		{
			vbo_mesh_.set_layout(igm.layout_vertex);
			vbo_inst_.set_layout(igm.layout_instance);
			std::vector<detail::mapping> vao_m;
			for(const auto& x : igm.mapping_vertex) {
				vao_m.push_back({ x[0], vbo_mesh_, x[1]});
			}
			for(const auto& x : igm.mapping_instance) {
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
			spo_.use();
			vao_.bind();
			if(is_instances_changed_) {
				vbo_inst_.update_data(instances_);
				is_instances_changed_ = false;
			}
			if(is_mesh_changed_) {
				vbo_mesh_.update_data(mesh_.vertices);
				ebo_mesh_.update_data(mesh_.indices);
				is_mesh_changed_ = false;
			}
			else {
				ebo_mesh_.bind();
			}
			mesh_.draw_elements_instanced(instances_.size());
		} 
	};

	template<
		typename mesh_t,
		typename instance_t,
		typename group_id_t=std::string
	>
	class multi_instance_group
	{
	private:
		typedef instance_group<mesh_t,instance_t> group_t;
		typedef std::shared_ptr<group_t> p_group_t;
		typedef std::size_t instance_id_t;
		typedef std::pair<group_id_t,instance_t> instance_group_id_pair_t;
		typedef const group_id_t& cr_gid_t;
		std::map<group_id_t, p_group_t> groups_;
		program spo_;
		instance_group_mapping igm_;

	public:
		multi_instance_group(
			const program& spo,
			const instance_group_mapping& igm
		)
		: spo_(spo), igm_(igm)
		{}

		const program& get_program() const { return spo_; }

		void set_group(cr_gid_t gid, const mesh_t& mesh) {
			p_group_t g = std::shared_ptr<group_t>(new group_t(spo_, igm_));
			g->set_mesh(mesh);
			groups_[gid] = g;
		}

		void set_instances(const std::vector<instance_group_id_pair_t>& v) {
			std::map<group_id_t, std::vector<instance_t>> map;
			for(const auto& q : v) {
				map[q.first].push_back(q.second);
			}
			for(const auto& q : map) {
				const p_group_t& pg = groups_[q.first];
				if(!pg) {
					std::cerr << "ERROR multi_instance_group::set_instances: Unknown group id!" << std::endl;
					continue;
				}
				pg->set_instances(q.second);
			}
		}

		void render() {
			for(const auto& q : groups_) {
				q.second->render();
			}
		}
	};

	/** A 2D spatial data structure for fast access to neighbours
	 */
	template<typename T, unsigned NUM_BUCKETS>
	struct spatial_grid_2d
	{
	private:
		float x1_, y1_, x2_, y2_;
		float dx_inv_, dy_inv_;

		std::vector<T>* buckets_;

	public:
		static std::size_t v2i(float v, float v1, float dv_inv) {
			const int i = static_cast<int>((v-v1)*dv_inv);
			if(i < 0)
				return 0;
			else if(i >= NUM_BUCKETS)
				return NUM_BUCKETS-1;
			else 
				return i;
		}

		std::size_t x2i(float x) const {
			return v2i(x, x1_, dx_inv_);
		}

		std::size_t y2i(float y) const {
			return v2i(y, y1_, dy_inv_);
		}

		std::size_t p2i(float x, float y) const {
			return x2i(x) + NUM_BUCKETS*y2i(y);
		}

		spatial_grid_2d(float x1, float y1, float x2, float y2)
		: x1_(x1), y1_(y1), x2_(x2), y2_(y2) {
			dx_inv_ = 1.0f / ((x2_ - x1_) / static_cast<float>(NUM_BUCKETS));
			dy_inv_ = 1.0f / ((y2_ - y1_) / static_cast<float>(NUM_BUCKETS));
			buckets_ = new std::vector<T>[NUM_BUCKETS*NUM_BUCKETS];
		}

		~spatial_grid_2d() {
			delete[] buckets_;
		}

		void clear() {
			delete[] buckets_;
			buckets_ = new std::vector<T>(NUM_BUCKETS*NUM_BUCKETS);
		}

		void add(float x, float y, const T& v) {
			buckets_[p2i(x, y)].push_back(v);
		}

		template<typename F>
		void traverse(float px, float py, float r, F f) const {
			std::size_t ix = x2i(px);
			std::size_t iy = y2i(py);
			std::size_t irx = std::ceil(r * dx_inv_);
			std::size_t iry = std::ceil(r * dy_inv_);
			std::size_t x1 = (ix < irx ? 0 : ix - irx);
			std::size_t x2 = std::min<std::size_t>(NUM_BUCKETS-1, ix + irx);
			std::size_t y1 = (iy < iry ? 0 : iy - iry);
			std::size_t y2 = std::min<std::size_t>(NUM_BUCKETS-1, iy + iry);
			for(std::size_t y=y1; y<=y2; y++) {
				for(std::size_t x=x1; x<=x2; x++) {
					for(const T& v : buckets_[x+NUM_BUCKETS*y]) {
						f(v);
					}
				}
			}
		}
	};

}

#endif
