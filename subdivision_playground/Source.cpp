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
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <filesystem>
#include "mesh.h"

using namespace std;

extern void cleanUpScene();

GLFWwindow* window;
GLboolean		keyboard[512] = { GL_FALSE };
GLboolean		mouse[7] = { GL_FALSE };

int				window_width = 600;
int				window_height = 600;
char			window_title[] = "Subdivision";

unsigned int	viewLocation;
unsigned int	modelLocation;
unsigned int	projectionLocation;
unsigned int	invTMatrixLocation;
unsigned int	lightPositionLocation;
const float RADIUS = 1.0f;
bool flag = false;

#define numVBOs	2
#define numVAOs	1

GLuint VBO[numVBOs];
GLuint VAO[numVAOs];
GLuint renderingProgram;

glm::mat4 model, view, projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
glm::mat4 invTmatrix, rotateM, scaleM;

GLdouble currentTime, deltaTime, lastTime = 0.0f;
GLfloat	cameraSpeed;
double prevMousePosX, prevMousePosY;
int isWireFrame = false;

unsigned int framebuffer;
unsigned int textureColorbuffer;
Mesh* object;
std::vector<std::string> files;
int selectedModel = 0;
int subdivisionSteps = 0;

struct Quaternion {
	float cosine;
	glm::vec3 axis;

};

class ArcballCamera {
public:

	glm::vec3 position = glm::vec3(0.0f, 0.0f, -5.0f);
	glm::vec3 startPos;
	glm::vec3 currentPos = startPos;
	glm::vec3 startPosUnitVector;
	glm::vec3 currentPosUnitVector;

	Quaternion currentQuaternion;
	Quaternion lastQuaternion = { 0.0f, glm::vec3(1.0f, 0.0f, 0.0f) };

	float cosValue, cosValue_2;
	float theta;
	float angle = 180.0f;
	glm::vec3 rotationalAxis = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 rotationalAxis_2;
	ArcballCamera() {};
	float z_axis(float, float);
	glm::vec3 getUnitVector(glm::vec3);
	float dotProduct();
	void rotation();
	void replace();


};

float ArcballCamera::z_axis(float x, float y) {
	float z = 0;
	if (sqrt((x * x) + (y * y)) <= RADIUS) z = (float)sqrt((RADIUS * RADIUS) - (x * x) - (y * y));
	return z;
}

glm::vec3 ArcballCamera::getUnitVector(glm::vec3 vectr) {
	float magnitude1;
	glm::vec3 unitVector;
	magnitude1 = (vectr.x * vectr.x) + (vectr.y * vectr.y) + (vectr.z * vectr.z);
	magnitude1 = sqrt(magnitude1);
	if (magnitude1 == 0) {
		unitVector.x = 0;
		unitVector.y = 0;
		unitVector.z = 0;
	}
	else {
		unitVector.x = vectr.x / magnitude1;
		unitVector.y = vectr.y / magnitude1;
		unitVector.z = vectr.z / magnitude1;
	}
	return unitVector;
}

float ArcballCamera::dotProduct() {
	float result = (startPosUnitVector.x * currentPosUnitVector.x) + (startPosUnitVector.y * currentPosUnitVector.y) + (startPosUnitVector.z * currentPosUnitVector.z);
	return result;
}

