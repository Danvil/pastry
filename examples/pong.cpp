
#include <pastry/pastry.hpp>
#include <iostream>
#include <random>

std::default_random_engine s_rnd;

class Dong
: public pastry::renderling
{
public:
	std::vector<Eigen::Vector2f> circle_vertices;
	std::vector<Eigen::Vector2f> box_vertices;
	pastry::array_buffer vbo;
	pastry::program spo;
	pastry::vertex_array vao;

	Eigen::Vector2f ball_pos;
	Eigen::Vector2f ball_vel;
	Eigen::Vector2f box_pos;

	static constexpr float BALL_SPEED = 420.0f;
	static constexpr float R = 15.0f;
	static constexpr int NUM = 32;
	static constexpr float BOX_W = 170.0f;
	static constexpr float BOX_H = 35.0f;

	float width, height;

	float box_speed;

	Dong() {
		width = 512;
		height = 512;

		vbo = pastry::array_buffer{{"pos", GL_FLOAT, 2}};
		vbo.init_data(GL_STATIC_DRAW);

		std::string vertexSource = PASTRY_GLSL(
			in vec2 position;
			uniform mat4 proj;
			uniform mat4 model;
			void main() {
				gl_Position = proj*model*vec4(position, 0.0, 1.0);
			}
		);
		std::string fragmentSource = PASTRY_GLSL(
			out vec4 outColor;
			uniform vec3 color;
			void main() {
				outColor = vec4(color, 1.0);
			}
		);
		spo = pastry::program(vertexSource, fragmentSource);
		spo.use();

		Eigen::Matrix4f proj = pastry::create_orthogonal_projection(width, height, -1.0f, +1.0f);
		spo.get_uniform<Eigen::Matrix4f>("proj").set(proj);

		vao = pastry::vertex_array{spo, {
			{"position", vbo, "pos"}
		}};
		vao.bind();

		// create circle mesh
		for(int i=0; i<=NUM; i++) {
			float phi = 2.0f * 3.1415f * static_cast<float>(i) / static_cast<float>(NUM);
			circle_vertices.push_back({R*std::cos(phi), R*std::sin(phi)});
		}

		// create box mesh
		box_vertices.push_back({0,0});
		box_vertices.push_back({BOX_W,0});
		box_vertices.push_back({BOX_W,BOX_H});
		box_vertices.push_back({0,BOX_H});
		box_vertices.push_back({0,0});

		box_pos = {100, 450};

		start_game();
	}

	void start_game() {
		box_speed = 300.f;
		is_gameover = false;
		ball_pos = {0.5f*width,100};
		std::uniform_real_distribution<float> uniform_dist(-1.0f, +1.0f);
		ball_vel = Eigen::Vector2f{uniform_dist(s_rnd),uniform_dist(s_rnd)}.normalized();
	}

	bool is_initialized = false;
	bool is_gameover = false;
	float is_pong = false;

	void update(float t, float dt)
	{
		if(t < 2.7f) {
			return; // wait to load
		}

		box_speed += 10.0f*dt;
		is_pong -= dt;
		is_initialized = true;

		if(pastry::is_key_pressed('A')) {
			box_pos[0] -= dt*box_speed;
		}
		if(pastry::is_key_pressed('D')) {
			box_pos[0] += dt*box_speed;
		}
		ball_pos += ball_vel * dt * BALL_SPEED;

		if(is_gameover) {
			if(pastry::is_key_pressed('W')) {
				start_game();
			}
			return;
		}

		// handle collision
		if(ball_pos[0] < R && ball_vel[0] < 0) {
			ball_vel[0] *= -1.0f;
		}
		if(ball_pos[0] + R > width && ball_vel[0] > 0) {
			ball_vel[0] *= -1.0f;
		}
		if(ball_pos[1] < R && ball_vel[1] < 0) {
			ball_vel[1] *= -1.0f;
		}
		if(ball_pos[1] + R > box_pos[1] && ball_vel[1] > 0) {
			// this is bottom
			// check box position
			if(box_pos[0] <= ball_pos[0] && ball_pos[0] <= box_pos[0] + BOX_W) {
				is_pong = 0.7f;
				ball_vel[1] *= -1.0f;
			}
			else {
				is_gameover = true;
			}
		}
	}

	void render()
	{
		if(!is_initialized) {
			pastry::render_text(50, 200, "Press 'A' and 'D' to move");
			return;
		}

		if(is_pong > 0.0f) {
			pastry::render_text(200, 220, "PONG");
		}

		if(is_gameover) {
			pastry::render_text(50, 200, "GAME OVER!");
			pastry::render_text(50, 300, "Press 'W' to play again");
		}

		glLineWidth(3.0f);
		spo.use();
		vao.bind();

		spo.get_uniform<Eigen::Matrix4f>("model").set(
			pastry::create_model_matrix_2d(ball_pos[0], ball_pos[1], 0.0f));
		spo.get_uniform<Eigen::Vector3f>("color").set(
			{0.0f,0.4f,1.0f});
		vbo.update_data(circle_vertices);
		glDrawArrays(GL_LINE_STRIP, 0, circle_vertices.size());

		spo.get_uniform<Eigen::Matrix4f>("model").set(
			pastry::create_model_matrix_2d(box_pos[0], box_pos[1], 0.0f));
		spo.get_uniform<Eigen::Vector3f>("color").set(
			{1.0f,0.4f,0.0f});
		vbo.update_data(box_vertices);
		glDrawArrays(GL_LINE_STRIP, 0, box_vertices.size());
	}

};

int main()
{
	pastry::initialize();

	pastry::add_renderling(std::make_shared<Dong>());

	pastry::run();

	return 0;
}
