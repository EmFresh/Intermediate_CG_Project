#pragma once

#include <GL/glew.h>
#include <functional>
#include <string>
#include "Shader.h"

class FrameBuffer
{
public:
	FrameBuffer() = delete;
	FrameBuffer(unsigned numColorAttachments, std::string tag = "");
	~FrameBuffer();

	/// <summary>
	/// 
	/// </summary>
	/// <param name="index"></param>
	/// <param name="width"></param>
	/// <param name="height"></param>
	/// <param name="internalFormat"></param>
	/// <param name="filter"></param>
	/// <param name="wrap"></param>
	void initColourTexture(unsigned index, unsigned width, unsigned height, GLint internalFormat = GL_RGBA8, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE);
	void initDepthTexture(unsigned width, unsigned height);
	void resizeColour(unsigned index, unsigned width, unsigned height, GLint internalFormat = GL_RGBA8, GLint filter = GL_LINEAR, GLint wrap = GL_CLAMP_TO_EDGE);
	void resizeDepth(unsigned width, unsigned height);

	bool checkFBO();

	// Clears all OpenGL memory
	void unload();

	
	/// <summary>
	/// Set Clear Colour for the next clear calls
	/// </summary>
	/// <param name="colour"></param>
	static void setClearColour(ColourRGBA colour= {});
	/// <summary>
	/// Set Clear Colour for the next clear calls. Values range 0 -> 1 (inclusive)
	/// </summary>
	/// <param name="r">red channel</param>
	/// <param name="g">green channel</param>
	/// <param name="b">blue channel</param>
	/// <param name="a">alpha channel</param>
	static void setClearColour(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
	// Clears all attached textures
	void clear(GLbitfield =0);

	static void clearBackBuffer(bool clearCol = true, bool clearDep = true);
	//binds objects to frame buffer/s
	void enable();

	//binds the frame buffer to the default location
	static void disable();


	///~ Helper Functions ~///
	

	void setViewport(int x, int y, int width, int height)const;

	void moveColourToBackBuffer(int windowWidth, int windowHeight, uint from = 0);

	void moveColourToBuffer(int windowWidth, int windowHeight, FrameBuffer* fboID, uint from, uint to);

	void moveDepthToBackBuffer(int windowWidth, int windowHeight);

	void moveDepthToBuffer(int windowWidth, int windowHeight, GLuint fboID);

	void takeFromBackBufferColour(int windowWidth, int windowHeight);

	void takeFromBackBufferDepth(int windowWidth, int windowHeight);

	GLuint getDepthHandle() const;
	GLuint getColorHandle(unsigned m_index) const;

	void setPostProcess(std::function<void()>, unsigned layer = 0);
	std::function<void()> getPostProcess();

	unsigned getNumColourAttachments();

	GLuint getFrameBufferID();

	static void drawFullScreenQuad();

	unsigned getDepthWidth();
	unsigned getDepthHeight();


	std::string getTag();
	unsigned getLayer();

private:

	static void initFullScreenQuad();

	static GLuint m_fsQuadVAO_ID, m_fsQuadVBO_ID;

	GLuint
		m_layer = GL_NONE,
		m_fboID = GL_NONE,
		m_depthAttachment = GL_NONE,
		* m_colorAttachments = nullptr;

	GLint m_internalFormat = GL_RGBA8,
		m_filter = GL_LINEAR,
		m_wrap = GL_CLAMP_TO_EDGE;

	unsigned m_width, m_height;

	GLenum* m_buffs = nullptr;

	unsigned int m_numColorAttachments = 0;
	std::string m_tag;
	std::function<void()>m_postProcess;
	Shader* m_shader;
};