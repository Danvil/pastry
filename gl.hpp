// Copyright (c) 2014 David Weikersdorfer

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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

namespace danvil {
namespace pastry
{
	typedef GLuint glid_t;

	enum class rid
	{
		buffer,
		vertex_shader,
		geometry_shader,
		fragment_shader,
		program,
		vertex_array,
		texture_base,
		renderbuffer,
		framebuffer
	};

	namespace detail
	{
		inline const char* name(rid r)
		{
			#define PASTRY_RESOURCE_NAME(T) case rid::T: return #T;
			switch(r) {
			PASTRY_RESOURCE_NAME(buffer)
			PASTRY_RESOURCE_NAME(vertex_shader)
			PASTRY_RESOURCE_NAME(geometry_shader)
			PASTRY_RESOURCE_NAME(fragment_shader)
			PASTRY_RESOURCE_NAME(program)
			PASTRY_RESOURCE_NAME(vertex_array)
			PASTRY_RESOURCE_NAME(texture_base)
			PASTRY_RESOURCE_NAME(renderbuffer)
			PASTRY_RESOURCE_NAME(framebuffer)
			}
			#undef PASTRY_RESOURCE_NAME
		}
	}

	inline std::ostream& operator<<(std::ostream& os, rid r)
	{ os << detail::name(r); return os; }

	namespace detail
	{
		template<rid id> struct handler;

		template<> struct handler<rid::buffer>
		{
			static glid_t gl_create() { glid_t id; glGenBuffers(1, &id); return id; }
			static void gl_delete(glid_t id) { glDeleteBuffers(1, &id); }
		};

		template<> struct handler<rid::vertex_shader>
		{
			static glid_t gl_create() { return glCreateShader(GL_VERTEX_SHADER); }
			static void gl_delete(glid_t id) { glDeleteShader(id); }
		};

		template<> struct handler<rid::geometry_shader>
		{
			static glid_t gl_create() { return glCreateShader(GL_GEOMETRY_SHADER); }
			static void gl_delete(glid_t id) { glDeleteShader(id); }
		};

		template<> struct handler<rid::fragment_shader>
		{
			static glid_t gl_create() { return glCreateShader(GL_FRAGMENT_SHADER); }
			static void gl_delete(glid_t id) { glDeleteShader(id); }
		};

		template<> struct handler<rid::program>
		{
			static glid_t gl_create() { return glCreateProgram(); }
			static void gl_delete(glid_t id) { glDeleteProgram(id); }
		};

		template<> struct handler<rid::vertex_array>
		{
			static glid_t gl_create() { glid_t id; glGenVertexArrays(1, &id); return id; }
			static void gl_delete(glid_t id) { glDeleteVertexArrays(1, &id); }
		};

		template<> struct handler<rid::texture_base>
		{
			static glid_t gl_create() { glid_t id; glGenTextures(1, &id); return id; }
			static void gl_delete(glid_t id) { glDeleteTextures(1, &id); }
		};

		template<> struct handler<rid::renderbuffer>
		{
			static glid_t gl_create() { glid_t id; glGenRenderbuffers(1, &id); return id; }
			static void gl_delete(glid_t id) { glDeleteRenderbuffers(1, &id); }
		};

		template<> struct handler<rid::framebuffer>
		{
			static glid_t gl_create() { glid_t id; glGenFramebuffers(1, &id); return id; }
			static void gl_delete(glid_t id) { glDeleteFramebuffers(1, &id); }
		};

		constexpr glid_t INVALID_ID = 0;

		template<rid R>
		class resource_base
		{
		private:
			glid_t id_;
		
		public:
			resource_base()
			: id_(handler<R>::gl_create()) {}
			
			resource_base(glid_t id)
			: id_(id) {}
			
			resource_base(const resource_base&) = delete;
			resource_base& operator=(const resource_base&) = delete;
			
			~resource_base()
			{ handler<R>::gl_delete(id_); }
			
			glid_t id() const
			{ return id_; }
		};

		// class ressource_not_initialized : public std::runtime_error
		// {
		// public:
		// 	ressource_not_initialized(rid r)
		// 	: std::runtime_error(
		// 		(std::string("pastry: resource of type ") + detail::name(r) + " is not initialized!").c_str()) {}
		// };

		template<rid R>
		struct resource
		{
		private:
			std::shared_ptr<resource_base<R>> ptr_;
		
		public:
			resource()
			: ptr_(std::make_shared<resource_base<R>>()) {}

			resource(glid_t id)
			: ptr_(std::make_shared<resource_base<R>>(id)) {}

			glid_t id() const
			{ return ptr_->id(); }
			
			operator glid_t() const
			{ return ptr_->id(); }
		};
	}

	struct exception
	: public std::runtime_error
	{
		exception(const std::string& msg)
		: std::runtime_error(msg.c_str())
		{
			std::cerr << "--------------------------------------------------------------------------------" << std::endl;
			std::cerr << "--------  PASTRY ERROR  --------------------------------------------------------" << std::endl;
			std::cerr << "--------------------------------------------------------------------------------" << std::endl;
			std::cerr << msg << std::endl;
			std::cerr << "--------------------------------------------------------------------------------" << std::endl;
		}
	};

	struct invalid_shader_source
	: public exception
	{
		invalid_shader_source(const std::string& source)
		: exception(std::string("pastry: invalid shader source:\n") + source)
		{ }
	};

	struct invalid_shader_program
	: public exception
	{
		invalid_shader_program()
		: exception("pastry: invalide shader program")
		{ }
	};

	struct file_not_found
	: public exception
	{
		file_not_found(const std::string& fn)
		: exception(std::string("pastry: file not found: '") + fn + "'")
		{ }
	};

	namespace detail
	{
		inline bool can_read_file(const std::string& filename)
		{
			std::ifstream in(filename, std::ios::in | std::ios::binary);
			return in;
		}

