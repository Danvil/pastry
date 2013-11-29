#ifndef INCLUDED_PASTRY_PASTRYGL_HPP
#define INCLUDED_PASTRY_PASTRYGL_HPP

#include <Eigen/Dense>
#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <memory>
#include <tuple>
#include <array>

#define PASTRY_GLSL(src) "#version 150\n" #src

namespace pastry
{
	typedef GLuint id_t;

	struct array_buffer_id {};
	struct vertex_shader_id {};
	struct fragment_shader_id {};
	struct program_id {};
	struct vertex_attribute_id {};
	struct vertex_array_id {};
	struct uniform_id {};
	struct texture_id {};

	namespace detail
	{
		template<typename ID>
		struct handler;

		template<> struct handler<array_buffer_id>
		{
			static id_t gl_create() { id_t id; glGenBuffers(1, &id); return id; }
			static void gl_delete(id_t id) { glDeleteBuffers(1, &id); }
		};

		template<> struct handler<vertex_shader_id>
		{
			static id_t gl_create() { return glCreateShader(GL_VERTEX_SHADER); }
			static void gl_delete(id_t id) { glDeleteShader(id); }
		};

		template<> struct handler<fragment_shader_id>
		{
			static id_t gl_create() { return glCreateShader(GL_FRAGMENT_SHADER); }
			static void gl_delete(id_t id) { glDeleteShader(id); }
		};

		template<> struct handler<program_id>
		{
			static id_t gl_create() { return glCreateProgram(); }
			static void gl_delete(id_t id) { glDeleteProgram(id); }
		};

		template<> struct handler<vertex_attribute_id>
		{
			static id_t gl_create() { return 0; }
			static void gl_delete(id_t) { }
		};

		template<> struct handler<vertex_array_id>
		{
			static id_t gl_create() { id_t id; glGenVertexArrays(1, &id); return id; }
			static void gl_delete(id_t id) { glDeleteVertexArrays(1, &id); }
		};

		template<> struct handler<uniform_id>
		{
			static id_t gl_create() { return 0; }
			static void gl_delete(id_t) { }
		};

		template<> struct handler<texture_id>
		{
			static id_t gl_create() { id_t id; glGenTextures(1, &id); return id; }
			static void gl_delete(id_t id) { glDeleteTextures(1, &id); }
		};

		constexpr id_t INVALID_ID = 0;

		template<typename T>
		struct resource_base
		{
		private:
			id_t id_;
		public:
			resource_base() {
				id_ = handler<T>::gl_create();
			}
			resource_base(id_t id) {
				id_ = id;
			}
			~resource_base() {
				handler<T>::gl_delete(id_);
			}
			id_t id() const { return id_; }
		};

		template<typename T>
		struct resource
		{
		private:
			std::shared_ptr<resource_base<T>> ptr_;
		public:
			typedef T id_type;
			resource() { }
			resource(id_t id) {
				id_set(id);
			}
			bool id_is_valid() const {
				return ptr_ && ptr_->id() != INVALID_ID;
			}
			void id_create_if_invalid() {
				if(!id_is_valid())
					id_create();
			}
			void id_set(id_t id) {
				ptr_ = std::make_shared<resource_base<T>>(id);
			}
			void id_create() {
				ptr_ = std::make_shared<resource_base<T>>();
			}
			void id_delete() {
				ptr_.reset();
			}
			operator id_t() const {
				return id();
			}
			id_t id() const {
				if(ptr_) {
					return ptr_->id();
				}
				std::cerr << "ERROR: Invalid resource ID!" << std::endl;
				return INVALID_ID;
			}
		};

	}

	namespace detail
	{
		inline void compile_shader(id_t q, const std::string& source) {
			GLint len = source.size();
			const GLchar* str = source.data();
			glShaderSource(q, 1, &str, &len);
			glCompileShader(q);

			GLint status;
			glGetShaderiv(q, GL_COMPILE_STATUS, &status);
			if(status != GL_TRUE) {
				char buffer[1024];
				glGetShaderInfoLog(q, 1024, NULL, buffer);
				std::cerr << "ERROR: " << buffer << std::endl;
			}
		}
	}

