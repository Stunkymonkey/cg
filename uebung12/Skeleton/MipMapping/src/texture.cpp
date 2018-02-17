#include <texture.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>

void ltrim(std::string &str)
{
	str.erase(str.begin(),
	          std::find_if(str.begin(), str.end(), [](char chr) { return !std::isspace(chr); }));
}

bool ReadLine(std::istream &stream, std::string &line)
{
	do
	{
		if (!std::getline(stream, line))
			return false;
	} while (line[0] == '#');
	return true;
}

Texture::Texture(const std::string & file_path)
{
	auto ThrowError = [file_path](const char *what) {
		throw std::runtime_error(std::string("Syntax error while parsing ") + file_path + ": " +
		                         what);
	};

	std::ifstream file_stream(file_path);
	if (!file_stream.is_open())
		throw std::runtime_error(std::string("Failed to open ") + file_path);

	std::string line;
	std::string word;
	int max_val;
	int header = 0;
	int index = 0;
	glm::vec4 color{0, 0, 0, 1};

	while (std::getline(file_stream, line))
	{
		ltrim(line);
		if (line.empty() || line[0] == '#')
			continue;
		std::istringstream line_stream{line};
		while (line_stream >> word)
		{
			switch (header)
			{
			case 0:
				if (word != "P3")
					ThrowError("Unsupported format.");
				++header;
				break;
			case 1:
				if (!(std::istringstream(word) >> m_width))
					ThrowError(word.c_str());
				++header;
				break;
			case 2:
				if (!(std::istringstream(word) >> m_height))
					ThrowError(word.c_str());
				++header;
				break;
			case 3:
				if (!(std::istringstream(word) >> max_val))
					ThrowError(word.c_str());
				++header;
				break;
			case 4:
				int val;
				if (!(std::istringstream(word) >> val))
					ThrowError(word.c_str());
				color[index] = static_cast<float>(val) / max_val;
				if (++index == 3)
				{
					index = 0;
					m_data.push_back(color);
				}
			}
		}
	}

	if (header != 4 || index != 0)
		ThrowError("Unexpected end of file.");

	if (m_data.size() != m_width * m_height)
		ThrowError("Invalid number of colors specified.");
}

int Texture::width() const { return m_width; }

int Texture::height() const { return m_height; }

const glm::vec4 *Texture::data() const { return m_data.data(); }
