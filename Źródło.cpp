#include <iostream>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <GL/glew.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/System/Time.hpp>
#include <string>
#include <time.h>
#include <SOIL/SOIL.h>
#include "Cube.h"
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

const GLchar* vertexSource = R"glsl(
#version 150 core
in vec3 position;
in vec3 color;
in vec2 texcoord;

out vec2 Texcoord;
out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
void main(){
Color = color;
Texcoord = texcoord;
gl_Position = proj * view * model * vec4(position,1.0);
}
)glsl";

const GLchar* fragmentSource = R"glsl(
#version 150 core
in vec3 Color;
in vec2 Texcoord;

out vec4 outColor;
uniform sampler2D tex;
void main()
{
outColor = texture(tex, Texcoord);
}
)glsl";

void cubeCollideCube(glm::vec3 &v1, glm::vec3 &v2) {
	float v3, v4, delta;

	/*std::cout << "cube 1: (" << v1[0] << "," << v1[1] << "," << v1[2] << ")" << std::endl;
	std::cout << "cube 2: (" << v2[0] << "," << v1[1] << "," << v1[2] << ")" << std::endl;*/

	for (int i = 0; i < 3; i++) {
		delta = 4 * (v1[i] * v1[i] - 2 * v1[i] * v2[i] + v2[i] * v2[i]);
		v3 = (2 * (v1[i] + v2[i]) - sqrtf(delta)) / 4;
		v4 = (2 * (v1[i] + v2[i]) + sqrtf(delta)) / 4;
		//ten sam kierunek i v1 > v2 przed, to ten sam kierunek i v1 < v2 po
		if (fabs(v1[i]) > fabs(v2[i]) && ((v1[i] > 0 && v2[i]>0) || (v1[i]<0 &&v2[i]<0))) {
			if (v3 > v4) {
				v1[i] = v4;
				v2[i] = v3;
			}
			else {
				v1[i] = v3;
				v2[i] = v4;
			}
		}
		//ten sam kierunek i v1 < v2 przed, to ten sam kierunek i v1 > v2 po
		else if (fabs(v1[i]) < fabs(v2[i]) && ((v1[i] > 0 && v2[i]) > 0 || (v1[i] < 0 && v2[i] < 0))) {
			if (v3 > v4) {
				v1[i] = v3;
				v2[i] = v4;
			}
			else {
				v1[i] = v4;
				v2[i] = v3;
			}
		}
		//przeciwne kierunki i v1 > v2 przed, to 
		else if (fabs(v1[i]) > fabs(v2[i]) && ((v1[i] > 0 && v2[i] < 0) || (v1[i] < 0 && v2[i] > 0))) {
			if (v3 > v4) {
				v1[i] = v4;
				v2[i] = v3;
			}
			else {
				v1[i] = v3;
				v2[i] = v4;
			}
		}
		else if (fabs(v1[i]) < fabs(v2[i]) && ((v1[i] > 0 && v2[i] < 0) || (v1[i] < 0 && v2[i] > 0))) {
			if (v3 > v4) {
				v1[i] = v3;
				v2[i] = v4;
			}
			else {
				v1[i] = v4;
				v2[i] = v3;
			}
		}
		else if (v1[i] == v2[i] && ((v1[i] > 0 && v2[i] < 0) || (v1[i] < 0 && v2[i] > 0))) {
			v1[i] = -v1[i];
			v2[i] = -v2[i];
		}
		/*delta = 4 * (v1[i] * v1[i] - 2 * v1[i] * v2[i] + v2[i] * v2[i]);
		v3 = (2 * (v1[i] + v2[i]) - sqrtf(delta)) / 4;
		v4 = (2 * (v1[i] + v2[i]) + sqrtf(delta)) / 4;
		v1[i] = v3;
		v2[i] = v4;*/
	}
}

