/*************************************************************************
* glOwl - OpenGLObjectWrapperLibrary
*------------------------------------------------------------------------
* Copyright (c) 2014-2017 Michael Becher
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would
*    be appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
*************************************************************************/

#include "Texture2D.hpp"

#include <algorithm>
#include <iostream>

Texture2D::Texture2D(TextureLayout const& layout, GLvoid * data, bool generateMipmap)
	:Texture(layout.internal_format,layout.format,layout.type, layout.levels), m_width(layout.width), m_height(layout.height)
{
	glGenTextures(1, &m_name);

	glBindTexture(GL_TEXTURE_2D, m_name);

	for (auto& pname_pvalue : layout.int_parameters)
		glTexParameteri(GL_TEXTURE_2D, pname_pvalue.first, pname_pvalue.second);
	
	//for (auto& pname_pvalue : layout.float_parameters)
	//	glTexParameterf(GL_TEXTURE_2D, pname_pvalue.first, pname_pvalue.second);

	GLsizei levels = 1;

	if (generateMipmap)
		levels = 1 + floor(log2(std::max(m_width, m_height)));
	
	glTexStorage2D(GL_TEXTURE_2D, levels, m_internal_format, m_width, m_height);

	if (data != nullptr)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, m_type, data);
	
	if (generateMipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	//	m_texture_handle = glGetTextureHandleARB(m_name);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		// "Do something cop!"
		std::cerr << "GL error during texture creation: " << err << std::endl;
	}
}

void Texture2D::bindTexture() const
{
	glBindTexture(GL_TEXTURE_2D, m_name);
}

void Texture2D::updateMipmaps()
{
	glBindTexture(GL_TEXTURE_2D, m_name);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::reload(TextureLayout const& layout, GLvoid * data, bool generateMipmap)
{
	m_width = layout.width;
	m_height = layout.height;
	m_internal_format = layout.internal_format;
	m_format = layout.format;
	m_type = layout.type;

	glDeleteTextures(1, &m_name);

	glGenTextures(1, &m_name);

	glBindTexture(GL_TEXTURE_2D, m_name);

	for (auto& pname_pvalue : layout.int_parameters)
		glTexParameteri(GL_TEXTURE_2D, pname_pvalue.first, pname_pvalue.second);

	for (auto& pname_pvalue : layout.float_parameters)
		glTexParameterf(GL_TEXTURE_2D, pname_pvalue.first, pname_pvalue.second);

	GLsizei levels = 1;

	if (generateMipmap)
		levels = 1 + floor(log2(std::max(m_width, m_height)));

	glTexStorage2D(GL_TEXTURE_2D, levels, m_internal_format, m_width, m_height);

	if (data != nullptr)
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_width, m_height, m_format, m_type, data);

	if (generateMipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		// "Do something cop!"
		std::cerr << "GL error during  texture reload: " << err << std::endl;
	}
}

TextureLayout Texture2D::getTextureLayout() const
{
	return TextureLayout(m_internal_format,m_width,m_height,1,m_format,m_type, m_levels);
}

unsigned int Texture2D::getWidth() const
{
	return m_width;
}

unsigned int Texture2D::getHeight() const
{
	return m_height;
}