	struct vertex_shader : public detail::resource<vertex_shader_id>
	{
		vertex_shader() {}
		vertex_shader(const std::string& source) {
			id_create();
			compile(source);
		}
		void compile(const std::string& source) {
			detail::compile_shader(id(), source);
		}
	};

	struct fragment_shader : public detail::resource<fragment_shader_id>
	{
		fragment_shader() {}
		fragment_shader(const std::string& source) {
			id_create();
			compile(source);
		}
		void compile(const std::string& source) {
			detail::compile_shader(id(), source);
		}
	};

	namespace detail
	{
		#define PASTRY_UNIFORM_TYPES_IMPL_VAL(F,N) \
			F(float, N, glUniform##N##fv, glGetUniformfv) \
			F(int, N, glUniform##N##iv, glGetUniformiv) \
			F(unsigned int, N, glUniform##N##uiv, glGetUniformuiv)

		#define PASTRY_UNIFORM_TYPES_IMPL_MATN(F,N) \
			F(float, N, glUniformMatrix##N##fv, glGetUniformfv)

		#define PASTRY_UNIFORM_TYPES_IMPL_MATRC(F,R,C) \
			F(float, N, glUniformMatrix##R##x##C##fv, glGetUniformfv)

		template<typename K, unsigned int ROWS, unsigned int COLS, unsigned int NUM>
		struct uniform_impl;

		// K => glUniform1Kv
		// K[N] => glUniformNKv
		// Eigen::VectorNK => glUniformNKv

		#define PASTRY_UNIFORM_VAL_DEF(TYPE,N,SETVEC,GETVEC) \
			template<unsigned int NUM> \
			struct uniform_impl<TYPE,N,1,NUM> { \
				static void set(id_t loc, const TYPE* v) { SETVEC(loc, NUM, v); } \
				static void get(id_t prog, id_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
			};

		PASTRY_UNIFORM_TYPES_IMPL_VAL(PASTRY_UNIFORM_VAL_DEF,1)
		PASTRY_UNIFORM_TYPES_IMPL_VAL(PASTRY_UNIFORM_VAL_DEF,2)
		PASTRY_UNIFORM_TYPES_IMPL_VAL(PASTRY_UNIFORM_VAL_DEF,3)
		PASTRY_UNIFORM_TYPES_IMPL_VAL(PASTRY_UNIFORM_VAL_DEF,4)

		// Eigen::MatrixNf => glUniformMatrixNfv

		#define PASTRY_UNIFORM_MATN_DEF(TYPE,N,SETVEC,GETVEC) \
			template<unsigned int NUM> \
			struct uniform_impl<TYPE,N,N,NUM> { \
				static void set(id_t loc, const TYPE* v) { SETVEC(loc, NUM, GL_FALSE, v); } \
				static void get(id_t prog, id_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
			};

		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,2)
		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,3)
		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,4)

	}

/*
    |  - C++ -                      |  - GLSL -
	| uniform<float>				| float v
	| uniform<int,2>				| int v[2]
	| uniform<Eigen::Vector3f,4>	| vec3 v[4]
	| uniform<Eigen::Matrix4f,2>	| mat4 v[2]
*/

	/** Example: C++ int[2] / GLSL int[2]
	 * T: uniform type, must be float (GLSL: float), int (GLSL: int) or unsigned int (GLSL: uint)
	 * NUM>1: length of uniform array (for NUM=1 there is a specialization)
	 */
	template<typename T, unsigned int NUM>
	struct uniform
	: public detail::resource<uniform_id>
	{
		void set(std::initializer_list<T> values_list) {
			if(values_list.size() != NUM) {
				std::cerr << "ERROR in uniform::set: Wrong number of arrays!" << std::endl;
				return;
			}
			detail::uniform_impl<T,1,1,NUM>::set(id(), values_list.begin());
		}
		std::array<T,NUM> get(id_t prog_id) {
			std::array<T,NUM> a;
			detail::uniform_impl<T,1,1,NUM>::get(prog_id, id(), a.begin());
			return a;
		}
		void set_ptr(const T* v) {
			detail::uniform_impl<T,1,1,NUM>::set(id(), v);
		}
		void get_ptr(id_t prog_id, const T* v) {
			detail::uniform_impl<T,1,1,NUM>::get(prog_id, id(), v);
		}
	};

	/** Example: C++ int / GLSL int */
	template<typename T>
	struct uniform<T,1>
	: public detail::resource<uniform_id>
	{
		void set(const T& v) {
			detail::uniform_impl<T,1,1,1>::set(id(), &v);
		}
		T get(id_t prog_id) {
			T v;
			detail::uniform_impl<T,1,1,1>::get(prog_id, id(), &v);
			return v;
		}
	};

	/** Example: C++ Eigen::Vector3f / GLSL vec3 */
	template<typename K, int R, int C>
	struct uniform<Eigen::Matrix<K,R,C>,1>
	: public detail::resource<uniform_id>
	{
		typedef Eigen::Matrix<K,R,C> mat_t;
		void set(const mat_t& v) {
			detail::uniform_impl<K,R,C,1>::set(id(), v.data());
		}
		mat_t get(id_t prog_id) {
			mat_t v;
			detail::uniform_impl<K,R,C,1>::get(prog_id, id(), v.data());
			return v;
		}
	};

	/** Example: C++ Eigen::Vector3f[2] / GLSL vec3[2] */
	template<typename K, int R, int C, unsigned int NUM>
	struct uniform<Eigen::Matrix<K,R,C>,NUM>
	: public detail::resource<uniform_id>
	{
		typedef Eigen::Matrix<K,R,C> mat_t;
		void set(std::initializer_list<mat_t> values_list) {
			if(values_list.size() != NUM) {
				std::cerr << "ERROR in uniform::set: Wrong number of arrays!" << std::endl;
				return;
			}
			// copy to continuous memory
			K buff[R*C*NUM];
			for(unsigned int i=0; i<NUM; i++) {
				const K* p = values_list[i].data();
				std::copy(p, p+R*C, &buff[i*R*C]);
			}
			// write to opengl
			detail::uniform_impl<K,R,C,NUM>::set(id(), buff);
		}
		std::array<mat_t,NUM> get(id_t prog_id) {
			// read from opengl
			K buff[R*C*NUM];
			detail::uniform_impl<K,R,C,NUM>::get(prog_id, id(), buff);
			// create array
			std::array<mat_t,NUM> a;
			for(unsigned int i=0; i<NUM; i++) {
				const K* p = &buff[i*R*C];
				std::copy(p, p+R*C, a[i].data());
			}
			return a;
		}
	};

	namespace detail
	{
		struct va_data
		{
			std::string name;
			GLenum type;
			GLint size;
			std::size_t bytes_per_element;
			std::size_t bytes_total;
			std::size_t offset_begin;
			std::size_t offset_end;
		};

		struct layout_item {
			std::string name;
			GLenum type;
			int size;
		};

		inline std::size_t bytes_per_element(GLenum type) {
			switch(type) {
			case GL_BYTE: return 1;
			case GL_UNSIGNED_BYTE: return 1;
			case GL_SHORT: return 2;
			case GL_UNSIGNED_SHORT: return 2;
			case GL_INT: return 4;
			case GL_UNSIGNED_INT: return 4;
			case GL_FLOAT: return 4;
			case GL_DOUBLE: return 8;
			default: return 1;
			}
		}

		inline std::vector<va_data> va_conf(std::initializer_list<layout_item> list) {
			std::vector<va_data> q;
			q.reserve(list.size());
			for(const layout_item& i : list) {
				va_data dat;
				dat.name = i.name;
				dat.type = i.type;
				dat.size = i.size;
				dat.bytes_per_element = bytes_per_element(i.type);
				dat.bytes_total = dat.size * dat.bytes_per_element;
				dat.offset_begin = (q.empty() ? 0 : q.back().offset_end);
				dat.offset_end = dat.offset_begin + dat.bytes_total;
				q.push_back(dat);
			}
			return q;
		}
	}

	inline detail::layout_item layout_skip_bytes(int a) {
		return {"", 0, a};
	}

	struct vertex_attribute : public detail::resource<vertex_attribute_id>
	{
		vertex_attribute() {}
		void configure(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) {
			glVertexAttribPointer(id(), size, type, normalized, stride, pointer);
		}
		void enable() {
			glEnableVertexAttribArray(id());
		}
	};

	struct array_buffer : public detail::resource<array_buffer_id>
	{
	private:
		std::size_t num_bytes_;
		GLuint usage_;

	public:
		std::vector<detail::va_data> layout_;

		array_buffer()
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW) {}

		array_buffer(std::initializer_list<detail::layout_item> list)
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW) {
			id_create();
			bind();
			set_layout(list);
		}

		array_buffer(std::initializer_list<detail::layout_item> list, std::size_t num_bytes, GLuint usage)
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW) {
			id_create();
			bind();
			set_layout(list);
			init_data(num_bytes, usage);
		}

		void set_layout(std::initializer_list<detail::layout_item> list) {
			layout_ = detail::va_conf(list);
		}
		
		void bind() const {
			glBindBuffer(GL_ARRAY_BUFFER, id());
		}
		
		template<typename T>
		void init_data(const std::vector<T>& v, GLuint usage) {
			init_data(v.data(), v.size(), usage);
		}
		
		template<typename T>
		void init_data(const T* buf, std::size_t num_elements, GLuint usage) {
			init_data(reinterpret_cast<const void*>(buf), sizeof(T)*num_elements, usage);
		}
		
		void init_data(const void* buf, std::size_t num_bytes, GLuint usage) {
			usage_ = usage;
			num_bytes_ = num_bytes;
			bind();
			glBufferData(GL_ARRAY_BUFFER, num_bytes_, buf, usage);
		}

		void init_data(std::size_t num_bytes, GLuint usage) {
			init_data(NULL, num_bytes_, usage);
		}

		void init_data(GLuint usage) {
			init_data(NULL, 0, usage);
		}

		template<typename T>
		void update_data(const std::vector<T>& v) {
			update_data(v.data(), v.size());
		}
		
		template<typename T>
		void update_data(const T* buf, std::size_t num_elements) {
			update_data(reinterpret_cast<const void*>(buf), sizeof(T)*num_elements);
		}
		
		void update_data(const void* buf, std::size_t num_bytes) {
			if(num_bytes != num_bytes_) {
				init_data(buf, num_bytes, usage_);
			}
			else {
				bind();
				glBufferSubData(GL_ARRAY_BUFFER, 0, num_bytes, buf);
			}
		}

		static void unbind() {
			glBindBuffer(GL_ARRAY_BUFFER, detail::INVALID_ID);
		}
	};