int main()
{
	unsigned int fps = 30;

	srand(time(NULL));

	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;

	// Okno renderingu
	sf::Window window(sf::VideoMode(800, 800, 32), std::to_string(fps), sf::Style::Titlebar | sf::Style::Close, settings);
	window.setFramerateLimit(fps);

	glEnable(GL_DEPTH_TEST);

	// Inicjalizacja GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	// Utworzenie VAO (Vertex Array Object)
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Utworzenie VBO (Vertex Buffer Object)
	// i skopiowanie do niego danych wierzcho³kowych
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// Utworzenie i skompilowanie shadera wierzcho³ków
	GLuint vertexShader =
		glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Utworzenie i skompilowanie shadera fragmentów
	GLuint fragmentShader =
		glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Zlinkowanie obu shaderów w jeden wspólny program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor");
	glLinkProgram(shaderProgram);

	glUseProgram(shaderProgram);

	// Specifikacja formatu danych wierzcho³kowych
	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	GLint texAttrib = glGetAttribLocation(shaderProgram, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_2D);
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* image = stbi_load("dirt.png", &width, &height, &nrChannels, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(image);

	GLfloat vertices[] = {
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,1.2f, 1.0f, 2.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,1.2f, 1.0f, 2.0f
	};

	GLfloat verticesBox[] = {
		//A
		0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,

		//B
		0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,

		//C
		20.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,

		//D
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,

		//E
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,

		//F
		0.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		20.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 0.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.5f,
		0.0f, 20.0f, 20.0f, 1.0f, 1.0f, 0.0f, 0.5f,
	};

	unsigned int numOfCubes = 100;

	glm::vec3 cubeFinalPositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  -1.0f, 0.0f),
		glm::vec3(0.0f,  -2.0f, 0.0f),
		glm::vec3(-1.0f,  -2.0f, 0.0f),
		glm::vec3(-1.0f, -1.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-2.0f, 0.0f, 0.0f),
		glm::vec3(-2.0f, -1.0f, 0.0f),
		glm::vec3(-2.0f, -2.0f, 0.0f),

		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f,  -1.0f, -1.0f),
		glm::vec3(0.0f,  -2.0f, -1.0f),
		glm::vec3(-1.0f,  -2.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f,  0.0f, -1.0f),
		glm::vec3(-2.0f,  0.0f, -1.0f),
		glm::vec3(-2.0f,  -1.0f, -1.0f),
		glm::vec3(-2.0f,  -2.0f, -1.0f),

		glm::vec3(0.0f, 0.0f, -2.0f),
		glm::vec3(0.0f,  -1.0f, -2.0f),
		glm::vec3(0.0f,  -2.0f, -2.0f),
		glm::vec3(-1.0f,  -2.0f, -2.0f),
		glm::vec3(-1.0f, -1.0f, -2.0f),
		glm::vec3(-1.0f,  0.0f, -2.0f),
		glm::vec3(-2.0f,  0.0f, -2.0f),
		glm::vec3(-2.0f,  -1.0f, -2.0f),
		glm::vec3(-2.0f,  -2.0f, -2.0f)
	};

	float up = 0.1f;
	float low = -0.1f;

	Cube cubes[100];
	
	for (int i = 0; i < numOfCubes; i++) {
		cubes[i].position = glm::vec3((float)rand() / (float)(RAND_MAX)*up*190, (float)rand() / (float)(RAND_MAX)*up*190, (float)rand() / (float)(RAND_MAX)*up*190);
		cubes[i].mass = 0.0f;
		cubes[i].velocity = glm::vec3(low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (up - low))),
			low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (up - low))),
			low + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (up - low))));
		cubes[i].center = glm::vec3(cubes[i].position.x + 0.5f, cubes[i].position.y + 0.5f, cubes[i].position.z + 0.5f);
	}

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	glm::mat4 view;
	view = glm::lookAt(glm::vec3(3.0f, 5.0f, 3.0f), //position,target,up vector
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);

	GLint uniTrans = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
	glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	int primitive_type = GL_TRIANGLES;

	bool rotation = false;
	bool move = false;

	float obrotX = 0.0;
	float cameraSpeed = 0.06;

	float j = 0.01;

	window.setMouseCursorGrabbed(true);
	window.setMouseCursorVisible(false);

	sf::Clock clock;
	sf::Time time;

	double yaw = -90;
	double pitch = 0;
	double xoffset, yoffset;
	double lastX = 400;
	double lastY = 400;

	float sensitivity = 0.5f;

	glm::vec3 front;

	bool firstTime = true;

	float fov = 45.0f;

	float k = 0;

	// Rozpoczêcie pêtli zdarzeñ
	bool running = true;
	while (running) {

		time = clock.getElapsedTime();
		clock.restart();

		cameraSpeed = 0.002f * (float)time.asMilliseconds();

		sf::Event windowEvent;

		//sf::Mouse::setPosition(sf::Vector2i(400, 400), window);
		sf::Vector2i localPosition = sf::Mouse::getPosition(window);


		while (window.pollEvent(windowEvent)) {

			switch (windowEvent.type) {
			case sf::Event::Closed:
				running = false;
				break;
			case ::sf::Event::MouseMoved:

				if (firstTime) {
					lastX = localPosition.x;
					lastY = localPosition.y;
					firstTime = false;

				}

				xoffset = localPosition.x - lastX;
				yoffset = localPosition.y - lastY;
				lastX = localPosition.x;
				lastY = localPosition.y;

				xoffset *= sensitivity;
				yoffset *= sensitivity;
				yaw += xoffset;
				pitch -= yoffset;

				if (pitch > 89.0f)
					pitch = 89.0f;
				if (pitch < -89.0f)
					pitch = -89.0f;

				front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
				front.y = sin(glm::radians(pitch));
				front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
				cameraDirection = glm::normalize(front);



				break;
			case sf::Event::KeyPressed:
				switch (windowEvent.key.code) {
				case sf::Keyboard::Escape:
					running = false;
					break;
				case sf::Keyboard::Left:
					obrotX -= cameraSpeed;
					cameraDirection.x = sin(obrotX);
					cameraDirection.z = -cos(obrotX);
					break;
				case sf::Keyboard::Right:
					obrotX += cameraSpeed;
					cameraDirection.x = sin(obrotX);
					cameraDirection.z = -cos(obrotX);
					break;
				case sf::Keyboard::W:
					cameraPos += 4 * cameraSpeed * cameraDirection;
					break;
				case sf::Keyboard::S:
					cameraPos -= 4 * cameraSpeed * cameraDirection;
					break;
				case sf::Keyboard::A:
					cameraPos -= glm::normalize(glm::cross(cameraDirection, cameraUp)) * (cameraSpeed * 4);
					break;
				case sf::Keyboard::D:
					cameraPos += glm::normalize(glm::cross(cameraDirection, cameraUp)) * (cameraSpeed * 4);
					break;
				case sf::Keyboard::Num0:
					primitive_type = GL_POINTS;
					std::cout << "GL_POINTS" << std::endl;
					break;
				case sf::Keyboard::Num1:
					primitive_type = GL_LINES;
					std::cout << "GL_LINES" << std::endl;
					break;
				case sf::Keyboard::Num2:
					primitive_type = GL_LINE_STRIP;
					std::cout << "GL_LINE_STRIP" << std::endl;
					break;
				case sf::Keyboard::Num3:
					primitive_type = GL_LINE_LOOP;
					std::cout << "GL_LINE_LOOP" << std::endl;
					break;
				case sf::Keyboard::Num4:
					primitive_type = GL_TRIANGLES;
					std::cout << "GL_TRIANGLES" << std::endl;
					break;
				case sf::Keyboard::Num5:
					primitive_type = GL_TRIANGLE_STRIP;
					std::cout << "GL_TRIANGLE_STRIP" << std::endl;
					break;
				case sf::Keyboard::Num6:
					primitive_type = GL_TRIANGLE_FAN;
					std::cout << "GL_TRIANGLE_FAN" << std::endl;
					break;
				case sf::Keyboard::Num7:
					primitive_type = GL_QUADS;
					std::cout << "GL_QUADS" << std::endl;
					break;
				case sf::Keyboard::Num8:
					primitive_type = GL_QUAD_STRIP;
					std::cout << "GL_QUAD_STRIP" << std::endl;
					break;
				case sf::Keyboard::Num9:
					primitive_type = GL_POLYGON;
					std::cout << "GL_POLYGON" << std::endl;
					break;
				case sf::Keyboard::R:
					rotation = !rotation;
					break;
				case sf::Keyboard::G:
					move = !move;
				}
			}
		}



		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

		for (unsigned int i = 0; i < numOfCubes; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);

			//translacja 
			
			model = glm::translate(model, cubes[i].position);

			if (rotation) {
				float angle = 10.0f * i + j;
				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
				j += 0.01;
			}
			else if (!rotation && !move) {
				float angle = 10.0f * i + j;
				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			}
			if (move) {

				cubes[i].position.x += cubes[i].velocity.x;
				cubes[i].position.y += cubes[i].velocity.y;
				cubes[i].position.z += cubes[i].velocity.z;

				cubes[i].center.x = cubes[i].position.x + 0.5f;
				cubes[i].center.y = cubes[i].position.y + 0.5f;
				cubes[i].center.z = cubes[i].position.z + 0.5f;

				//collision cube-cube
				for (unsigned int j = 0; j < numOfCubes; j++) {
					float distance = sqrtf((cubes[j].center.x - cubes[i].center.x) * 
						(cubes[j].center.x - cubes[i].center.x) + (cubes[j].center.y - cubes[i].center.y) * 
						(cubes[j].center.y - cubes[i].center.y) + (cubes[j].center.z - cubes[i].center.z) * 
						(cubes[j].center.z - cubes[i].center.z));
					if (distance < sqrtf(3.0f)) {
						cubeCollideCube(cubes[i].velocity, cubes[j].velocity);
					}
				}
				//collision cube-wall
				if (cubes[i].center.x >= 19.5 || cubes[i].center.x < 0.5) {
					cubes[i].velocity.x = -cubes[i].velocity.x;
				}
				else if (cubes[i].center.y >= 19.5 || cubes[i].center.y < 0.5) {
					cubes[i].velocity.y = -cubes[i].velocity.y;
				}
				else if (cubes[i].center.z >= 19.5 || cubes[i].center.z < 0.5) {
					cubes[i].velocity.z = -cubes[i].velocity.z;
				}
				

			}

			glUniformMatrix4fv(uniTrans, 1, GL_FALSE, glm::value_ptr(model));

			glDrawArrays(primitive_type, 0, 36);
		}

		glDrawArrays(primitive_type, 0, 36);
		window.display();

	}
	// Kasowanie programu i czyszczenie buforów
	glDeleteProgram(shaderProgram);
	glDeleteShader(fragmentShader);
	glDeleteShader(vertexShader);
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
	// Zamkniêcie okna renderingu
	window.close();
	return 0;
}