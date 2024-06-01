#include "graphics/graphics_opengl.h"
#include "graphics/pipeline/rp_clay_opengl.h"
#include "graphics/pipeline/rp_temp_opengl.h"
#include "graphics/pipeline/rp_deferred_opengl.h"
#include "graphics/pipeline/rp_forward_opengl.h"
#include "io/callbacks_glfw.h"

#include <sstream>
#include <fstream>
#include <iostream>

// TODO: Shader errors currently rely on MessageBox from Windows.h.
// When a proper logging system has been implemented, switch to that.
#include <Windows.h>


Graphics_OpenGL::Graphics_OpenGL() {
	this->backend = Graphics::Backend::OPENGL;

	glfwDefaultWindowHints();
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	this->window = glfwCreateWindow(512, 512, "Hello World!", NULL, NULL);
	if (!this->window) {
		return;
	}
	glfwMakeContextCurrent(this->window);

	// TODO: Track whether GLEW is initialized?
	glewInit();

	glGetIntegerv(GL_MAJOR_VERSION, &this->GLmajorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &this->GLminorVersion);
	auto vendor = glGetString(GL_VENDOR);
	auto renderer = glGetString(GL_RENDERER);
	this->gpuName = std::string((char*)vendor) + " " + std::string((char*)renderer);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_TEXTURE_2D);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

Graphics_OpenGL::~Graphics_OpenGL() {
	glfwTerminate();
	this->window = nullptr;
};

std::string Graphics_OpenGL::getBackendString() {
	return "OpenGL " + std::to_string(this->GLmajorVersion) +
		"." + std::to_string(this->GLminorVersion);
}
std::string Graphics_OpenGL::getGPUNameString() {
	return this->gpuName;
}

void Graphics_OpenGL::setRenderPipeline(RenderPipelineType pipelineType) {
	if (this->pipeline) {
		delete this->pipeline;
		this->pipeline = nullptr;
	}
	switch (pipelineType) {
	case RenderPipelineType::Clay:
		this->pipeline = new RP_Clay_OpenGL(*this);
		break;
	case RenderPipelineType::Temp:
		this->pipeline = new RP_Temp_OpenGL(*this);
		break;
	case RenderPipelineType::Deferred:
		this->pipeline = new RP_Deferred_OpenGL(*this);
		break;
	case RenderPipelineType::Forward:
		this->pipeline = new RP_Forward_OpenGL(*this);
		break;
	default:
		this->pipeline = nullptr;
		break;
	}
	if (this->pipeline) {
		this->pipeline->init();
	}
}

void Graphics_OpenGL::resizeFramebuffer(size_t width, size_t height) {
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	if (this->pipeline) {
		this->pipeline->resizeFramebuffer(width, height);
	}
}

GPUMesh* Graphics_OpenGL::createMesh() {
	return new GPUMesh_OpenGL();
}
GPUTexture* Graphics_OpenGL::createTexture(Texture* thisTexture) {
	return new GPUTexture_OpenGL(thisTexture);
}

bool Graphics_OpenGL::createWindow(
	std::string window_title,
	size_t width,
	size_t height,
	bool fullscreen
) {
	glfwSetWindowTitle(this->window, window_title.c_str());
	glfwSetWindowSize(this->window, (int)width, (int)height);
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	if (!this->pipeline) {
		this->setRenderPipeline(DefaultRenderPipeline);
		if (!this->pipeline) {
			return false;
		}
		this->pipeline->resizeFramebuffer(width, height);
	}

	// Get the full width of the window.
	int fwidth = 0;
	int fheight = 0;
	glfwGetWindowSize(this->window, &fwidth, &fheight);

	// Center the window.
	const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(
		this->window,
		(vidmode->width - fwidth) / 2,
		(vidmode->height - fheight) / 2
	);

	glfwMakeContextCurrent(this->window);

	// TODO: Make VSync into a setting.
	glfwSwapInterval(0);

	glfwShowWindow(this->window);
	if (fullscreen) {
		glfwSetWindowMonitor(
			this->window, glfwGetPrimaryMonitor(),
			0, 0, vidmode->width, vidmode->height, GLFW_DONT_CARE
		);
	}

	glfwGetFramebufferSize(this->window, &fwidth, &fheight);
	this->resizeFramebuffer((size_t)fwidth, (size_t)fheight);

	return false;
}

