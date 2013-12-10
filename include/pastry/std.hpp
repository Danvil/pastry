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

	class instance_manager
	{
	private:
		typedef uint64_t id_t;

		/** Family defines type (shader, attributes) */
		struct family
		{
			pastry::program spo;
			instance_group_mapping mapping;
			std::size_t instance_size;
		};

		/** Species defines mesh */
		struct species
		{
			std::string family;
			pastry::vertex_array vao;
			multi_mesh mesh;
			std::vector<unsigned char> data;
			std::size_t num_instances;
			bool dirty;
		};

		/** Instances defines instance data */
		struct instance {
			std::string species;
			id_t spos;
		};

		std::map<std::string,family> families_;
		std::map<std::string,species> species_;
		std::map<id_t,instance> instances_;
		id_t next_id_;

	private:
		void set_instance_data_impl(id_t id, const std::vector<unsigned char>& v) {
			const instance& x = instances_[id];
			species& s = species_[x.species];
			const family& f = families_[s.family];
			s.data.insert(s.data.begin() + f.instance_size*x.spos,
				v.begin(), v.end());
			s.dirty = true;
		}

	public:
		void add_family(const std::string& fname, const program& spo, const instance_group_mapping& mapping) {
			family f;
			f.spo = spo;
			f.mapping = mapping;
			f.instance_size = detail::va_bytes_total(mapping.layout_instance);
			families_[fname] = f;
		}

		family& get_family(const std::string& fname) {
			return families_[fname];
		}

		void add_species(const std::string& sname, const std::string& fname, const multi_mesh& m) {
			const family& f = families_[fname];
			species s;
			s.family = fname;
			s.mesh = m;
			s.mesh.get_vertex_bo().set_layout(f.mapping.layout_vertex);
			s.mesh.get_instance_bo().set_layout(f.mapping.layout_instance);
			std::vector<detail::mapping> vao_m;
			for(const auto& x : f.mapping.mapping_vertex) {
				vao_m.push_back({ x[0], s.mesh.get_vertex_bo(), x[1]});
			}
			for(const auto& x : f.mapping.mapping_instance) {
				vao_m.push_back({ x[0], s.mesh.get_instance_bo(), x[1], 1});
			}
			s.vao = vertex_array(create_yes);
			s.vao.set_layout(f.spo, vao_m.begin(), vao_m.end());
			s.num_instances = 0;
			s.dirty = true;
			species_[sname] = s;
		}

		std::size_t add_instance(const std::string& sname) {
			species& s = species_[sname];
			const family& f = families_[s.family];
			instance x;
			x.species = sname;
			x.spos = s.num_instances++;
			s.data.insert(s.data.end(), f.instance_size, 0);
			id_t id = next_id_++;
			instances_[id] = x;
			return id;
		}

		template<typename Inst> // Inst must be POD
		void set_instance_data(id_t id, const Inst& data) {
			std::size_t n = sizeof(data);
			std::size_t n_check = families_[species_[instances_[id].species].family].instance_size;
			if(n != n_check) {
				std::cerr << "ERROR instance_manager::set_instance_data: Instance size does not match. ";
				std::cerr << "now=" << n << " - family=" << n_check << std::endl;
				return;
			}
			const unsigned char* p = reinterpret_cast<const unsigned char*>(&data);
			set_instance_data_impl(id, 
				std::vector<unsigned char>{p, p + sizeof(Inst)});
		}

		void render() {
			// update buffers
			for(auto& q : species_) {
				species& s = q.second;
				const family& f = families_[s.family];
				f.spo.use();
				s.vao.bind();
				if(s.dirty) {
					s.mesh.set_instances_raw(s.num_instances, s.data);
				}
				s.mesh.render();
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
