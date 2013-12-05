#ifndef INCLUDED_PASTRY_PASTRYGL_HPP
#define INCLUDED_PASTRY_PASTRYGL_HPP

#include <Eigen/Dense>
#include <GL/glew.h>
#include <GL/gl.h>
#include <algorithm>
#include <iostream>
#include <fstream>
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
	struct geometry_shader_id {};
	struct fragment_shader_id {};
	struct program_id {};
	struct vertex_attribute_id {};
	struct vertex_array_id {};
	struct uniform_id {};
	struct texture_id {};
	struct renderbuffer_id {};
	struct framebuffer_id {};

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

		template<> struct handler<geometry_shader_id>
		{
			static id_t gl_create() { return glCreateShader(GL_GEOMETRY_SHADER); }
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

		template<> struct handler<renderbuffer_id>
		{
			static id_t gl_create() { id_t id; glGenRenderbuffers(1, &id); return id; }
			static void gl_delete(id_t id) { glDeleteRenderbuffers(1, &id); }
		};

		template<> struct handler<framebuffer_id>
		{
			static id_t gl_create() { id_t id; glGenFramebuffers(1, &id); return id; }
			static void gl_delete(id_t id) { glDeleteFramebuffers(1, &id); }
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
		inline bool can_read_file(const std::string& filename) {
			std::ifstream in(filename, std::ios::in | std::ios::binary);
			return in;
		}

		inline std::string load_text_file(const std::string& filename) {
			// http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
			std::ifstream in(filename, std::ios::in | std::ios::binary);
			if(in) {
				std::string contents;
				in.seekg(0, std::ios::end);
				contents.resize(in.tellg());
				in.seekg(0, std::ios::beg);
				in.read(&contents[0], contents.size());
				in.close();
				return contents;
			}
			std::cerr << "ERROR in load_text_file: Could not open file '" << filename << "'" << std::endl;
			return "";
		}

		inline char int_to_char(unsigned i) {
			if(i == 0 || i > 9) return '0';
			return '1' + i - 1;
		}

		inline std::string int_to_string(unsigned i) {
			std::string r = "";
			while(i > 0) {
				r = int_to_char(i % 10) + r;
				i /= 10;
			}
			if(r.length() < 3) {
				r.insert(0, 3 - r.length(), '0');
			}
			return r;
		}

		inline void compile_shader(id_t q, std::string source) {
			// prepare by replacing "; " with ";\n"
			// this is to get better compiler error messages
			{
				size_t index = 0;
				while(true) {
					index = source.find("; ", index);
					if(index == std::string::npos) break;
					source.replace(index, 2, ";\n");
					index += 2;
				}
			}
			// compile
			GLint len = source.size();
			const GLchar* str = source.data();
			glShaderSource(q, 1, &str, &len);
			glCompileShader(q);
			// check
			GLint status;
			glGetShaderiv(q, GL_COMPILE_STATUS, &status);
			if(status != GL_TRUE) {
				// annotate code with line numbers
				{
					int i = 2; // first line ist done manually
					source = "001: " + source;
					size_t index = 0;
					while(true) {
						index = source.find("\n", index);
						if(index == std::string::npos) break;
						source.replace(index, 1, "\n" + int_to_string(i++) + ": ");
						index += 6;
					}
				}
				// get error message
				char buffer[1024];
				glGetShaderInfoLog(q, 1024, NULL, buffer);
				// print everything
				std::cerr << "ERROR: compile_shader" << std::endl;
				std::cerr << source << std::endl;
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

	struct geometry_shader : public detail::resource<geometry_shader_id>
	{
		geometry_shader() {}
		geometry_shader(const std::string& source) {
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

	template<typename T>
	T load_shader(const std::string& filename) {
		return T{detail::load_text_file(filename)};
	}

	namespace detail
	{
		template<typename K, unsigned int ROWS, unsigned int COLS, unsigned int NUM>
		struct uniform_impl {
			// static_assert(false,
			// 	"\n\n"
			// 	"ERROR with pastry::uniform / glUniform / glUniformMatrix:\n"
			// 	"\tInvalid row/column dimensions for OpenGL uniform.\n"
			// 	"\tSupported are: 1x1, 2x1, 3x1, 4x1, 2x2, 3x3, 4x4, 2x3, 3x2, 2x4, 4x2, 3x4, 4x3.\n");
		};

		// K => glUniform1Kv
		// K[N] => glUniformNKv
		// Eigen::VectorNK => glUniformNKv

		#define PASTRY_UNIFORM_TYPES_IMPL_VAL(F,N) \
			F(float, N, glUniform##N##fv, glGetUniformfv) \
			F(int, N, glUniform##N##iv, glGetUniformiv) \
			F(unsigned int, N, glUniform##N##uiv, glGetUniformuiv)

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

		#define PASTRY_UNIFORM_TYPES_IMPL_MATN(F,N) \
			F(float, N, glUniformMatrix##N##fv, glGetUniformfv)

		#define PASTRY_UNIFORM_MATN_DEF(TYPE,N,SETVEC,GETVEC) \
			template<unsigned int NUM> \
			struct uniform_impl<TYPE,N,N,NUM> { \
				static void set(id_t loc, const TYPE* v) { SETVEC(loc, NUM, GL_FALSE, v); } \
				static void get(id_t prog, id_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
			};

		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,2)
		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,3)
		PASTRY_UNIFORM_TYPES_IMPL_MATN(PASTRY_UNIFORM_MATN_DEF,4)

		// Eigen::MatrixNf => glUniformMatrixNfv

		#define PASTRY_UNIFORM_TYPES_IMPL_MATRC(F,R,C) \
			F(float, R,C, glUniformMatrix##R##x##C##fv, glGetUniformfv)

		#define PASTRY_UNIFORM_MATRC_DEF(TYPE,R,C,SETVEC,GETVEC) \
			template<unsigned int NUM> \
			struct uniform_impl<TYPE,R,C,NUM> { \
				static void set(id_t loc, const TYPE* v) { SETVEC(loc, NUM, GL_FALSE, v); } \
				static void get(id_t prog, id_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
			};

		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,2,3)
		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,3,2)
		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,2,4)
		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,4,2)
		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,3,4)
		PASTRY_UNIFORM_TYPES_IMPL_MATRC(PASTRY_UNIFORM_MATRC_DEF,4,3)

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

	template<typename K, unsigned int N>
	inline detail::layout_item layout_skip() {
		return layout_skip_bytes(sizeof(K)*N);
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
		program(const vertex_shader& vs, const fragment_shader& fs) {
			id_create();
			attach(vs);
			attach(fs);
			link();
		}
		program(const vertex_shader& vs, const geometry_shader& gs, const fragment_shader& fs) {
			id_create();
			attach(vs);
			attach(gs);
			attach(fs);
			link();
		}
		void attach(const vertex_shader& s) {
			glAttachShader(id(), s.id());
		}
		void attach(const geometry_shader& s) {
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

	inline program create_program(const std::string& src_vertex, const std::string& src_frag) {
		return program{ {src_vertex}, {src_frag} };
	}

	inline program create_program(const std::string& src_vertex, const std::string& src_geom, const std::string& src_frag) {
		return program{ {src_vertex}, {src_geom}, {src_frag} };
	}

	inline program load_program(const std::string& fn_vertex, const std::string& fn_frag) {
		return program{
			load_shader<vertex_shader>(fn_vertex),
			load_shader<fragment_shader>(fn_frag)
		};
	}

	inline program load_program(const std::string& fn_vertex, const std::string& fn_geom, const std::string& fn_frag) {
		return program{
			load_shader<vertex_shader>(fn_vertex),
			load_shader<geometry_shader>(fn_geom),
			load_shader<fragment_shader>(fn_frag)
		};
	}

	/** Tries to load a shader with files fn.vert, fn.geom, fn.frag */
	inline program load_program(const std::string& fn) {
		std::string fn_vert = fn + ".vert";
		std::string fn_geom = fn + ".geom";
		std::string fn_frag = fn + ".frag";
		if(detail::can_read_file(fn_geom)) {
			return load_program(fn_vert, fn_geom, fn_frag);
		}
		else {
			return load_program(fn_vert, fn_frag);
		}
	}

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
		void create(GLenum filter, GLenum wrap) {
			id_create();
			bind();
			set_filter(filter);
			set_wrap(wrap);
		}
		void create() {
			create(GL_LINEAR, GL_REPEAT);
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
		void image_2d_rgba_ub(int w, int h, unsigned char* data_rgb_ub) {
			width_ = w;
			height_ = h;
			channels_ = 3;
			glTexImage2D(target, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data_rgb_ub);
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
		static void unbind() {
			glBindTexture(target, detail::INVALID_ID);
		}
		static void activate_unit(unsigned int num) {
			glActiveTexture(GL_TEXTURE0 + num);
		}
	};

	struct renderbuffer : public detail::resource<renderbuffer_id>
	{		
		void bind() {
			glBindRenderbuffer(GL_RENDERBUFFER, id());
		}
		void storage(GLenum internalformat, unsigned width, unsigned height) {
			glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
		}
	};

	struct framebuffer : public detail::resource<framebuffer_id>
	{
		void bind() {
			glBindFramebuffer(GL_FRAMEBUFFER, id());
		}
		void attach(GLenum attachment, const texture& tex) {
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex.id(), 0);
		}
		void attach(GLenum attachment, const renderbuffer& rbo) {
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo.id());
		}
		static void unbind() {
			glBindFramebuffer(GL_FRAMEBUFFER, detail::INVALID_ID);
		}
	};

	/** Enables/disables an OpenGL capability like GL_BLEND and automatically restores the state
	 * Usage example:
	 *		{ capability(GL_BLEND,true);
	 *			// ... code to execute with blending enabled
	 * 		}
	 */
	struct capability
	{
		capability(GLenum cap, bool set_to) {
			cap_ = cap;
			was_enabled_ = glIsEnabled(cap_);
			set_to_ = set_to;
			if(was_enabled_ != set_to_) {
				if(set_to_) {
					glEnable(cap_);
				}
				else {
					glDisable(cap_);
				}
			}
		}
		~capability() {
			if(was_enabled_ != set_to_) {
				if(was_enabled_) {
					glEnable(cap_);
				}
				else {
					glDisable(cap_);
				}
			}
		}
	private:
		GLenum cap_;
		bool was_enabled_;
		bool set_to_;
	};

}
#endif