void Graphics_OpenGL::destroyWindow() {
	glfwHideWindow(this->window);
}


bool Graphics_OpenGL::pollEvents() {
	glfwPollEvents();
	return !glfwWindowShouldClose(this->window);
}

void Graphics_OpenGL::swapBuffers() {
	glfwSwapBuffers(this->window);
}


/*
* ===== GPUMesh =====
*/

GPUMesh_OpenGL::~GPUMesh_OpenGL() {
	this->deleteVAO();
}

bool GPUMesh_OpenGL::uploadFrom(const Mesh& mesh) {
	auto& v = mesh.getVertices();
	auto& i = mesh.getIndices();
	return this->setupVAO(v.size(), i.size(), v.data(), i.data());
}

void GPUMesh_OpenGL::draw() {
	if (!this->VAO) {
		return;
	}
	glBindVertexArray(this->VAO);
	// TODO: Bind textures.
	glDrawElements(GL_TRIANGLES, (GLsizei)this->numIdxs, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

bool GPUMesh_OpenGL::setupVAO(
	size_t numVerts, size_t numIdxs, const Vertex* verts, const VertexIndex* idxs
) {
	this->deleteVAO();

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);
	glBindVertexArray(this->VAO);
	
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, numVerts * sizeof(Vertex), verts, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIdxs * sizeof(VertexIndex), idxs, GL_STATIC_DRAW);
	this->numIdxs = numIdxs;

	glVertexAttribPointer((GLuint)Graphics_OpenGL::VertAttribs::Position,
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, position));
	glVertexAttribPointer((GLuint)Graphics_OpenGL::VertAttribs::Normal,
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
	glVertexAttribPointer((GLuint)Graphics_OpenGL::VertAttribs::Tangent,
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, tangent));
	glVertexAttribPointer((GLuint)Graphics_OpenGL::VertAttribs::Bitangent,
		3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, bitangent));
	glVertexAttribPointer((GLuint)Graphics_OpenGL::VertAttribs::UV,
		2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, uv));

	glEnableVertexAttribArray((GLuint)Graphics_OpenGL::VertAttribs::Position);
	glEnableVertexAttribArray((GLuint)Graphics_OpenGL::VertAttribs::Normal);
	glEnableVertexAttribArray((GLuint)Graphics_OpenGL::VertAttribs::Tangent);
	glEnableVertexAttribArray((GLuint)Graphics_OpenGL::VertAttribs::Bitangent);
	glEnableVertexAttribArray((GLuint)Graphics_OpenGL::VertAttribs::UV);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// TODO: Check errors.
	return true;
}

void GPUMesh_OpenGL::deleteVAO() {
	if (glIsVertexArray(this->VAO)) {
		glDeleteVertexArrays(1, &this->VAO);
		this->VAO = 0;
	}
	if (glIsBuffer(this->VBO)) {
		glDeleteBuffers(1, &this->VBO);
		this->VBO = 0;
	}
	if (glIsBuffer(this->EBO)) {
		glDeleteBuffers(1, &this->EBO);
		this->EBO = 0;
	}
	this->numIdxs = 0;
}


/*
* ===== GPUTexture =====
*/

GPUTexture_OpenGL::GPUTexture_OpenGL(Texture* thisTexture) : GPUTexture(thisTexture) {}

GPUTexture_OpenGL::~GPUTexture_OpenGL() {
	if (glIsTexture(this->texID)) {
		glDeleteTextures(1, &this->texID);
	}
}

bool GPUTexture_OpenGL::upload(
	uint8_t* data,
	size_t width,
	size_t height,
	size_t numChannels
) {
	size_t oldWidth = this->thisTexture->getWidth();
	size_t oldHeight = this->thisTexture->getHeight();
	size_t oldNumChannels = this->thisTexture->getNumChannels();
	GLenum format = numChannelsToFormat(numChannels);

	if (oldWidth != width || oldHeight != height || oldNumChannels != numChannels) {
		if (glIsTexture(this->texID)) {
			glDeleteTextures(1, &this->texID);
		}
		glGenTextures(1, &this->texID);
		glBindTexture(GL_TEXTURE_2D, this->texID);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			format,
			(GLsizei)width,
			(GLsizei)height,
			0,
			format,
			GL_UNSIGNED_BYTE,
			data
		);
	}
	else {
		glBindTexture(GL_TEXTURE_2D, this->texID);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			(GLsizei)width, (GLsizei)height, format, GL_UNSIGNED_BYTE, data
		);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;
}