	struct program : public detail::resource<program_id>
	{
		program() {}
		program(const std::string& vertex_source, const std::string& fragment_source) {
			id_create();
			create(
				vertex_shader(vertex_source),
				fragment_shader(fragment_source));
		}
		program(const vertex_shader& vs, const fragment_shader& fs) {
			id_create();
			create(vs, fs);
		}
		void attach(const vertex_shader& s) {
			glAttachShader(id(), s.id());
		}
		void attach(const fragment_shader& s) {
			glAttachShader(id(), s.id());
		}
		void link() {
			glLinkProgram(id());
		}
		void use() {
			glUseProgram(id());
		}
		void create(const vertex_shader& vs, const fragment_shader& fs) {
			attach(vs);
			attach(fs);
			link();
		}

		vertex_attribute get_attribute(const std::string& name) const {
			vertex_attribute va;
			GLint a = glGetAttribLocation(id(), name.data());
			if(a == -1) {
				std::cerr << "ERROR: Inactive or invalid vertex attribute '" << name << "'" << std::endl;
			}
			else {
				va.id_set(a);
			}
			return va;
		}

		template<typename T, unsigned int NUM=1>
		uniform<T,NUM> get_uniform(const std::string& name) {
			uniform<T,NUM> u;
			GLint a = glGetUniformLocation(id(), name.data());
			if(a == -1) {
				std::cerr << "ERROR: Inactive or invalid uniform name '" << name << "'" << std::endl;
			}
			else {
				u.id_set(a);
			}
			return u;
		}
	};

