/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009,2010  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "compat.h"
#include <GL/glew.h>
#include <vector>
#include "swftypes.h"

namespace lightspark
{

void cleanGLErrors();

class TextureBuffer
{
private:
	GLuint texId;
	GLenum filtering;
	uint32_t allocWidth;
	uint32_t allocHeight;
	uint32_t width;
	uint32_t height;
	uint32_t horizontalAlignment;
	uint32_t verticalAlignment;
	bool inited;
	uint32_t nearestPOT(uint32_t a) const;
	void setAllocSize(uint32_t w, uint32_t h);
public:
	/**
	  	TextureBuffer constructor

		@param initNow Create right now the texture (can be true only if created inside the Render Thread)
		@param width The requested width
		@param height The requested height
		@param filtering The requested texture filtering from OpenGL enumeration
	*/
	TextureBuffer(bool initNow, uint32_t width=0, uint32_t height=0, GLenum filtering=GL_NEAREST);
	/**
	  	TextureBuffer destructor

		Destroys the GL resources allocated for this texture
		@pre Should be run inside the RenderThread or shutdown should be already run
	*/
	~TextureBuffer();
	/**
	   	Return the texture id

		@ret The OpenGL texture id
	*/
	GLuint getId() {return texId;}
	/**
	  	Initialize the texture from the values stored

		@pre Running inside the RenderThread
	*/
	void init();
	/**
	  	Initialize the texture using new values

		@param width The requested width
		@param height The requested height
		@param filtering The requested texture filtering from OpenGL enumeration
		@pre Running inside the RenderThread
	*/
	void init(uint32_t width, uint32_t height, GLenum filtering=GL_NEAREST);
	/**
	  	Frees the GL resources

		@pre Running inside the RenderThread
	*/
	void shutdown();
	/**
		Bind as the current texture

		@pre Running inside the RenderThread
	*/
	void bind();
	/**
		Unbind the current texture

		@pre Running inside the RenderThread
	*/
	void unbind();
	/**
		Set the given uniform with the coordinate scale of the current texture

		@pre Running inside the RenderThread
	*/
	void setTexScale(GLuint uniformLocation);
	/**
		Load data inside the texture

		@pre Running inside the RenderThread
	*/
	void setBGRAData(uint8_t* bgraData, uint32_t w, uint32_t h);
	void resize(uint32_t width, uint32_t height);
	/**
		Request a minimum alignment for width and height
	*/
	void setRequestedAlignment(uint32_t w, uint32_t h);
	uint32_t getAllocWidth() const { return allocWidth;}
	uint32_t getAllocHeight() const { return allocHeight;}
};

class MatrixApplier
{
private:
	struct packedMatrix
	{
		float data[4][4];
	};
	std::vector<packedMatrix> savedStack;
public:
	MatrixApplier();
	MatrixApplier(const MATRIX& m);
	void concat(const MATRIX& m);
	void unapply();
};

class TextureChunk
{
friend class RenderThread;
private:
	uint32_t* chunks;
	TextureChunk(uint32_t w, uint32_t h);
public:
	TextureChunk():chunks(NULL),width(0),height(0){}
	TextureChunk(const TextureChunk& r);
	TextureChunk& operator=(const TextureChunk& r);
	~TextureChunk();
	uint32_t getNumberOfChunks() const { return ((width+127)/128)*((height+127)/128); }
	uint32_t width;
	uint32_t height;
};

class ITextureUploadable
{
protected:
	~ITextureUploadable(){}
public:
	virtual void sizeNeeded(uint32_t& w, uint32_t& h)=0;
	/*
		Upload data to memory mapped to the graphics card (note: size is guaranteed to be enough
	*/
	virtual void upload(uint8_t* data, uint32_t w, uint32_t h)=0;
	virtual const TextureChunk& getTexture() const=0;
	/*
		Signal the completion of the upload to the texture
		NOTE: fence may be called on shutdown even if the upload has not happen, so be ready for this event
	*/
	virtual void uploadFence()=0;
};

class CairoRenderer: public ITextureUploadable
{
protected:
	~CairoRenderer(){}
private:
	const TextureChunk& tex;
public:
	CairoRenderer(const TextureChunk& _t):tex(_t){}
	void sizeNeeded(uint32_t& w, uint32_t& h);
	void upload(uint8_t* data, uint32_t w, uint32_t h);
	const TextureChunk& getTexture() const;
	void uploadFence();
};

};
#endif
