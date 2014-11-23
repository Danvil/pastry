#include <pastry/pastry.hpp>

namespace pastry {

class Primitives2D
: public renderling
{
private:
	pastry::array_buffer vbo;
	pastry::program spo;
	pastry::vertex_array va;

public:
	struct Vertex
	{
		float x, y;
		float cr, cg, cb;
	};

	Primitives2D()
	: color_(1.0f,1.0f,1.0f) {

		vbo = pastry::array_buffer({
			{"position", GL_FLOAT, 2},
			{"color", GL_FLOAT, 3}
		});
		vbo.init_data(GL_DYNAMIC_DRAW);

		std::string vert_src = PASTRY_GLSL(
			uniform mat4 proj;
			uniform mat4 view;
			in vec2 position;
			in vec3 color;
			out vec3 vcolor;
			void main() {
				gl_Position = proj*vec4(position, 0.0, 1.0f);
				vcolor = color;
			}
		);
		std::string frag_src = PASTRY_GLSL(
			in vec3 vcolor;
			out vec4 outColor;
			void main() {
				outColor = vec4(vcolor,1);
			}
		);
		spo = pastry::program(vert_src, frag_src);

		va = pastry::vertex_array(spo, {
			{"position", vbo},
			{"color", vbo}
		});
	}

	void setColor(const Eigen::Vector3f& color)
	{ color_ = color; }

	void setCamera(const Eigen::Vector2f& center, float width)
	{
		// update projection matrix
		int w, h;
		pastry::fb_get_dimensions(w,h);
		float aspect = static_cast<float>(h)/static_cast<float>(w);
		float left = center.x() - width*0.5f;
		float right = center.x() + width*0.5f;
		float top = center.y() + width*aspect*0.5f;
		float bottom = center.y() - width*aspect*0.5f;
		proj_ = pastry::math_orthogonal_projection(left, right, top, bottom, -1.0f, +1.0f);
	}

	void addLine(const Eigen::Vector2f& a, const Eigen::Vector2f& b)
	{
		lines_vertices_.push_back({a[0],a[1],color_[0],color_[1],color_[2]});
		lines_vertices_.push_back({b[0],b[1],color_[0],color_[1],color_[2]});
	}

	void update(float t, float dt) {}

	void render()
	{
		vbo.update_data(lines_vertices_); // FIXME should be done in update
		spo.get_uniform<Eigen::Matrix4f>("proj").set(proj_);
		spo.use();
		va.bind();
		glDrawArrays(GL_LINES, 0, lines_vertices_.size());
		lines_vertices_.clear();
	}

private:
	Eigen::Matrix4f proj_;
	Eigen::Vector3f color_;
	std::vector<Vertex> lines_vertices_;
};

std::shared_ptr<Primitives2D> s_primitive_renderling;

void initializePrimitives()
{
	s_primitive_renderling.reset(new Primitives2D());
	scene_add(s_primitive_renderling, 1000000); // FIXME :D
}

void setPrimitiveColor(const Eigen::Vector3f& color)
{
	s_primitive_renderling->setColor(color);
}

void setCamera2D(const Eigen::Vector2f& center, float width)
{
	s_primitive_renderling->setCamera(center, width);
}

void renderLine2D(const Eigen::Vector2f& start, const Eigen::Vector2f& end)
{
	s_primitive_renderling->addLine(start, end);
}

void renderCircle2D(const Eigen::Vector2f& center, float radius)
{
	const int SEGS = 16;
	float delta_angle = 2.0f * M_PI / static_cast<float>(SEGS);
	float cos_angle = std::cos(delta_angle);
	float sin_angle = std::sin(delta_angle);
	float lastx = radius;
	float lasty = 0.0f;
	for(int i=1; i<=SEGS; i++) {
		float x = cos_angle * lastx - sin_angle * lasty;
		float y = sin_angle * lastx + cos_angle * lasty;
		renderLine2D({lastx, lasty}, {x,y});
		lastx = x;
		lasty = y;
	}
}

void renderBox2D(const Eigen::Vector2f& center, float angle, const Eigen::Vector2f& size)
{
	float ca = std::cos(angle);
	float sa = std::sin(angle);
	Eigen::Matrix2f rot;
	rot << ca, -sa, sa, ca;
	float dx = size.x() * 0.5f;
	float dy = size.y() * 0.5f;
	Eigen::Vector2f corners[4] = {
		center + rot * Eigen::Vector2f{-dx, -dy},
		center + rot * Eigen::Vector2f{+dx, -dy},
		center + rot * Eigen::Vector2f{+dx, +dy},
		center + rot * Eigen::Vector2f{-dx, +dy}
	};
	renderLine2D(corners[0], corners[1]);
	renderLine2D(corners[1], corners[2]);
	renderLine2D(corners[2], corners[3]);
	renderLine2D(corners[3], corners[0]);
}

void renderPolygon2D(const std::vector<Eigen::Vector2f>& points)
{
	if(points.size() < 2) {
		return;
	}
	for(size_t i=0; i<points.size()-1; i++) {
		renderLine2D(points[i], points[i+1]);
	}
	renderLine2D(points[points.size()-1], points[0]);
}

}
