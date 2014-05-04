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
		typedef uint64_t glid_t;

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
			glid_t spos;
		};

		std::map<std::string,family> families_;
		std::map<std::string,species> species_;
		std::map<glid_t,instance> instances_;
		glid_t next_id_;

	private:
		void set_instance_data_impl(glid_t id, const std::vector<unsigned char>& v) {
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
			s.vao.set_layout(f.spo, vao_m.begin(), vao_m.end());
			s.num_instances = 0;
			s.dirty = true;
			species_[sname] = s;
		}

		void change_species_mesh(const std::string& sname, const multi_mesh& m) {
			// TODO is this ok?
			add_species(sname, species_[sname].family, m);
		}

		std::size_t add_instance(const std::string& sname) {
			species& s = species_[sname];
			const family& f = families_[s.family];
			instance x;
			x.species = sname;
			x.spos = s.num_instances++;
			s.data.insert(s.data.end(), f.instance_size, 0);
			glid_t id = next_id_++;
			instances_[id] = x;
			return id;
		}

		template<typename Inst> // Inst must be POD
		void set_instance_data(glid_t id, const Inst& data) {
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

	/** A 2D chunk voxel grid */
	template<typename T>
	class voxel_grid_2d
	{
	private:
		static constexpr uint32_t SHIFT = 4;
		static constexpr uint64_t SIZE = (1 << SHIFT);
		static constexpr uint64_t MAG = (1ull << 32);

		std::map<uint64_t, std::vector<T>> chunks_;

		T default_;

	public:
		void set_default(const T& t) {
			default_ = t;
		}

		const T& operator()(int x, int y) const {
			uint64_t gid, lid;
			pos_to_index(x, y, gid, lid);
			auto it = chunks_.find(gid);
			if(it == chunks_.end()) {
				auto q = chunks_.insert({gid, std::vector<T>(SIZE*SIZE, default_)});
				it = q.first;
			}
			return it->second[lid];
		}

		T& operator()(int x, int y) {
			uint64_t gid, lid;
			pos_to_index(x, y, gid, lid);
			auto it = chunks_.find(gid);
			if(it == chunks_.end()) {
				auto q = chunks_.insert({gid, std::vector<T>(SIZE*SIZE, default_)});
				it = q.first;
			}
			return it->second[lid];
		}

		bool exists(int x, int y) const {
			uint64_t gid, lid;
			pos_to_index(x, y, gid, lid);
			return chunks_.find(gid) != chunks_.end();
		}

		template<typename F>
		void foreach(F f) {
			for(auto& q : chunks_) {
				uint64_t gid = q.first;
				for(std::size_t lid=0; lid<SIZE*SIZE; lid++) {
					int32_t x, y;
					index_to_pos(gid, lid, x, y);
					f(x, y, q.second[lid]);
				}
			}
		}

	private:
		static void pos_to_index(int32_t x, int32_t y, uint64_t& gid, uint64_t& lid) {
			int32_t gx = (x >> SHIFT);
			int32_t gy = (y >> SHIFT);
			uint64_t ix = (gx < 0 ? (MAG + static_cast<int64_t>(gx)) : gx);
			uint64_t iy = (gy < 0 ? (MAG + static_cast<int64_t>(gy)) : gy);
			gid = ix + iy*MAG;
			uint64_t lx = x & (SIZE-1);
			uint64_t ly = y & (SIZE-1);
			lid = lx + ly*SIZE;
		}

		static void index_to_pos(uint64_t gid, uint64_t lid, int32_t& x, int32_t& y) {
			int32_t gx, gy;
			uint64_t ix = gid & (MAG - 1);
			uint64_t iy = gid / MAG;
			if(ix >= MAG/2) {
				int64_t gxl = ix - MAG;
				gx = -gxl;
			}
			if(iy >= MAG/2) {
				int64_t gyl = iy - MAG;
				gy = -gyl;
			}
			int32_t lx, ly;
			lx = lid & (SIZE - 1);
			ly = lid / SIZE;
			x = lx + SIZE * gx;
			y = ly + SIZE * gy;
			// std::cout << "gid=" << " " << gid << std::endl;
			// std::cout << "lid=" << " " << lid << std::endl;
			// std::cout << "ix=" << " " << ix << std::endl;
			// std::cout << "iy=" << " " << iy << std::endl;
			// std::cout << "gx=" << " " << gx << std::endl;
			// std::cout << "gy=" << " " << gy << std::endl;
			// std::cout << "lx=" << " " << lx << std::endl;
			// std::cout << "ly=" << " " << ly << std::endl;
			// std::cout << "x=" << " " << x << std::endl;
			// std::cout << "y=" << " " << y << std::endl;
		}
	};

}

#endif
