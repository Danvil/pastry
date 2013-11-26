#ifndef INCLUDED_PASTRY_PASTRYGL_HPP
#define INCLUDED_PASTRY_PASTRYGL_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <string>
#include <memory>
#include <array>

namespace pastry
{
	typedef GLuint id_t;

	struct array_buffer_id {};
	struct attribute_id {};
	struct vertex_shader_id {};
	struct fragment_shader_id {};
	struct program_id {};
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

		template<> struct handler<attribute_id>
		{
			static id_t gl_create() { return 0; }
			static void gl_delete(id_t) { }
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

	struct array_buffer : public detail::resource<array_buffer_id>
	{
		void bind() {
			glBindBuffer(GL_ARRAY_BUFFER, id());
		}
		void data(void* buf, std::size_t buf_num_bytes, GLuint usage) {
			bind();
			glBufferData(GL_ARRAY_BUFFER, buf_num_bytes, buf, usage);
		}
	};

	namespace detail
	{
		void compile_shader(id_t q, const std::string& source) {
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

	struct vertex_attribute : public detail::resource<attribute_id>
	{
		vertex_attribute() {}
		void configure(GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer) {
			glVertexAttribPointer(id(), size, type, normalized, stride, pointer);
		}
		void enable() {
			glEnableVertexAttribArray(id());
		}
	};

	namespace detail
	{
		#define PASTRY_UNIFORM_TYPES_FN(F,N) \
			F(float, N, glUniform##N##fv, glGetUniformfv) \
			F(int, N, glUniform##N##iv, glGetUniformiv) \
			F(unsigned int, N, glUniform##N##uiv, glGetUniformuiv)

		#define PASTRY_UNIFORM_TYPES_F(F) \
			PASTRY_UNIFORM_TYPES_FN(F,1) \
			PASTRY_UNIFORM_TYPES_FN(F,2) \
			PASTRY_UNIFORM_TYPES_FN(F,3) \
			PASTRY_UNIFORM_TYPES_FN(F,4)

		template<typename TYPE, unsigned int LEN, unsigned int NUM>
		struct uniform_impl;

		#define PASTRY_UNIFORM_DEF(TYPE,LEN,SETVEC,GETVEC) \
			template<unsigned int NUM>\
			struct uniform_impl<TYPE,LEN,NUM> {\
				static void set(id_t loc, const TYPE* v) { SETVEC(loc, NUM, v); }\
				static void get(id_t prog, id_t loc, TYPE* v) { GETVEC(prog, loc, v); }\
			};

		PASTRY_UNIFORM_TYPES_F(PASTRY_UNIFORM_DEF)
	}

	/**
	 * T: uniform type, must be float (GLSL: float), int (GLSL: int) or unsigned int (GLSL: uint)
	 * LEN: length of uniform type: 1 for integral, N for vecN
	 * NUM: length of uniform array: 1 for single value, M for vecN[M] or T[M]
	 */
	template<typename T, unsigned int LEN, unsigned int NUM>
	struct uniform : public detail::resource<uniform_id>
	{
		uniform() {}
		void set(std::initializer_list<std::initializer_list<T>> values_list) {
			if(values_list.size() != NUM) {
				std::cerr << "ERROR in uniform::set: Wrong number of arrays!" << std::endl;
				return;
			}
			// format into buffer
			T buff[NUM*LEN];
			for(int i=0; i<NUM; i++) {
				if((values_list.begin() + i)->size() != LEN) {
					std::cerr << "ERROR in uniform::set: Wrong number of arguments!" << std::endl;
				}
				for(int j=0; j<LEN; j++) {
					buff[j + i*LEN] = *((values_list.begin() + i)->begin() + j);
				}
			}
			// write data
			set_ptr(buff);
		}
		std::array<std::array<T,LEN>,NUM> get(id_t prog_id) {
			// get raw data
			T buff[LEN*NUM];
			get_ptr(prog_id, buff);
			// format into list of arrays
			std::array<std::array<T,LEN>,NUM> a;
			for(int i=0; i<NUM; i++) {
				std::array<T,LEN> a;
			}
			return a;
		}
		void set_ptr(const T* v) {
			detail::uniform_impl<T,LEN,NUM>::set(id(), v);
		}
		void get_ptr(id_t prog_id, const T* v) {
			detail::uniform_impl<T,LEN,NUM>::get(prog_id, id(), v);
		}
	};

	template<typename T, unsigned int LEN>
	struct uniform<T,LEN,1> : public detail::resource<uniform_id>
	{
		uniform() {}
		void set(std::initializer_list<T> values) {
			if(values.size() != LEN) {
				std::cerr << "ERROR in uniform::set: Wrong number of arguments!" << std::endl;
			}
			else {
				set_ptr(values.begin());
			}
		}
		std::array<T,LEN> get(id_t prog_id) {
			std::array<T,LEN> a;
			get_ptr(prog_id, a.begin());
			return a;
		}
		void set_ptr(const T* v) {
			detail::uniform_impl<T,LEN,1>::set(id(), v);
		}
		void get_ptr(id_t prog_id, const T* v) {
			detail::uniform_impl<T,LEN,1>::get(prog_id, id(), v);
		}
	};

	template<typename T, unsigned int NUM>
	struct uniform<T,1,NUM> : public detail::resource<uniform_id>
	{
		uniform() {}
		void set(std::initializer_list<T> values) {
			if(values.size() != NUM) {
				std::cerr << "ERROR in uniform::set: Wrong number of arrays!" << std::endl;
			}
			else {
				set_ptr(values.begin());
			}
		}
		std::array<T,NUM> get(id_t prog_id) {
			std::array<T,NUM> a;
			get_ptr(prog_id, a.begin());
			return a;
		}
		void set_ptr(const T* v) {
			detail::uniform_impl<T,1,NUM>::set(id(), v);
		}
		void get_ptr(id_t prog_id, const T* v) {
			detail::uniform_impl<T,1,NUM>::get(prog_id, id(), v);
		}
	};

	template<typename T>
	struct uniform<T,1,1> : public detail::resource<uniform_id>
	{
		uniform() {}
		void set(T value) {
			set_ptr(&value);
		}
		T get(id_t prog_id) {
			T a;
			get_ptr(prog_id, &a);
			return a;
		}
	private:
		void set_ptr(const T* v) {
			detail::uniform_impl<T,1,1>::set(id(), v);
		}
		void get_ptr(id_t prog_id, const T* v) {
			detail::uniform_impl<T,1,1>::get(prog_id, id(), v);
		}
	};

	struct program : public detail::resource<program_id>
	{
		program() {}
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
		vertex_attribute get_attribute(const std::string& name) {
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
		template<typename T, unsigned int LEN=1, unsigned int NUM=1>
		uniform<T,LEN,NUM> get_uniform(const std::string& name) {
			uniform<T,LEN,NUM> u;
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

	struct texture : public detail::resource<texture_id>
	{
		static constexpr GLenum target = GL_TEXTURE_2D;
		texture() {}
		texture(int w, int h, float* data_rgb_f) {
			id_create();
			bind();
			set_wrap(GL_REPEAT);
			set_filter(GL_LINEAR);
			image_2d_rgb_f(w, h, data_rgb_f);
		}
		void bind() {
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
			glTexImage2D(target, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, data_rgb_f);
		}
		static void activate_unit(unsigned int num) {
			glActiveTexture(GL_TEXTURE0 + num);
		}
	};

}
#endif
