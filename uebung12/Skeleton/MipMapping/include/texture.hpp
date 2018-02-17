#pragma once

#include <glm/glm.hpp>
#include <vector>

class Texture
{
	int m_width;
	int m_height;
	std::vector<glm::vec4> m_data;

  public:
	Texture(const std::string & file_path);
	int width() const;
	int height() const;
	const glm::vec4 *data() const;
};