		inline std::string load_text_file(const std::string& filename)
		{
			// http://insanecoding.blogspot.de/2011/11/how-to-read-in-file-in-c.html
			std::ifstream in(filename, std::ios::in | std::ios::binary);
			if(!in) {
				throw file_not_found(filename);
			}
			std::string contents;
			in.seekg(0, std::ios::end);
			contents.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&contents[0], contents.size());
			in.close();
			return contents;
		}

		inline char int_to_char(unsigned i)
		{
			if(i == 0 || i > 9)
				return '0';
			else
				return '1' + i - 1;
		}

		inline std::string int_to_string(unsigned i)
		{
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

		inline void compile_shader(glid_t q, std::string source)
		{
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
				// throw exception
				throw invalid_shader_source(source + "\n" + buffer);
			}
		}
	}

	struct vertex_shader
	: public detail::resource<rid::vertex_shader>
	{
		vertex_shader() {}
		
		vertex_shader(const std::string& source)
		{ compile(source); }
		
		void compile(const std::string& source)
		{ detail::compile_shader(id(), source); }
	};

	struct geometry_shader
	: public detail::resource<rid::geometry_shader>
	{
		geometry_shader() {}
		
		geometry_shader(const std::string& source)
		{ compile(source); }
		
		void compile(const std::string& source)
		{ detail::compile_shader(id(), source); }
	};

	struct fragment_shader
	: public detail::resource<rid::fragment_shader>
	{
		fragment_shader() {}
		
		fragment_shader(const std::string& source)
		{ compile(source); }
		
		void compile(const std::string& source)
		{ detail::compile_shader(id(), source); }
	};

	template<typename T>
	T load_shader(const std::string& filename)
	{ return T{detail::load_text_file(filename)}; }

	struct vertex_attribute;

	template<typename T, unsigned int NUM> struct uniform;

	struct program
	: public detail::resource<rid::program>
	{
		program() {}
		
		program(const vertex_shader& vs, const fragment_shader& fs)
		{
			attach(vs);
			attach(fs);
			link();
		}
		
		program(const vertex_shader& vs, const geometry_shader& gs, const fragment_shader& fs)
		{
			attach(vs);
			attach(gs);
			attach(fs);
			link();
		}
		
		void attach(const vertex_shader& s)
		{ glAttachShader(id(), s.id()); }
		
		void attach(const geometry_shader& s)
		{ glAttachShader(id(), s.id()); }
		
		void attach(const fragment_shader& s)	
		{ glAttachShader(id(), s.id()); }
		
		void link()
		{
			glLinkProgram(id());
			// check if link was successful
			GLint status;
			glGetProgramiv(id(), GL_LINK_STATUS, &status);
			if(status != GL_TRUE) {
				// print error message
				char buffer[1024];
				glGetProgramInfoLog(id(), 1024, NULL, buffer);
				throw invalid_shader_program();
			}
		}
		
		void use() const
		{ glUseProgram(id()); }
		
		static void unuse()
		{ glUseProgram(detail::INVALID_ID); }
		
		inline vertex_attribute get_attribute(const std::string& name) const;

		template<typename T, unsigned int NUM=1>
		uniform<T,NUM> get_uniform(const std::string& name) const;
	};

	inline program create_program(const std::string& src_vertex, const std::string& src_frag) 
	{
		return program{ {src_vertex}, {src_frag} };
	}

	inline program create_program(const std::string& src_vertex, const std::string& src_geom, const std::string& src_frag)
	{
		return program{ {src_vertex}, {src_geom}, {src_frag} };
	}

	inline program load_program(const std::string& fn_vertex, const std::string& fn_frag)
	{
		return program{
			load_shader<vertex_shader>(fn_vertex),
			load_shader<fragment_shader>(fn_frag)
		};
	}

	inline program load_program(const std::string& fn_vertex, const std::string& fn_geom, const std::string& fn_frag)
	{
		return program{
			load_shader<vertex_shader>(fn_vertex),
			load_shader<geometry_shader>(fn_geom),
			load_shader<fragment_shader>(fn_frag)
		};
	}

