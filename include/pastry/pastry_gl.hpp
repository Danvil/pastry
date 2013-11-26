#ifndef INCLUDED_PASTRY_PASTRYGL_HPP
#define INCLUDED_PASTRY_PASTRYGL_HPP

#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <memory>

namespace pastry
{
	typedef GLuint id_t;

	struct array_buffer_id {};
	struct attribute_id {};
	struct vertex_shader_id {};
	struct fragment_shader_id {};
	struct program_id {};

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
				return ptr_ ? ptr_->id() : INVALID_ID;
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
			GLint a = glGetAttribLocation(id(), name.data());
			if(a == -1) {
				std::cerr << "ERROR: Inactive or invalid vertex attribute" << std::endl;
				a = 0;
			}
			vertex_attribute va;
			va.id_set(a);
			return va;
		}
	};

}
#endif
