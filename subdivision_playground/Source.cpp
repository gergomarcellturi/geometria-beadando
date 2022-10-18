#include <array>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>


using namespace std;

extern void cleanUpScene();

GLFWwindow* window;
GLboolean		keyboard[512] = { GL_FALSE };

int				window_width = 600;
int				window_height = 600;
char			window_title[] = "Subdivision";

unsigned int	viewLocation;
unsigned int	modelLocation;
unsigned int	projectionLocation;
unsigned int	invTMatrixLocation;
unsigned int	lightPositionLocation;

std::vector< glm::vec3 > loaded_vertices;
std::vector< glm::vec3 > loaded_normals;

std::vector< glm::vec3 > temp_vertices;
std::vector< glm::vec2 > temp_uvs;
std::vector< glm::vec3 > temp_normals;
std::vector< unsigned int > vertIndices, uvIndices, normIndices;

#define numVBOs	1
#define numVAOs	1

GLuint VBO[numVBOs];
GLuint VAO[numVAOs];
GLuint renderingProgram;

glm::mat4 model, view, projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
glm::mat4 invTmatrix, rotateM, scaleM;

GLdouble currentTime, deltaTime, lastTime = 0.0f;
GLfloat	cameraSpeed;
bool isWireFrame = false;

float starting_vertices[] = {
		-0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,
		 0.25f,  0.25f, -0.25f,
		 0.25f,  0.25f, -0.25f,
		-0.25f,  0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f, 

		-0.25f, -0.25f,  0.25f, 
		 0.25f, -0.25f,  0.25f,  
		 0.25f,  0.25f,  0.25f,
		 0.25f,  0.25f,  0.25f, 
		-0.25f,  0.25f,  0.25f, 
		-0.25f, -0.25f,  0.25f, 

		-0.25f,  0.25f,  0.25f, 
		-0.25f,  0.25f, -0.25f, 
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f, -0.25f,
		-0.25f, -0.25f,  0.25f,
		-0.25f,  0.25f,  0.25f,

		 0.25f,  0.25f,  0.25f,
		 0.25f,  0.25f, -0.25f,  
		 0.25f, -0.25f, -0.25f,  
		 0.25f, -0.25f, -0.25f, 
		 0.25f, -0.25f,  0.25f,
		 0.25f,  0.25f,  0.25f, 

		-0.25f, -0.25f, -0.25f,
		 0.25f, -0.25f, -0.25f,  
		 0.25f, -0.25f,  0.25f, 
		 0.25f, -0.25f,  0.25f, 
		-0.25f, -0.25f,  0.25f, 
		-0.25f, -0.25f, -0.25f, 

		-0.25f,  0.25f, -0.25f, 
		 0.25f,  0.25f, -0.25f,  
		 0.25f,  0.25f,  0.25f, 
		 0.25f,  0.25f,  0.25f, 
		-0.25f,  0.25f,  0.25f, 
		-0.25f,  0.25f, -0.25f,  
};

float starting_normals[] = {
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,
	 0.0f,  0.0f, -1.0f,

	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,
	 0.0f,  0.0f, 1.0f,

	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,
	-1.0f,  0.0f,  0.0f,

	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,
	1.0f,  0.0f,  0.0f,

	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,
	0.0f, -1.0f,  0.0f,

	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
	0.0f,  1.0f,  0.0f,
};

glm::vec3 cameraPosition = glm::vec3(2.0f, 5.0f, 5.0f);
glm::vec3 cameraMovingX = glm::vec3(-1.0f, 0.0f, 0.0f);
glm::vec3 cameraMovingY = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightPosition;

bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}

string readShaderSource(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram() {
	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl");

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}


	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if ((action == GLFW_PRESS) && (key == GLFW_KEY_ESCAPE))
		cleanUpScene();

	if (action == GLFW_PRESS)
		keyboard[key] = GL_TRUE;
	else if (action == GLFW_RELEASE)
		keyboard[key] = GL_FALSE;
}

void computeModelMatrix() {
	model = glm::mat4(1.0f),
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
}

void computeCameraMatrix() {
	view = glm::lookAt(cameraPosition, cameraTarget, cameraUpVector);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
}