	/** Tries to load a shader with files fn.vert, fn.geom, fn.frag */
	inline program load_program(const std::string& fn)
	{
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

	struct vertex_attribute
	{
		GLint loc;

		bool is_valid() const
		{ return loc >= 0; }

		void configure(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer)
		{ glVertexAttribPointer(loc, size, type, normalized, stride, pointer); }

		void set_divisor(unsigned divisor)
		{ glVertexAttribDivisor(loc, divisor); }

		void enable()
		{ glEnableVertexAttribArray(loc); }
	};

	vertex_attribute program::get_attribute(const std::string& name) const
	{
		vertex_attribute va;
		va.loc = glGetAttribLocation(id(), name.data());
		if(va.loc == -1) {
			std::cerr << "ERROR: Inactive or invalid vertex attribute '" << name << "'" << std::endl;
		}
		return va;
	}

	namespace detail
	{
		template<typename K, unsigned int ROWS, unsigned int COLS, unsigned int NUM>
		struct uniform_impl;
		// {
		// 	static_assert(false,
		// 		"\n\n"
		// 		"ERROR with pastry::uniform / glUniform / glUniformMatrix:\n"
		// 		"\tInvalid row/column dimensions for OpenGL uniform.\n"
		// 		"\tSupported are: 1x1, 2x1, 3x1, 4x1, 2x2, 3x3, 4x4, 2x3, 3x2, 2x4, 4x2, 3x4, 4x3.\n");
		// };

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
				static void set(glid_t loc, const TYPE* v) { SETVEC(loc, NUM, v); } \
				static void get(glid_t prog, glid_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
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
				static void set(glid_t loc, const TYPE* v) { SETVEC(loc, NUM, GL_FALSE, v); } \
				static void get(glid_t prog, glid_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
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
				static void set(glid_t loc, const TYPE* v) { SETVEC(loc, NUM, GL_FALSE, v); } \
				static void get(glid_t prog, glid_t loc, TYPE* v) { GETVEC(prog, loc, v); } \
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

	struct invalid_uniform_location
	: public exception
	{
		invalid_uniform_location(const std::string& name)
		: exception("pastry: Inactive or invalid uniform location: name=" + name)
		{}
	};

	struct uniform_base
	{
		std::string name;

		GLint loc;

		program spo;

		bool valid() const
		{ return loc >= 0; }

	protected:
		void prepare() const
		{
			// if(!valid()) {
			// 	throw invalid_uniform_location{name};
			// }
			spo.use();
		}
	};

	/** Example: C++ int[2] / GLSL int[2]
	 * T: uniform type, must be float (GLSL: float), int (GLSL: int) or unsigned int (GLSL: uint)
	 * NUM>1: length of uniform array (for NUM=1 there is a specialization)
	 */
	template<typename T, unsigned int NUM=1>
	struct uniform
	: public uniform_base
	{
		void set(std::initializer_list<T> values_list)
		{
			if(values_list.size() != NUM) {
				std::cerr << "ERROR in uniform::set: Wrong number of arrays!" << std::endl;
				return;
			}
			if(!valid()) {
				return;
			}
			prepare();
			detail::uniform_impl<T,1,1,NUM>::set(loc, values_list.begin());
		}

		std::array<T,NUM> get(glid_t prog_id)
		{
			std::array<T,NUM> a;
			if(!valid()) {
				return a;
			}
			prepare();
			detail::uniform_impl<T,1,1,NUM>::get(prog_id, loc, a.begin());
			return a;
		}
	};

	/** Example: C++ int / GLSL int */
	template<typename T>
	struct uniform<T,1>
	: public uniform_base
	{
		void set(const T& v)
		{
			if(!valid()) {
				return;
			}
			prepare();
			detail::uniform_impl<T,1,1,1>::set(loc, &v);
		}

		T get(glid_t prog_id)
		{
			T v;
			if(!valid()) {
				return v;
			}
			prepare();
			detail::uniform_impl<T,1,1,1>::get(prog_id, loc, &v);
			return v;
		}
	};

	/** Example: C++ Eigen::Vector3f / GLSL vec3 */
	template<typename K, int R, int C>
	struct uniform<Eigen::Matrix<K,R,C>,1> : public uniform_base
	{
		typedef Eigen::Matrix<K,R,C> mat_t;

		void set(const mat_t& v)
		{
			if(!valid()) {
				return;
			}
			prepare();
			detail::uniform_impl<K,R,C,1>::set(loc, v.data());
		}

		mat_t get(glid_t prog_id)
		{
			mat_t v;
			if(!valid()) {
				return v;
			}
			prepare();
			detail::uniform_impl<K,R,C,1>::get(prog_id, loc, v.data());
			return v;
		}
	};

	struct invalid_uniform_initializer_list
	: public exception
	{
		invalid_uniform_initializer_list(unsigned expected, unsigned actual)
		: exception("pastry: invalid initializer_list for uniform: expected=TODO, actual=TODO")
		{}
	};

	/** Example: C++ Eigen::Vector3f[2] / GLSL vec3[2] */
	template<typename K, int R, int C, unsigned int NUM>
	struct uniform<Eigen::Matrix<K,R,C>,NUM> : public uniform_base
	{
		typedef Eigen::Matrix<K,R,C> mat_t;

		void set(std::initializer_list<mat_t> values_list)
		{
			if(values_list.size() != NUM) {
				throw invalid_uniform_initializer_list{NUM, values_list.list()};
			}
			// copy to continuous memory
			K buff[R*C*NUM];
			for(unsigned int i=0; i<NUM; i++) {
				const K* p = values_list[i].data();
				std::copy(p, p+R*C, &buff[i*R*C]);
			}
			// write to opengl
			if(!valid()) {
				return;
			}
			prepare();
			detail::uniform_impl<K,R,C,NUM>::set(loc, buff);
		}

		std::array<mat_t,NUM> get(glid_t prog_id)
		{
			std::array<mat_t,NUM> a;
			// read from opengl
			if(!valid()) {
				return;
			}
			prepare();
			K buff[R*C*NUM];
			detail::uniform_impl<K,R,C,NUM>::get(prog_id, loc, buff);
			// create array
			for(unsigned int i=0; i<NUM; i++) {
				const K* p = &buff[i*R*C];
				std::copy(p, p+R*C, a[i].data());
			}
			return a;
		}
	};

	template<typename T, unsigned int NUM>
	uniform<T,NUM> program::get_uniform(const std::string& name) const
	{
		uniform<T,NUM> u;
		u.name = name;
		u.spo = *this;
		u.loc = glGetUniformLocation(id(), name.data());
		return u;
	}

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

		struct layout_item
		{
			std::string name;
			GLenum type;
			int size;
		};

		inline std::size_t bytes_per_element(GLenum type)
		{
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

		inline std::vector<va_data> va_conf(const std::vector<layout_item>& list)
		{
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

		inline std::size_t va_bytes_total(const std::vector<layout_item>& list)
		{
			std::size_t n = 0;
			for(const layout_item& i : list) {
				n += i.size * bytes_per_element(i.type);
			}
			return n;
		}
	}

	inline detail::layout_item layout_skip_bytes(int a)
	{ return {"", 0, a}; }

	template<typename K, unsigned int N>
	inline detail::layout_item layout_skip()
	{ return layout_skip_bytes(sizeof(K)*N); }

	template<int TARGET>
	struct buffer
	: public detail::resource<rid::buffer>
	{
	private:
		std::size_t num_bytes_;
		GLuint usage_;

	public:
		std::vector<detail::va_data> layout_;

		buffer()
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW) {}

		buffer(std::initializer_list<detail::layout_item> list)
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW)
		{
			bind();
			set_layout(list);
		}

		buffer(std::initializer_list<detail::layout_item> list, GLuint usage)
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW)
		{
			bind();
			set_layout(list);
			init_data(usage);
		}

		buffer(std::initializer_list<detail::layout_item> list, std::size_t num_bytes, GLuint usage)
		: num_bytes_(0), usage_(GL_DYNAMIC_DRAW)
		{
			bind();
			set_layout(list);
			init_data(num_bytes, usage);
		}

		void set_layout(const std::vector<detail::layout_item>& list)
		{ layout_ = detail::va_conf(list); }
		
		void bind() const
		{ glBindBuffer(TARGET, id()); }
		
		template<typename T>
		void init_data(const std::vector<T>& v, GLuint usage)
		{ init_data(v.data(), v.size(), usage); }
		
		template<typename T>
		void init_data(const T* buf, std::size_t num_elements, GLuint usage)
		{ init_data(reinterpret_cast<const void*>(buf), sizeof(T)*num_elements, usage); }
		
		void init_data(std::size_t num_bytes, GLuint usage)
		{ init_data(nullptr, num_bytes_, usage); }

		void init_data(GLuint usage)
		{ init_data(nullptr, 0, usage); }

		template<typename T>
		void update_data(const std::vector<T>& v)
		{ update_data(v.data(), v.size()); }
		
		template<typename T>
		void update_data(const T* buf, std::size_t num_elements)
		{ update_data(reinterpret_cast<const void*>(buf), sizeof(T)*num_elements); }
		
	private:
		void init_data(const void* buf, std::size_t num_bytes, GLuint usage)
		{
			usage_ = usage;
			num_bytes_ = num_bytes;
			bind();
			glBufferData(TARGET, num_bytes_, buf, usage);
		}

		void update_data(const void* buf, std::size_t num_bytes)
		{
			if(num_bytes != num_bytes_) {
				init_data(buf, num_bytes, usage_);
			}
			else {
				bind();
				glBufferSubData(TARGET, 0, num_bytes, buf);
			}
		}

		static void unbind()
		{ glBindBuffer(TARGET, detail::INVALID_ID); }
	};

	typedef buffer<GL_ARRAY_BUFFER> array_buffer;

	typedef buffer<GL_ELEMENT_ARRAY_BUFFER> element_array_buffer;

	namespace detail
	{
		struct mapping
		{
			std::string shader_name;
			array_buffer vb;
			std::string vb_name;
			unsigned divisor;
			
			mapping()
			{}
			
			mapping(const std::string& nsn, const array_buffer& nvb)
			: shader_name(nsn), vb(nvb), vb_name(nsn), divisor(0)
			{}
			
			mapping(const std::string& nsn, const array_buffer& nvb, const std::string& nvbn)
			: shader_name(nsn), vb(nvb), vb_name(nvbn), divisor(0)
			{}
			
			mapping(const std::string& nsn, const array_buffer& nvb, const std::string& nvbn, unsigned nd)
			: shader_name(nsn), vb(nvb), vb_name(nvbn), divisor(nd)
			{}
		};
	}

	struct vertex_array
	: public detail::resource<rid::vertex_array>
	{
		std::vector<vertex_attribute> attributes_;

		vertex_array()
		{}

		vertex_array(const program& p, std::initializer_list<detail::mapping> list)
		{ set_layout(p, list.begin(), list.end()); }

		template<typename It>
		void set_layout(const program& p, It kt1, It kt2)
		{
			bind();
			for(auto kt=kt1; kt!=kt2; ++kt) {
				const detail::mapping& m = *kt;
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
				// std::cout << "shader_name = " << shader_name << std::endl;
				// std::cout << "vb = " << vb << std::endl;
				// std::cout << "vn_name = " << vn_name << std::endl;
				// std::cout << "size = " << it->size << std::endl;
				// std::cout << "type = " << it->type << std::endl;
				// std::cout << "stride = " << vb.layout_.back().offset_end << std::endl;
				// std::cout << "offset = " << it->offset_begin << std::endl;
				// std::cout << "divisor = " << m.divisor << std::endl;
				va.configure(it->size, it->type, GL_FALSE, vb.layout_.back().offset_end, (GLvoid*)it->offset_begin);
				va.enable();
				va.set_divisor(m.divisor); // for instancing
				attributes_.push_back(va);
			}
		}

		void bind()
		{
			glBindVertexArray(id());
		}

	};

	namespace detail
	{
		template<int MODE> struct mesh_type_traits;
		#define MESH_TYPE_DEF(M,NB,NE) \
			template<> struct mesh_type_traits<M> { \
				static constexpr unsigned num_base = NB; \
				static constexpr unsigned num_per_element = NE; \
			};
		MESH_TYPE_DEF(GL_POINTS,0,1)
		MESH_TYPE_DEF(GL_LINE_STRIP,1,1)
		MESH_TYPE_DEF(GL_LINES,0,2)
		MESH_TYPE_DEF(GL_TRIANGLE_STRIP,2,1)
		MESH_TYPE_DEF(GL_TRIANGLE_FAN,2,1)
		MESH_TYPE_DEF(GL_TRIANGLES,0,3)

		template<int IT> struct mesh_index_types_traits;
		#define INDEX_TYPE_DEF(IT,K) \
			template<> struct mesh_index_types_traits<IT> { \
				typedef K result; \
			};
		INDEX_TYPE_DEF(GL_UNSIGNED_BYTE,uint8_t)
		INDEX_TYPE_DEF(GL_UNSIGNED_SHORT,uint16_t)
		INDEX_TYPE_DEF(GL_UNSIGNED_INT,uint32_t)

		template<typename I, int N>
		struct index { typedef std::array<I,N> result; };
		
		template<typename I>
		struct index<I,1> { typedef I result; };
	}

	/** Class to hold mesh data
	 * Example usage:
	 *   // mesh data for a quad
	 *   struct my_vertex { float x,y,z; };
	 *   pastry::triangle_mesh<my_vertex> mesh;
	 *   mesh.vertices = { {0,0,0}, {1,0,0}, {1,1,0}, {0,1,0} };
	 *   mesh.indices= { {0,1,2}, {0,2,3} };
	 */
	template<typename V, int MODE, int INDEX_TYPE=GL_UNSIGNED_INT>
	struct mesh
	{
		typedef detail::mesh_type_traits<MODE> m_traits;
		
		typedef detail::mesh_index_types_traits<INDEX_TYPE> i_traits;
		
		typedef typename detail::index<
			typename i_traits::result,
			m_traits::num_per_element>::result I;
		
		std::vector<V> vertices;
		
		std::vector<I> indices;

		void draw_arrays() const
		{ glDrawArrays(MODE, 0, vertices.size()); }

		void draw_arrays_instanced(std::size_t primcount) const
		{ glDrawArraysInstanced(MODE, 0, vertices.size(), primcount); }

		void draw_elements() const
		{
			std::size_t count = m_traits::num_per_element*indices.size();
			glDrawElements(MODE, count, INDEX_TYPE, 0);
		}

		void draw_elements_instanced(std::size_t primcount) const
		{
			std::size_t count = m_traits::num_per_element*indices.size();
			glDrawElementsInstanced(MODE, count, INDEX_TYPE, 0, primcount);
		}
	};

	// template<typename V>
	// using point_mesh = mesh<V, GL_POINTS>;

	// template<typename V>
	// using line_strip_mesh = mesh<V, GL_LINE_STRIP>;

	// template<typename V>
	// using line_mesh = mesh<V, GL_LINES>;

	// template<typename V>
	// using triangle_strip_mesh = mesh<V, GL_TRIANGLE_STRIP>;

	// template<typename V>
	// using triangle_fan_mesh = mesh<V, GL_TRIANGLE_FAN>;

	// template<typename V>
	// using triangle_mesh = mesh<V, GL_TRIANGLES>;

	struct single_mesh
	{
	private:
		GLenum mode_;
		GLenum index_type_;
		
		array_buffer vertex_bo_;
		element_array_buffer index_bo_;

		std::size_t num_vertices_ = 0;
		std::size_t num_indices_ = 0;

	public:
		const array_buffer& get_vertex_bo() const
		{ return vertex_bo_; }
		
		const element_array_buffer& get_index_bo() const
		{ return index_bo_; }
		
		array_buffer& get_vertex_bo()
		{ return vertex_bo_; }
		
		element_array_buffer& get_index_bo()
		{ return index_bo_; }
		
		void set_vertex_bo(const array_buffer& o)
		{ vertex_bo_ = o; }
		
		void set_index_bo(const element_array_buffer& o)
		{ index_bo_ = o; }

	public:
		single_mesh() {
			clear();
		}
		
		single_mesh(GLenum mode) {
			set_mode(mode);
			clear();
		}

		void clear() {
			num_vertices_ = 0;
			num_indices_ = 0;
		}

		void set_mode(GLenum mode) {
			mode_ = mode;
		}

		template<typename V>
		void set_vertices(const std::vector<V>& vertices) {
			num_vertices_ = vertices.size();
			vertex_bo_.update_data(vertices);
		}

		void set_indices(const std::vector<uint8_t>& indices) {
			index_type_ = GL_UNSIGNED_BYTE;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		void set_indices(const std::vector<uint16_t>& indices) {
			index_type_ = GL_UNSIGNED_SHORT;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		void set_indices(const std::vector<uint32_t>& indices) {
			index_type_ = GL_UNSIGNED_INT;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		void clear_indices() {
			set_indices(std::vector<uint8_t>{});
		}

		void render() {
			if(num_vertices_ == 0) {
				return;
			}
			if(num_indices_ == 0) {
				vertex_bo_.bind();
				glDrawArrays(mode_, 0, num_vertices_);
			}
			else {
				vertex_bo_.bind();
				index_bo_.bind(); // bind the index buffer object!
				glDrawElements(mode_, num_indices_, index_type_, 0);
			}
		}
	};

	struct multi_mesh
	{
	private:
		GLenum mode_;
		GLenum index_type_;
		
		array_buffer vertex_bo_;
		element_array_buffer index_bo_;
		array_buffer instance_bo_;

		std::size_t num_vertices_;
		std::size_t num_indices_;
		std::size_t num_instances_;

	public:
		const array_buffer& get_vertex_bo() const { return vertex_bo_; }
		const element_array_buffer& get_index_bo() const { return index_bo_; }
		const array_buffer& get_instance_bo() const { return instance_bo_; }
		array_buffer& get_vertex_bo() { return vertex_bo_; }
		element_array_buffer& get_index_bo() { return index_bo_; }
		array_buffer& get_instance_bo() { return instance_bo_; }
		void set_vertex_bo(const array_buffer& o) { vertex_bo_ = o; }
		void set_index_bo(const element_array_buffer& o) { index_bo_ = o; }
		void set_instance_bo(const array_buffer& o) { instance_bo_ = o; }

	public:
		multi_mesh() {
			clear();
		}
		
		multi_mesh(GLenum mode) {
			set_mode(mode);
			clear();
		}

		void clear() {
			num_vertices_ = 0;
			num_indices_ = 0;			
			num_instances_ = 0;
		}

		void set_mode(GLenum mode) {
			mode_ = mode;
		}

		template<typename V>
		void set_vertices(const std::vector<V>& vertices) {
			num_vertices_ = vertices.size();
			vertex_bo_.update_data(vertices);
		}

		void set_indices(const std::vector<uint8_t>& indices) {
			index_type_ = GL_UNSIGNED_BYTE;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		void set_indices(const std::vector<uint16_t>& indices) {
			index_type_ = GL_UNSIGNED_SHORT;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		void set_indices(const std::vector<uint32_t>& indices) {
			index_type_ = GL_UNSIGNED_INT;
			num_indices_ = indices.size();
			index_bo_.update_data(indices);
		}

		template<typename A>
		void set_instances(const std::vector<A>& instances) {
			num_instances_ = instances.size();
			instance_bo_.update_data(instances);
		}

		void set_instances_raw(std::size_t num, const std::vector<unsigned char>& instances_data) {
			num_instances_ = num;
			instance_bo_.update_data(instances_data);
		}

		void render() {
			if(num_vertices_ == 0) {
				return;
			}
			if(num_indices_ == 0) {
				// use glDrawArrays
				if(num_instances_ == 0) {
					glDrawArrays(mode_, 0, num_vertices_);
				}
				else {
					glDrawArraysInstanced(mode_, 0, num_vertices_, num_instances_);
				}
			}
			else {
				// use glDrawElements
				index_bo_.bind(); // bind the index buffer object!
				if(num_instances_ == 0) {
					glDrawElements(mode_, num_indices_, index_type_, 0);
				}
				else {
					glDrawElementsInstanced(mode_, num_indices_, index_type_, 0, num_instances_);
				}
			}
		}
	};

	namespace detail
	{
		template<unsigned C> struct texture_format;
		#define TEXTURE_FORMAT(N,V) \
			template<> struct texture_format<N> { \
				static constexpr GLint result = V; \
			};
		TEXTURE_FORMAT(1, GL_RED)
		TEXTURE_FORMAT(2, GL_RG)
		TEXTURE_FORMAT(3, GL_RGB)
		TEXTURE_FORMAT(4, GL_RGBA)
		#undef TEXTURE_FORMAT

		template<typename T> struct texture_type;
		#define TEXTURE_TYPE(T,V) \
			template<> struct texture_type<T> { \
				static constexpr GLint result = V; \
			};
		TEXTURE_TYPE(unsigned char, GL_UNSIGNED_BYTE)
		TEXTURE_TYPE(char, GL_BYTE)
		TEXTURE_TYPE(unsigned short, GL_UNSIGNED_SHORT)
		TEXTURE_TYPE(short, GL_SHORT)
		TEXTURE_TYPE(unsigned int, GL_UNSIGNED_INT)
		TEXTURE_TYPE(int, GL_INT)
		TEXTURE_TYPE(float, GL_FLOAT)
		#undef TEXTURE_TYPE
	}

	template<GLenum TARGET>
	struct texture_base
	: public detail::resource<rid::texture_base>
	{
		static constexpr GLenum target = TARGET;

		texture_base()
		{}

		texture_base(glid_t tex_id)
		: detail::resource<rid::texture_base>(tex_id)
		{}
		
		void create(GLenum filter, GLenum wrap)
		{
			bind();
			set_filter(filter);
			set_wrap(wrap);
		}
	
		void create()
		{ create(GL_LINEAR, GL_REPEAT); }
		
		void bind() const
		{ glBindTexture(target, id()); }
		
		void set_wrap_s(GLint value)
		{ glTexParameteri(target, GL_TEXTURE_WRAP_S, value); }
		
		void set_wrap_t(GLint value)
		{ glTexParameteri(target, GL_TEXTURE_WRAP_T, value); }
		
		void set_wrap_r(GLint value)
		{ glTexParameteri(target, GL_TEXTURE_WRAP_R, value); }
		
		void set_wrap(GLint value)
		{
			set_wrap_s(value);
			set_wrap_t(value);
			set_wrap_r(value);
		}
		
		void set_border_color(float cr, float cg, float cb)
		{
			float color[] = {cr, cg, cb};
			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, color);
		}
		
		void set_min_filter(GLint value)
		{ glTexParameteri(target, GL_TEXTURE_MIN_FILTER, value); }
	
		void set_mag_filter(GLint value)
		{ glTexParameteri(target, GL_TEXTURE_MAG_FILTER, value); }
		
		void set_filter(GLint value)
		{
			set_min_filter(value);
			set_mag_filter(value);
		}
		
		// void generate_mipmap() {
		// 	glGenerateMipmap(target);
		// }
		
		int get_param_i(GLenum pname) const
		{
			bind();
			GLint val;
			glGetTexLevelParameteriv(target, 0, pname, &val);
			return val;
		}
		
		GLint internalformat() const
		{ return get_param_i(GL_TEXTURE_INTERNAL_FORMAT); }

		int width() const
		{ return get_param_i(GL_TEXTURE_WIDTH); }
		
		int height() const
		{ return get_param_i(GL_TEXTURE_HEIGHT); }
		
		int channels() const
		{
			switch(internalformat()) {
				case GL_DEPTH_COMPONENT:
				case GL_DEPTH_COMPONENT16:
				case GL_DEPTH_COMPONENT24:
				case GL_DEPTH_COMPONENT32F:
				case GL_DEPTH_STENCIL:
				case GL_DEPTH32F_STENCIL8:
				case GL_DEPTH24_STENCIL8:
				case GL_RED:
				case GL_R8:
				case GL_R32F:
					return 1;
				case GL_RG:
				case GL_RG8:
				case GL_RG32F:
					return 2;
				case GL_RGB:
				case GL_RGB8:
				case GL_RGB32F:
					return 3;
				case GL_RGBA:
				case GL_RGBA8:
				case GL_RGBA32F:
					return 4;
				default:
					// ERROR unknown
					return 0;
			}
		}

		GLenum format() const
		{
			switch(internalformat()) {
				case GL_DEPTH_COMPONENT:
				case GL_DEPTH_COMPONENT16:
				case GL_DEPTH_COMPONENT24:
				case GL_DEPTH_COMPONENT32F:
					return GL_DEPTH_COMPONENT;
				case GL_DEPTH_STENCIL:
				case GL_DEPTH32F_STENCIL8:
				case GL_DEPTH24_STENCIL8:
					return GL_DEPTH_STENCIL;
				case GL_RED:
				case GL_R8:
				case GL_R32F:
					return GL_RED;
				case GL_RG:
				case GL_RG8:
				case GL_RG32F:
					return GL_RG;
				case GL_RGB:
				case GL_RGB8:
				case GL_RGB32F:
					return GL_RGB;
				case GL_RGBA:
				case GL_RGBA8:
				case GL_RGBA32F:
					return GL_RGBA;
				default:
					// ERROR unknown
					return 0;
			}
		}

		static void unbind()
		{ glBindTexture(target, detail::INVALID_ID); }

		static void activate_unit(unsigned int num)
		{ glActiveTexture(GL_TEXTURE0 + num); }
	};

	struct texture_2d
	: public texture_base<GL_TEXTURE_2D>
	{
		texture_2d()
		{}

		texture_2d(glid_t tex_id)
		: texture_base<GL_TEXTURE_2D>(tex_id)
		{}
		
		void set_image_impl(GLint internalformat, unsigned w, unsigned h, GLenum format, GLenum type, const void* data)
		{
			bind();
			glTexImage2D(target,
				0, // level: use base image level
				internalformat,
				w, h,
				0, // must be 0
				format, // format of source data
				type, // type of source data
				data);
		}

		template<typename S, unsigned C>
		void set_image(GLint internalformat, unsigned w, unsigned h, const S* data=0)
		{
			 // internalformat is i.e. GL_RGBA8, GL_RGB32F, ...
			set_image_impl(internalformat, w, h, detail::texture_format<C>::result, detail::texture_type<S>::result, data);
		}

		template<typename S>
		void set_image_depth(GLint internalformat, unsigned w, unsigned h, const S* data=0)
		{
			 // internalformat is i.e. GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32F
			set_image_impl(internalformat, w, h, GL_DEPTH_COMPONENT, detail::texture_type<S>::result, data);
		}

		void set_image_depth_stencil(GLint internalformat, unsigned w, unsigned h)
		{
			 // internalformat is i.e. GL_DEPTH32F_STENCIL8, GL_DEPTH24_STENCIL8
			GLenum type = 0;
			if(internalformat == GL_DEPTH24_STENCIL8) type = GL_UNSIGNED_INT_24_8;
			if(internalformat == GL_DEPTH32F_STENCIL8) type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV; // ??
			set_image_impl(internalformat, w, h, GL_DEPTH_STENCIL, type, 0);
		}

		template<typename S>
		std::vector<S> get_image() const
		{
			bind();
			if(internalformat() == GL_DEPTH24_STENCIL8) {
				std::vector<unsigned> buff(width()*height());
				glGetTexImage(target, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, buff.data());
				std::vector<unsigned char> buff2(width()*height());
				for(size_t i=0; i<buff2.size(); i++) buff2[i] = (buff[i] >> 24); // depth
				for(size_t i=0; i<buff2.size(); i++) buff2[i] = buff[i] & 0xFF; // stencil
				return buff2;
			}
			else {
				std::vector<S> buff(width()*height()*channels());
				glGetTexImage(target, 0, format(), detail::texture_type<S>::result, buff.data());
				return buff;
			}
		}

		template<typename S, unsigned C>
		static texture_2d create_normal(GLint internalformat, unsigned w, unsigned h, const S* data=0)
		{
			texture_2d tex;
			tex.create();
			tex.set_image<S,C>(internalformat, w, h, data);
			return tex;
		}

		template<typename S>
		static texture_2d create_depth(GLint internalformat, unsigned w, unsigned h, const S* data=0)
		{
			texture_2d tex;
			tex.create();
			tex.set_image_depth<S>(internalformat, w, h, data);
			return tex;
		}

	};

	struct texture_cube_map
	: public texture_base<GL_TEXTURE_CUBE_MAP>
	{
		static GLenum cube_map_type(unsigned i)
		{
			constexpr GLenum types[6] = { 
				GL_TEXTURE_CUBE_MAP_NEGATIVE_X, // left 
				GL_TEXTURE_CUBE_MAP_POSITIVE_Z, // front
				GL_TEXTURE_CUBE_MAP_POSITIVE_X, // right
				GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, // back
				GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, // bottom
				GL_TEXTURE_CUBE_MAP_POSITIVE_Y  // top
			};
			return types[i];
		}

		texture_cube_map()
		{}

		texture_cube_map(glid_t tex_id)
		: texture_base<GL_TEXTURE_CUBE_MAP>(tex_id)
		{}
		
		template<typename S, unsigned C>
		void set_image_mm(GLenum target, GLint internalformat, unsigned level, unsigned w, unsigned h, const S* data=0)
		{
			bind();
			glTexImage2D(target,
				level, // level: use base image level
				internalformat, // i.e. GL_RGBA8, GL_R32F, GL_RG16UI ...
				w, h,
				0, // must be 0
				detail::texture_format<C>::result, // format of source data
				detail::texture_type<S>::result, // type of source data
				data);
		}

		template<typename S, unsigned C>
		void set_image(GLenum target, GLint internalformat, unsigned level, unsigned w, unsigned h, const S* data=0)
		{
			set_image_mm(target, internalformat, 0, w, h, data);
		}

		template<typename S, unsigned C>
		void set_image(GLint internalformat, unsigned w, unsigned h, const std::vector<S*>& data={})
		{
			bind();
			for(int i=0; i<6; i++) {
				glTexImage2D(cube_map_type(i),
					0, // level: use base image level
					internalformat, // i.e. GL_RGBA8, GL_R32F, GL_RG16UI ...
					w, h,
					0, // must be 0
					detail::texture_format<C>::result, // format of source data
					detail::texture_type<S>::result, // type of source data
					i < data.size() ? data[i] : 0);
			}
		}

		template<typename S>
		std::vector<std::vector<S>> get_image() const
		{
			std::vector<std::vector<S>> result(6, std::vector<S>(width()*height()*channels()));
			for(int i=0; i<6; i++) {
				bind();
				glGetTexImage(cube_map_type(i),
					0, // level: use base image level
					format(),
					detail::texture_type<S>::result,
					result[i].data());
			}
			return result;
		}

	};

	namespace TextureModes
	{
		enum def {
			NORM=0,
			SNORM=1,
			FLOAT=2,
			INT=3,
			UINT=4
		};
	}
	typedef TextureModes::def TextureMode;

	namespace detail
	{
		template<unsigned CHANNELS, int MODE, unsigned BPC> struct texture_internal_format;
		#define PASTRY_DETAIL_TEX_CHANNELS_1 R
		#define PASTRY_DETAIL_TEX_CHANNELS_2 RG
		#define PASTRY_DETAIL_TEX_CHANNELS_3 RGB
		#define PASTRY_DETAIL_TEX_CHANNELS_4 RGBA
		#define PASTRY_DETAIL_TEX_CHANNELS(C) PASTRY_DETAIL_TEX_CHANNELS_##C
		#define PASTRY_DETAIL_TEX_MODE_0 
		#define PASTRY_DETAIL_TEX_MODE_1 _SNORM
		#define PASTRY_DETAIL_TEX_MODE_2 F
		#define PASTRY_DETAIL_TEX_MODE_3 I
		#define PASTRY_DETAIL_TEX_MODE_4 UI
		#define PASTRY_DETAIL_TEX_MODE(M) PASTRY_DETAIL_TEX_MODE_##M
		#define PASTRY_DETAIL_TEX_CONCAT4_(A, B, C, D) A ## B ## C ## D
		#define PASTRY_DETAIL_TEX_CONCAT4(A, B, C, D) PASTRY_DETAIL_TEX_CONCAT4_(A, B, C, D)
		#define PASTRY_DETAIL_TEX_FORMAT(C,M,BPC) \
			PASTRY_DETAIL_TEX_CONCAT4(GL_, PASTRY_DETAIL_TEX_CHANNELS(C), BPC, PASTRY_DETAIL_TEX_MODE(M))
		#define TEXTURE_INTERNAL_FORMAT(C,M,BPC) \
			template<> struct texture_internal_format<C,M,BPC> { \
				static constexpr GLint result = PASTRY_DETAIL_TEX_FORMAT(C,M,BPC); \
			};
		#define PASTRY_DETAIL_TEX_DEF_N(C,M) \
			TEXTURE_INTERNAL_FORMAT(C, M, 8) \
			TEXTURE_INTERNAL_FORMAT(C, M, 16)
		#define PASTRY_DETAIL_TEX_DEF_I(C,M) \
			TEXTURE_INTERNAL_FORMAT(C, M, 8) \
			TEXTURE_INTERNAL_FORMAT(C, M, 16) \
			TEXTURE_INTERNAL_FORMAT(C, M, 32)
		#define PASTRY_DETAIL_TEX_DEF_F(C,M) \
			TEXTURE_INTERNAL_FORMAT(C, M, 16) \
			TEXTURE_INTERNAL_FORMAT(C, M, 32)
		#define PASTRY_DETAIL_TEX_DEF(C) \
			PASTRY_DETAIL_TEX_DEF_N(C, 0) \
			PASTRY_DETAIL_TEX_DEF_N(C, 1) \
			PASTRY_DETAIL_TEX_DEF_F(C, 2) \
			PASTRY_DETAIL_TEX_DEF_I(C, 3) \
			PASTRY_DETAIL_TEX_DEF_I(C, 4)
		PASTRY_DETAIL_TEX_DEF(1)
		PASTRY_DETAIL_TEX_DEF(2)
		PASTRY_DETAIL_TEX_DEF(3)
		PASTRY_DETAIL_TEX_DEF(4)
		#undef TEXTURE_INTERNAL_FORMAT
	}

	template<unsigned CHANNELS, int MODE, unsigned BPC>
	struct texture
	: public texture_2d
	{
		constexpr GLint internal_format()
		{ return detail::texture_internal_format<CHANNELS,MODE,BPC>::result; }

		template<typename S, unsigned C>
		void set_image(unsigned w, unsigned h, const S* data)
		{
			set_image<S,C>(
				internal_format(),
				w, h, data);
		}
	};

	struct renderbuffer
	: public detail::resource<rid::renderbuffer>
	{		
		void bind()
		{ glBindRenderbuffer(GL_RENDERBUFFER, id()); }
		
		void storage(GLenum internalformat, unsigned width, unsigned height)
		{ glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height); }
	};

	struct framebuffer
	: public detail::resource<rid::framebuffer>
	{	
		enum class target { READ, WRITE, BOTH };

		static GLenum GetTarget(target t)
		{
			switch(t) {
				case target::READ: return GL_READ_FRAMEBUFFER;
				case target::WRITE: return GL_DRAW_FRAMEBUFFER;
				default: case target::BOTH: return GL_FRAMEBUFFER;
			}
		}

		void bind(target t=target::BOTH)
		{ glBindFramebuffer(GetTarget(t), id()); }
		
		void attach(GLenum attachment, const texture_2d& tex)
		{ glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, GL_TEXTURE_2D, tex.id(), 0); }
		
		void attach(GLenum attachment, const renderbuffer& rbo)
		{ glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rbo.id()); }
		
		static void unbind(target t=target::BOTH)
		{ glBindFramebuffer(GetTarget(t), detail::INVALID_ID); }
	};

	/** Enables/disables an OpenGL capability like GL_BLEND and automatically restores the state
	 * Usage example:
	 *		{ capability{{GL_BLEND, true}};
	 *			// ... code to execute with blending enabled
	 * 		}
	 */
	struct capability {
	private:
		struct capability_impl {
			capability_impl(GLenum cap, bool set_to) {
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

			~capability_impl() {
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
	public:
		capability(GLenum cap, bool set_to)
		: capabilities_{{cap, set_to}} {}
		capability(std::initializer_list<capability_impl> caps_list)
		: capabilities_(caps_list) {}
	private:
		std::vector<capability_impl> capabilities_;
	};

	template<typename F>
	void with_capabilities(const capability& caps, F f) {
		f();
	}

}}
#endif