	namespace detail
	{
		struct mapping
		{
			std::string shader_name;
			array_buffer vb;
			std::string vb_name;
			mapping() {}
			mapping(const std::string& nsn, const array_buffer& nvb)
			: shader_name(nsn), vb(nvb), vb_name(nsn) {}
			mapping(const std::string& nsn, const array_buffer& nvb, const std::string& nvbn)
			: shader_name(nsn), vb(nvb), vb_name(nvbn) {}
		};
	}

	struct vertex_array : public detail::resource<vertex_array_id>
	{
		std::vector<vertex_attribute> attributes_;

		vertex_array() {}

		vertex_array(const program& p, std::initializer_list<detail::mapping> list) {
			id_create();
			bind();
			for(const detail::mapping& m : list) {
				const std::string& shader_name = m.shader_name;
				const array_buffer& vb = m.vb;
				const std::string& vn_name = m.vb_name;
				// find name in array
				auto it = std::find_if(vb.layout_.begin(), vb.layout_.end(),
					[&vn_name](const detail::va_data& x) { return x.name == vn_name; });
				if(it == vb.layout_.end()) {
					std::cerr << "ERROR: Could not find variable '" << vn_name << "' in buffer layout!" << std::endl;
					continue;
				}
				// need to bind array buffer to establish connection between shader and buffer
				vb.bind();
				// create vertex attribute
				vertex_attribute va = p.get_attribute(shader_name);
				va.configure(it->size, it->type, GL_FALSE, vb.layout_.back().offset_end, (GLvoid*)it->offset_begin);
				va.enable();
				attributes_.push_back(va);
			}
		}