void ArcballCamera::rotation() {
	startPosUnitVector = getUnitVector(startPos);
	currentPosUnitVector = getUnitVector(currentPos);
	currentQuaternion.axis = glm::cross(startPos, currentPos);
	currentQuaternion.axis = getUnitVector(currentQuaternion.axis);

	cosValue = dotProduct();
	if (cosValue > 1) cosValue = 1;
	theta = (acos(cosValue) * 180 / 3.1416);
	currentQuaternion.cosine = cos((theta / 2) * 3.1416 / 180);

	currentQuaternion.axis.x = currentQuaternion.axis.x * sin((theta / 2) * 3.1416 / 180);
	currentQuaternion.axis.y = currentQuaternion.axis.y * sin((theta / 2) * 3.1416 / 180);
	currentQuaternion.axis.z = currentQuaternion.axis.z * sin((theta / 2) * 3.1416 / 180);

	cosValue_2 = (currentQuaternion.cosine * lastQuaternion.cosine)
		- glm::dot(currentQuaternion.axis, lastQuaternion.axis);


	glm::vec3 temporaryVector;

	temporaryVector = glm::cross(currentQuaternion.axis, lastQuaternion.axis);


	rotationalAxis_2.x = (currentQuaternion.cosine * lastQuaternion.axis.x) +
		(lastQuaternion.cosine * currentQuaternion.axis.x) +
		temporaryVector.x;

	rotationalAxis_2.y = (currentQuaternion.cosine * lastQuaternion.axis.y) +
		(lastQuaternion.cosine * currentQuaternion.axis.y) +
		temporaryVector.y;

	rotationalAxis_2.z = (currentQuaternion.cosine * lastQuaternion.axis.z) +
		(lastQuaternion.cosine * currentQuaternion.axis.z) +
		temporaryVector.z;

	angle = (acos(cosValue_2) * 180 / 3.1416) * 2;

	rotationalAxis.x = rotationalAxis_2.x / sin((angle / 2) * 3.1416 / 180);
	rotationalAxis.y = rotationalAxis_2.y / sin((angle / 2) * 3.1416 / 180);
	rotationalAxis.z = rotationalAxis_2.z / sin((angle / 2) * 3.1416 / 180);
}

void ArcballCamera::replace() {
	lastQuaternion.cosine = cosValue_2;
	lastQuaternion.axis = rotationalAxis_2;
}


ArcballCamera arcCamera;

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

glm::vec3 get_arcball_vector(int x, int y) {
	glm::vec3 P = glm::vec3(1.0 * x / window_width * 2 - 1.0,
		1.0 * y / window_height * 2 - 1.0,
		0);
	P.y = -P.y;
	float OP_squared = P.x * P.x + P.y * P.y;
	if (OP_squared <= 1 * 1)
		P.z = sqrt(1 * 1 - OP_squared);
	else
		P = glm::normalize(P);
	return P;
}

float zAxis(float x, float y) {
	float radius = 10.0f;
	float z = 0;
	if (sqrt((x * x) + (y * y)) <= radius) z = (float)sqrt((radius * radius) - (x * x) - (y * y));
	return z;
}

glm::vec3 getUnitVector(glm::vec3 vectr) {
	float magnitude1;
	glm::vec3 unitVector;
	magnitude1 = (vectr.x * vectr.x) + (vectr.y * vectr.y) + (vectr.z * vectr.z);
	magnitude1 = sqrt(magnitude1);
	if (magnitude1 == 0) {
		unitVector.x = 0;
		unitVector.y = 0;
		unitVector.z = 0;
	}
	else {
		unitVector.x = vectr.x / magnitude1;
		unitVector.y = vectr.y / magnitude1;
		unitVector.z = vectr.z / magnitude1;
	}
	return unitVector;
}

void computeCameraMatrix() {

	view = glm::mat4(1.0f);
	view = glm::translate(view, arcCamera.position);
	view = glm::rotate(view, glm::radians(arcCamera.angle), arcCamera.rotationalAxis);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
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

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
	if (flag == true) {
		arcCamera.currentPos.x = ((xPos - (window_width / 2)) / (window_width/ 2)) * RADIUS;
		arcCamera.currentPos.y = (((window_height / 2) - yPos) / (window_height/ 2)) * RADIUS;
		arcCamera.currentPos.z = arcCamera.z_axis(arcCamera.currentPos.x, arcCamera.currentPos.y);
		arcCamera.rotation();
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

		double startXPos, startYPos;
		glfwGetCursorPos(window, &startXPos, &startYPos);
		arcCamera.startPos.x = ((startXPos - (window_width / 2)) / (window_width / 2)) * RADIUS;
		arcCamera.startPos.y = (((window_height / 2) - startYPos) / (window_height / 2)) * RADIUS;
		arcCamera.startPos.z = arcCamera.z_axis(arcCamera.startPos.x, arcCamera.startPos.y);
		flag = true;
	}
	else if (action == GLFW_RELEASE) {
		arcCamera.replace();
		flag = false;

	}
}

void mouseScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
	arcCamera.position.z += yOffset * 0.5f;
}

void computeModelMatrix() {
	model = glm::mat4(1.0f),
	glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
}

void init(GLFWwindow* window) {
	std::string path = "models";
	for (const auto& entry : std::filesystem::directory_iterator(path))
		files.push_back(std::filesystem::path(entry.path()).filename().string());

	renderingProgram = createShaderProgram();

	GLuint rboId;
	glGenRenderbuffers(1, &rboId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_width, window_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebuffer);

	object = new Mesh();
	object->subdivision("models/" + files.at(selectedModel), subdivisionSteps);

	glGenBuffers(numVBOs, VBO);
	glGenVertexArrays(numVAOs, VAO);
	
	glBindVertexArray(VAO[0]);
	
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, object->vertexTriagnleList.size() * sizeof(glm::vec3), &object->vertexTriagnleList[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, object->normals.size() * sizeof(glm::vec3), &object->normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

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

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
	computeCameraMatrix();
}

void cleanUpScene() {
	delete object;
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glDeleteVertexArrays(numVAOs, VAO);
	glDeleteBuffers(numVBOs, VBO);
	glDeleteProgram(renderingProgram);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	window_width = width;
	window_height = height;

	glViewport(0, 0, width, height);

	projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 0.1f, 100.0f);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
}

void handleFrambufferResize(int width, int height)
{
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	framebufferSizeCallback(window, width, height);
}

void bufferData()
{
	glBindVertexArray(VAO[0]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, object->vertexTriagnleList.size() * sizeof(glm::vec3), &object->vertexTriagnleList[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, object->normals.size() * sizeof(glm::vec3), &object->normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void loadModel()
{
	subdivisionSteps = 0;
	delete object;
	object = new Mesh();
	object->subdivision("models/" + files.at(selectedModel), subdivisionSteps);

	bufferData();
}

void display() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
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

	object->draw();

	glBindVertexArray(0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	bool* p_open = NULL;

	bool opt_fullscreen = true;
	bool opt_padding = false;
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_AutoHideTabBar;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}
	else
	{
		dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
	}


	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags = ImGuiWindowFlags_NoDecoration;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace", p_open, window_flags);
	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("DockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Scene");
	ImGui::PopStyleVar();
	ImVec2 wsize = ImGui::GetWindowSize();
	handleFrambufferResize(wsize.x, wsize.y);
	ImGui::Image((ImTextureID)textureColorbuffer, wsize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::End();

	ImGui::Begin("tab");
	const char* options[]{"Fill", "Wireframe"};
	ImGui::Combo("polygon mode", &isWireFrame, options, 2);
	if (isWireFrame == 0)
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	int steps = subdivisionSteps;
	ImGui::SliderInt("subdivision steps", &subdivisionSteps, 0, 5);
	if (steps != subdivisionSteps)
	{
		object->subdivision("models/" + files.at(selectedModel), subdivisionSteps);
		bufferData();
	}

	if (ImGui::TreeNode("Models"))
	{
		for (int n = 0; n < files.size(); n++)
		{
			char buf[256];
			sprintf(buf, files.at(n).c_str(), n);
			if (ImGui::Selectable(buf, selectedModel == n))
			{
				if (selectedModel != n)
				{
					selectedModel = n;
					loadModel();
				}
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();

	ImGui::EndFrame();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	GLFWwindow* backup_current_context = glfwGetCurrentContext();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
	glfwMakeContextCurrent(backup_current_context);
}

int main(void) {
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	window = glfwCreateWindow(window_width, window_height, window_title, nullptr, nullptr);

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);

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
