#pragma once
#include "graphics/graphics.h"

#define GLEW_STATIC
#include "GL/glew.h"
#include "GL/gl.h"
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include <filesystem>
#include <unordered_map>


class Graphics_OpenGL : public Graphics {
public:

	enum class VertAttribs : GLuint {
		Position = 0,
		Normal = 1,
		Tangent = 2,
		Bitangent = 3,
		UV = 4,
	};

	Graphics_OpenGL();
	virtual ~Graphics_OpenGL() override;

	virtual std::string getBackendString() override;
	virtual std::string getGPUNameString() override;

	virtual void setRenderPipeline(RenderPipelineType pipelineType) override;

	virtual void resizeFramebuffer(size_t width, size_t height) override;

	virtual GPUMesh* createMesh() override;
	virtual GPUTexture* createTexture(Texture* thisTexture) override;

	virtual bool createWindow(
		std::string window_title,
		size_t width,
		size_t height,
		bool fullscreen
	) override;

	virtual void destroyWindow() override;

	virtual bool pollEvents() override;

	virtual void swapBuffers() override;

private:

	GLint GLmajorVersion = 0;
	GLint GLminorVersion = 0;
	std::string gpuName;
};



class GPUMesh_OpenGL : public GPUMesh {
public:

	virtual ~GPUMesh_OpenGL() override;

	virtual bool uploadFrom(const Mesh& mesh) override;

	// TODO: ONLY SUPPORTS TRIANGLES.
	virtual void draw() override;

private:

	GLuint VAO = 0;
	GLuint VBO = 0;
	GLuint EBO = 0;
	size_t numIdxs = 0;

	bool setupVAO(size_t numVerts, size_t numIdxs, const Vertex* verts, const VertexIndex* idxs);
	void deleteVAO();

};



class GPUTexture_OpenGL : public GPUTexture {
public:

	GPUTexture_OpenGL(Texture* thisTexture);
	virtual ~GPUTexture_OpenGL() override;

	virtual bool upload(
		uint8_t* data,
		size_t width,
		size_t height,
		size_t numChannels
	) override;

	GLuint getGLTexID();

private:

	GLuint texID = 0;

	static GLenum numChannelsToFormat(size_t numChannels);

};



/*
* An OpenGL shader.
* There is no generic Shader class because each backend handles shaders in a different way.
*/
class Shader_OpenGL {
public:

	Shader_OpenGL();
	Shader_OpenGL(const Shader_OpenGL& shader);
	Shader_OpenGL(Shader_OpenGL&& shader);
	Shader_OpenGL& operator=(const Shader_OpenGL& shader);
	Shader_OpenGL& operator=(Shader_OpenGL&& shader);


	// Sets this shader to be the currently bound shader program.
	void bind();

	// Imports a shader program from a set of code files.
	void read(std::filesystem::path vertPath);
	// Imports a shader program from a set of code files.
	void read(std::filesystem::path vertPath, std::filesystem::path fragPath);
	// Imports a shader program from a set of code files.
	void read(std::filesystem::path vertPath, std::filesystem::path geomPath, std::filesystem::path fragPath);

	// Compiles a shader program from the given code blocks.
	void compile(std::string vertCode);
	// Compiles a shader program from the given code blocks.
	void compile(std::string vertCode, std::string fragCode);
	// Compiles a shader program from the given code blocks.
	void compile(std::string vertCode, std::string geomCode, std::string fragCode);

	// Deletes the current shader program, if any.
	void clear();

	GLuint getID();


	// Returns the OpenGL-defined location of the specified uniform.
	GLuint getUniformLocation(std::string name);

	// The Shader must be bound with bind() before any uniforms can be set.

	void setUniform1f(std::string name, GLfloat value);
	void setUniform2f(std::string name, glm::vec2 value);
	void setUniform3f(std::string name, glm::vec3 value);
	void setUniform4f(std::string name, glm::vec4 value);
	void setUniform1i(std::string name, GLint value);
	void setUniform2i(std::string name, glm::ivec2 value);
	void setUniform3i(std::string name, glm::ivec3 value);
	void setUniform4i(std::string name, glm::ivec4 value);
	void setUniformMat4(std::string name, glm::mat4 value);
	void setUniformTex(std::string name, GLuint texID, GLuint index = 0, GLenum type = GL_TEXTURE_2D);

private:

	GLuint programID = 0;

	// Stores <name, location>. Cleared automatically upon shader deletion.
	std::unordered_map<std::string, GLuint> uniforms;

	// Returns false if an error is encountered, or true if successful.
	bool checkShaderErrors(GLuint shader, std::string type);

};