GLuint GPUTexture_OpenGL::getGLTexID() {
	return this->texID;
}

GLenum GPUTexture_OpenGL::numChannelsToFormat(size_t numChannels) {
	switch (numChannels) {
	case 1: return GL_RED;
	case 2: return GL_RG;
	case 3: return GL_RGB;
	case 4: return GL_RGBA;
	}
	return 0;
}




Shader_OpenGL::Shader_OpenGL() {}
Shader_OpenGL::Shader_OpenGL(const Shader_OpenGL& shader) {
	*this = shader;
}
Shader_OpenGL::Shader_OpenGL(Shader_OpenGL&& shader) {
	*this = shader;
}
Shader_OpenGL& Shader_OpenGL::operator=(const Shader_OpenGL& shader) {
	this->programID = shader.programID;
	this->uniforms = shader.uniforms;
	return *this;
}
Shader_OpenGL& Shader_OpenGL::operator=(Shader_OpenGL&& shader) {
	this->programID = shader.programID;
	this->uniforms = std::move(shader.uniforms);
	shader.programID = 0;
	shader.uniforms.clear();
	return operator=((const Shader_OpenGL&)shader);
}

void Shader_OpenGL::bind() {
	glUseProgram(this->programID);
}


void Shader_OpenGL::read(std::filesystem::path vertPath) {
	std::stringstream vertcode;
	std::string line;
	std::ifstream file;

	file.open(vertPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Vert Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		vertcode << line << "\n";
	}
	file.close();
	this->compile(vertcode.str());
}
void Shader_OpenGL::read(std::filesystem::path vertPath, std::filesystem::path fragPath) {
	std::stringstream vertcode;
	std::stringstream fragcode;
	std::string line;
	std::ifstream file;

	file.open(vertPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Vert Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		vertcode << line << "\n";
	}
	file.close();
	file.open(fragPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Frag Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		fragcode << line << "\n";
	}
	file.close();
	this->compile(vertcode.str(), fragcode.str());
}
void Shader_OpenGL::read(std::filesystem::path vertPath, std::filesystem::path geomPath, std::filesystem::path fragPath) {
	std::stringstream vertcode;
	std::stringstream geomcode;
	std::stringstream fragcode;
	std::string line;
	std::ifstream file;

	file.open(vertPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Vert Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		vertcode << line << "\n";
	}
	file.close();
	file.open(geomPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Geom Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		geomcode << line << "\n";
	}
	file.close();
	file.open(fragPath);
	if (!file.is_open()) {
		MessageBoxA(NULL, "Could not find file", "Frag Shader", MB_OK | MB_ICONERROR);
	}
	while (std::getline(file, line)) {
		fragcode << line << "\n";
	}
	file.close();
	this->compile(vertcode.str(), geomcode.str(), fragcode.str());
}