		void bind() {
			glBindVertexArray(id());
		}

	};

	struct texture : public detail::resource<texture_id>
	{
		static constexpr GLenum target = GL_TEXTURE_2D;
		int width_;
		int height_;
		int channels_;
		texture() {}
		texture(int w, int h, float* data_rgb_f) {
			create();
			image_2d_rgb_f(w, h, data_rgb_f);
		}
		texture(int w, int h, unsigned char* data_rgb_ub) {
			create();
			image_2d_rgb_ub(w, h, data_rgb_ub);
		}
		void create() {
			id_create();
			bind();
			set_wrap(GL_REPEAT);
			set_filter(GL_LINEAR);
		}
		int width() const { return width_; }
		int height() const { return height_; }
		void bind() const {
			glBindTexture(target, id());
		}
		void set_wrap_s(GLint value) {
			glTexParameteri(target, GL_TEXTURE_WRAP_S, value);
		}
		void set_wrap_t(GLint value) {
			glTexParameteri(target, GL_TEXTURE_WRAP_T, value);
		}
		void set_wrap(GLint value) {
			set_wrap_s(value);
			set_wrap_t(value);
		}
		void set_border_color(float cr, float cg, float cb) {
			float color[] = {cr, cg, cb};
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
		}
		void set_min_filter(GLint value) {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, value);
		}
		void set_mag_filter(GLint value) {
			glTexParameteri(target, GL_TEXTURE_MAG_FILTER, value);
		}
		void set_filter(GLint value) {
			set_min_filter(value);
			set_mag_filter(value);
		}
		// void generate_mipmap() {
		// 	glGenerateMipmap(target);
		// }
		void image_2d_rgb_f(int w, int h, float* data_rgb_f) {
			width_ = w;
			height_ = h;
			channels_ = 3;
			glTexImage2D(target, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, data_rgb_f);
		}
		void image_2d_rgb_ub(int w, int h, unsigned char* data_rgb_ub) {
			width_ = w;
			height_ = h;
			channels_ = 3;
			glTexImage2D(target, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data_rgb_ub);
		}
		std::vector<unsigned char> get_image_rgb_ub() const {
			bind();
			std::vector<unsigned char> buff(width_*height_*3);
			glGetTexImage(target, 0, GL_RGB, GL_UNSIGNED_BYTE, buff.data());
			return buff;
		}
		std::vector<unsigned char> get_image_red_ub() const {
			bind();
			std::vector<unsigned char> buff(width_*height_*1);
			glGetTexImage(target, 0, GL_RED, GL_UNSIGNED_BYTE, buff.data());
			return buff;
		}
		int get_param_i(GLenum pname) const {
			bind();
			GLint val;
			glGetTexLevelParameteriv(target, 0, pname, &val);
			return val;
		}
		int get_width() const {
			return get_param_i(GL_TEXTURE_WIDTH);
		}
		int get_height() const {
			return get_param_i(GL_TEXTURE_HEIGHT);
		}
		static void activate_unit(unsigned int num) {
			glActiveTexture(GL_TEXTURE0 + num);
		}
	};

}
#endif