bool processFile(FILE* objectFile, char* starting_charachter) {
	if (strcmp(starting_charachter, "v") == 0) {
		glm::vec3 vert;
		int result = fscanf(objectFile, "%f %f %f\n", &vert.x, &vert.y, &vert.z);
		if (result > 0) temp_vertices.push_back(vert);
	}
	else if (strcmp(starting_charachter, "vn") == 0) {
		glm::vec3 norm;
		int result = fscanf(objectFile, "%f %f %f\n", &norm.x, &norm.y, &norm.z);
		if (result > 0) temp_normals.push_back(norm);
	}
	else if (strcmp(starting_charachter, "vt") == 0) {
		glm::vec2 uv;
		int result = fscanf(objectFile, "%f %f\n", &uv.x, &uv.y);
		if (result > 0) temp_uvs.push_back(uv);
	}
	else if (strcmp(starting_charachter, "f") == 0) {
		std::string vert1, vert2, vert3;
		unsigned int vertIndex[3], uvIndex[3], normIndex[3];
		int result = fscanf(objectFile, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
			&vertIndex[0], &uvIndex[0], &normIndex[0], &vertIndex[1], &uvIndex[1], &normIndex[1], &vertIndex[2], &uvIndex[2], &normIndex[2]);
		if (result < 1) {
			printf("File can't be read with this parser \n");
			return false;
		}
		for (int i = 0; i < 3; i++) {
			vertIndices.push_back(vertIndex[i]);
			uvIndices.push_back(uvIndex[i]);
			normIndices.push_back(normIndex[i]);
		}
	}
	return true;
}

bool loadFile() {

	FILE* objectFile = fopen("test.obj", "r");
	if (objectFile == NULL) {
		cout << "Unable to open the file /n";
		return false;
	}

	//Beolvassuk a filet
	while (1) {

		char starting_charachter[256];

		int result = fscanf(objectFile, "%s", starting_charachter);
		if (result == EOF)
			break;

		bool processResult = processFile(objectFile, starting_charachter);
		if (processResult == false) {
			cout << "Failure during file processing";
			return false;
		}
	}

	for (unsigned int i = 0; i < vertIndices.size(); i++) {
		unsigned int indexV = vertIndices[i];
		glm::vec3 vertex = temp_vertices[indexV - 1];
		loaded_vertices.push_back(vertex);
	}

	return true;
}

void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram();

	bool result = loadFile();

	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);

	if (result == false) {
		glBufferData(GL_ARRAY_BUFFER, sizeof(starting_vertices), starting_vertices, GL_STATIC_DRAW);
	}

	else {
		glBufferData(GL_ARRAY_BUFFER, loaded_vertices.size() * sizeof(glm::vec3), &loaded_vertices[0], GL_STATIC_DRAW);
	}

	glBindVertexArray(VAO[0]);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUseProgram(renderingProgram);

	modelLocation = glGetUniformLocation(renderingProgram, "model");
	viewLocation = glGetUniformLocation(renderingProgram, "view");
	projectionLocation = glGetUniformLocation(renderingProgram, "projection");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	invTMatrixLocation = glGetUniformLocation(renderingProgram, "invTMatrix");
	lightPositionLocation = glGetUniformLocation(renderingProgram, "lightPosition");

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glEnable(GL_DEPTH_TEST);
}

void cleanUpScene() {
	glfwDestroyWindow(window);
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);
	glDeleteProgram(renderingProgram);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	currentTime = glfwGetTime();
	deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	cameraSpeed = 5.0f * (GLfloat)deltaTime;

	if ((keyboard[GLFW_KEY_A])) {
		cameraPosition -= cameraSpeed * glm::normalize(glm::cross(cameraPosition - cameraTarget, cameraUpVector));
	}

	if ((keyboard[GLFW_KEY_D])) {
		cameraPosition += cameraSpeed * glm::normalize(glm::cross(cameraPosition - cameraTarget, cameraUpVector));
	}

	if ((keyboard[GLFW_KEY_W])) {
		cameraPosition.y += cameraSpeed;
	}

	if ((keyboard[GLFW_KEY_S])) {
		cameraPosition.y -= cameraSpeed;
	}

	if ((keyboard[GLFW_KEY_F])) {
		isWireFrame = false;
	}
	if ((keyboard[GLFW_KEY_L])) {
		isWireFrame = true;
	}

	lightPosition = cameraPosition;

	computeModelMatrix();
	computeCameraMatrix();

	invTmatrix = glm::inverseTranspose(view * model);
	glUniformMatrix4fv(invTMatrixLocation, 1, GL_FALSE, glm::value_ptr(invTmatrix));
	glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition));
	glBindVertexArray(VAO[0]);

	//Wireframe váltás
	if (isWireFrame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if(loaded_vertices.size()>0) glDrawArrays(GL_TRIANGLES, 0, loaded_vertices.size());
	else glDrawArrays(GL_TRIANGLES, 0, sizeof(starting_vertices));

	glBindVertexArray(0);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;

	glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, keyCallback);

	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	init(window);

	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanUpScene();

	return EXIT_SUCCESS;
}