void Shader_OpenGL::compile(std::string vertCode) {
	this->clear();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	// We must extract the pointers so we can pass a multi-dim array.
	const char* vc = vertCode.c_str();
	glShaderSource(vs, 1, &vc, NULL);
	glCompileShader(vs);
	bool success = this->checkShaderErrors(vs, "Vertex Shader");
	if (success) {
		this->programID = glCreateProgram();
		glAttachShader(this->programID, vs);
		glLinkProgram(this->programID);
	}
	else {
		this->programID = 0;
	}
	glDeleteShader(vs);
}
void Shader_OpenGL::compile(std::string vertCode, std::string fragCode) {
	this->clear();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	// We must extract the pointers so we can pass a multi-dim array.
	const char* vc = vertCode.c_str();	
	const char* fc = fragCode.c_str();
	glShaderSource(vs, 1, &vc, NULL);
	glCompileShader(vs);
	glShaderSource(fs, 1, &fc, NULL);
	glCompileShader(fs);
	bool success = this->checkShaderErrors(vs, "Vertex Shader") &&
		this->checkShaderErrors(fs, "Fragment Shader");
	if (success) {
		this->programID = glCreateProgram();
		glAttachShader(this->programID, vs);
		glAttachShader(this->programID, fs);
		glLinkProgram(this->programID);
	}
	else {
		this->programID = 0;
	}
	glDeleteShader(vs);
	glDeleteShader(fs);
}
void Shader_OpenGL::compile(std::string vertCode, std::string geomCode, std::string fragCode) {
	this->clear();
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint gs = glCreateShader(GL_GEOMETRY_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	// We must extract the pointers so we can pass a multi-dim array.
	const char* vc = vertCode.c_str();
	const char* gc = geomCode.c_str();
	const char* fc = fragCode.c_str();
	glShaderSource(vs, 1, &vc, NULL);
	glShaderSource(gs, 1, &gc, NULL);
	glShaderSource(fs, 1, &fc, NULL);
	glCompileShader(vs);
	glCompileShader(gs);
	glCompileShader(fs);
	bool success = this->checkShaderErrors(vs, "Vertex Shader") &&
		this->checkShaderErrors(gs, "Geometry Shader") &&
		this->checkShaderErrors(fs, "Fragment Shader");
	if (success) {
		this->programID = glCreateProgram();
		glAttachShader(this->programID, vs);
		glAttachShader(this->programID, gs);
		glAttachShader(this->programID, fs);
		glLinkProgram(this->programID);
	}
	else {
		this->programID = 0;
	}
	glDeleteShader(vs);
	glDeleteShader(gs);
	glDeleteShader(fs);
}

void Shader_OpenGL::clear() {
	if (glIsProgram(this->programID)) {
		glDeleteProgram(this->programID);
	}
	this->programID = 0;
	this->uniforms.clear();
}

GLuint Shader_OpenGL::getID() {
	return this->programID;
}

GLuint Shader_OpenGL::getUniformLocation(std::string name) {
	if (this->uniforms.count(name)) {
		return this->uniforms[name];
	}
	else {
		GLuint loc = glGetUniformLocation(this->programID, name.c_str());
		this->uniforms[name] = loc;
		return loc;
	}
}

void Shader_OpenGL::setUniform1f(std::string name, GLfloat value) {
	glUniform1f(this->getUniformLocation(name), value);
}
void Shader_OpenGL::setUniform2f(std::string name, glm::vec2 value) {
	glUniform2f(this->getUniformLocation(name), value.x, value.y);
}
void Shader_OpenGL::setUniform3f(std::string name, glm::vec3 value) {
	glUniform3f(this->getUniformLocation(name), value.x, value.y, value.z);
}
void Shader_OpenGL::setUniform4f(std::string name, glm::vec4 value) {
	glUniform4f(this->getUniformLocation(name), value.x, value.y, value.z, value.w);
}
void Shader_OpenGL::setUniform1i(std::string name, GLint value) {
	glUniform1i(this->getUniformLocation(name), value);
}
void Shader_OpenGL::setUniform2i(std::string name, glm::ivec2 value) {
	glUniform2i(this->getUniformLocation(name), value.x, value.y);
}
void Shader_OpenGL::setUniform3i(std::string name, glm::ivec3 value) {
	glUniform3i(this->getUniformLocation(name), value.x, value.y, value.z);
}
void Shader_OpenGL::setUniform4i(std::string name, glm::ivec4 value) {
	glUniform4i(this->getUniformLocation(name), value.x, value.y, value.z, value.w);
}
void Shader_OpenGL::setUniformMat4(std::string name, glm::mat4 value) {
	glUniformMatrix4fv(this->getUniformLocation(name), 1, GL_FALSE, &value[0][0]);
}
void Shader_OpenGL::setUniformTex(std::string name, GLuint texID, GLuint index, GLenum type) {
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(type, texID);
	glUniform1i(this->getUniformLocation(name), (GLint)index);
}

bool Shader_OpenGL::checkShaderErrors(GLuint shader, std::string type) {
	GLint result = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		GLint loglength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &loglength);
		if (loglength > 0) {
			char* logtext = new char[loglength];
			glGetShaderInfoLog(shader, loglength, NULL, logtext);
			MessageBoxA(NULL, logtext, (type + " | Compilation Error").c_str(), MB_OK | MB_ICONERROR);
			delete[] logtext;
		}
		else {
			MessageBoxA(NULL, "Unknown error", (type + " | Error").c_str(), MB_OK | MB_ICONERROR);
		}
		return false;
	}
	return true;
}
