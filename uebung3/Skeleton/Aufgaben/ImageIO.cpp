#include "ImageIO.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <string>

cg::image<cg::color_space_t::RGB> cg::image_io::load_rgb_image(const std::string& path) const
{
	std::ifstream image_file(path, std::iostream::in | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		auto header = load_header(image_file);

		if (header.file_type != cg::image_io::header::PPM)
		{
			throw std::runtime_error("RGB images can only be loaded from PPM files");
		}

		// Read image
		std::vector<char> buffer(3 * header.width * header.height * ((header.max_value >= 256) ? 2 : 1));
		const auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());
		const auto* wbuffer = reinterpret_cast<char16_t*>(buffer.data());

		image_file.read(buffer.data(), buffer.size());

		// Create image
		cg::image<cg::color_space_t::RGB> image(header.width, header.height);

		std::size_t index = 0;

		for (unsigned int j = 0; j < header.height; ++j)
		{
			for (unsigned int i = 0; i < header.width; ++i)
			{
				if (header.max_value < 256)
				{
					image(i, j)[0] = static_cast<float>(cbuffer[index++]) / 255.0f;
					image(i, j)[1] = static_cast<float>(cbuffer[index++]) / 255.0f;
					image(i, j)[2] = static_cast<float>(cbuffer[index++]) / 255.0f;
				}
				else
				{
					image(i, j)[0] = static_cast<float>(wbuffer[index++]) / 65535.0f;
					image(i, j)[1] = static_cast<float>(wbuffer[index++]) / 65535.0f;
					image(i, j)[2] = static_cast<float>(wbuffer[index++]) / 65535.0f;
				}
			}
		}

		return image;
	}
		
	throw std::runtime_error("Unable to open file");
}

cg::image<cg::color_space_t::Gray> cg::image_io::load_grayscale_image(const std::string& path) const
{
	std::ifstream image_file(path, std::iostream::in | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		auto header = load_header(image_file);

		if (header.file_type != cg::image_io::header::PGM)
		{
			throw std::runtime_error("Grayscale images can only be loaded from PGM files");
		}

		// Read image
		std::vector<char> buffer(header.width * header.height * ((header.max_value >= 256) ? 2 : 1));
		const auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());
		const auto* wbuffer = reinterpret_cast<char16_t*>(buffer.data());

		image_file.read(buffer.data(), buffer.size());

		// Create image
		cg::image<cg::color_space_t::Gray> image(header.width, header.height);

		std::size_t index = 0;

		for (unsigned int j = 0; j < header.height; ++j)
		{
			for (unsigned int i = 0; i < header.width; ++i)
			{
				if (header.max_value < 256)
				{
					image(i, j)[0] = static_cast<float>(cbuffer[index++]) / 255.0f;
				}
				else
				{
					image(i, j)[0] = static_cast<float>(wbuffer[index++]) / 65535.0f;
				}
			}
		}

		return image;
	}

	throw std::runtime_error("Unable to open file");
}

cg::image<cg::color_space_t::BW> cg::image_io::load_bw_image(const std::string& path) const
{
	std::ifstream image_file(path, std::iostream::in | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		auto header = load_header(image_file);

		if (header.file_type != cg::image_io::header::PBM)
		{
			throw std::runtime_error("Black-and-white images can only be loaded from PBM files");
		}

		// Read image
		std::vector<char> buffer((header.width + header.width % 8) * header.height / 8);
		const auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());

		image_file.read(buffer.data(), buffer.size());

		// Create image
		cg::image<cg::color_space_t::BW> image(header.width, header.height);

		std::size_t index = buffer.size() - 1;

		for (int j = header.height - 1; j >= 0; --j)
		{
			for (int i = (header.width + header.width % 8) - 1; i >= 0; i -= 8)
			{
				image((i - 0) % header.width, j)[0] = ((cbuffer[index] & 1) != 0) ? 1.0f : 0.0f;
				image((i - 1) % header.width, j)[0] = ((cbuffer[index] & 2) != 0) ? 1.0f : 0.0f;
				image((i - 2) % header.width, j)[0] = ((cbuffer[index] & 4) != 0) ? 1.0f : 0.0f;
				image((i - 3) % header.width, j)[0] = ((cbuffer[index] & 8) != 0) ? 1.0f : 0.0f;
				image((i - 4) % header.width, j)[0] = ((cbuffer[index] & 16) != 0) ? 1.0f : 0.0f;
				image((i - 5) % header.width, j)[0] = ((cbuffer[index] & 32) != 0) ? 1.0f : 0.0f;
				image((i - 6) % header.width, j)[0] = ((cbuffer[index] & 64) != 0) ? 1.0f : 0.0f;
				image((i - 7) % header.width, j)[0] = ((cbuffer[index] & 128) != 0) ? 1.0f : 0.0f;

				--index;
			}
		}

		return image;
	}

	throw std::runtime_error("Unable to open file");
}

void cg::image_io::save_rgb_image(const std::string& path, const cg::image<cg::color_space_t::RGB>& image) const
{
	std::ofstream image_file(path, std::iostream::out | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		cg::image_io::header header;
		header.file_type = cg::image_io::header::PPM;
		header.width = image.get_width();
		header.height = image.get_height();
		header.max_value = 255;

		save_header(image_file, header);

		// Create buffer
		std::vector<char> buffer(3 * header.width * header.height);
		auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());

		std::size_t index = 0;

		for (unsigned int j = 0; j < header.height; ++j)
		{
			for (unsigned int i = 0; i < header.width; ++i)
			{
				cbuffer[index++] = static_cast<unsigned char>(image(i, j)[0] * 255.0f);
				cbuffer[index++] = static_cast<unsigned char>(image(i, j)[1] * 255.0f);
				cbuffer[index++] = static_cast<unsigned char>(image(i, j)[2] * 255.0f);
			}
		}

		// Save to file
		image_file.write(buffer.data(), buffer.size());
	}
	else
	{
		throw std::runtime_error("Unable to open file");
	}
}

void cg::image_io::save_grayscale_image(const std::string& path, const cg::image<cg::color_space_t::Gray>& image) const
{
	std::ofstream image_file(path, std::iostream::out | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		cg::image_io::header header;
		header.file_type = cg::image_io::header::PGM;
		header.width = image.get_width();
		header.height = image.get_height();
		header.max_value = 255;

		save_header(image_file, header);

		// Create buffer
		std::vector<char> buffer(header.width * header.height);
		auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());

		std::size_t index = 0;

		for (unsigned int j = 0; j < header.height; ++j)
		{
			for (unsigned int i = 0; i < header.width; ++i)
			{
				cbuffer[index++] = static_cast<unsigned char>(image(i, j)[0] * 255.0f);
			}
		}

		// Save to file
		image_file.write(buffer.data(), buffer.size());
	}
	else
	{
		throw std::runtime_error("Unable to open file");
	}
}

void cg::image_io::save_bw_image(const std::string& path, const cg::image<cg::color_space_t::BW>& image) const
{
	std::ofstream image_file(path, std::iostream::out | std::iostream::binary);

	if (image_file.is_open() && image_file.good())
	{
		cg::image_io::header header;
		header.file_type = cg::image_io::header::PBM;
		header.width = image.get_width();
		header.height = image.get_height();
		header.max_value = 255;

		save_header(image_file, header);

		// Create buffer
		std::vector<char> buffer((header.width + header.width % 8) * header.height / 8);
		auto* cbuffer = reinterpret_cast<unsigned char*>(buffer.data());

		std::size_t index = 0;

		for (unsigned int j = 0; j < header.height; ++j)
		{
			for (unsigned int i = 0; i < header.width; i += 8)
			{
				cbuffer[index] = static_cast<unsigned char>(0);

				cbuffer[index] |= (image((i + 0) % header.width, j)[0] != 0.0f) ? 128 : 0;
				cbuffer[index] |= (image((i + 1) % header.width, j)[0] != 0.0f) ? 64 : 0;
				cbuffer[index] |= (image((i + 2) % header.width, j)[0] != 0.0f) ? 32 : 0;
				cbuffer[index] |= (image((i + 3) % header.width, j)[0] != 0.0f) ? 16 : 0;
				cbuffer[index] |= (image((i + 4) % header.width, j)[0] != 0.0f) ? 8 : 0;
				cbuffer[index] |= (image((i + 5) % header.width, j)[0] != 0.0f) ? 4 : 0;
				cbuffer[index] |= (image((i + 6) % header.width, j)[0] != 0.0f) ? 2 : 0;
				cbuffer[index] |= (image((i + 7) % header.width, j)[0] != 0.0f) ? 1 : 0;

				++index;
			}
		}

		// Save to file
		image_file.write(buffer.data(), buffer.size());
	}
	else
	{
		throw std::runtime_error("Unable to open file");
	}
}

cg::image_io::header cg::image_io::load_header(std::ifstream& stream) const
{
	header file_header;

	// Read magic number
	std::array<char, 2> magic_number;
	stream.read(magic_number.data(), magic_number.size());

	if (magic_number[0] == 'P')
	{
		if (magic_number[1] == '1')
		{
			file_header.file_type = header::file_t::PLAIN_PBM;
		}
		else if (magic_number[1] == '2')
		{
			file_header.file_type = header::file_t::PLAIN_PGM;
		}
		else if (magic_number[1] == '3')
		{
			file_header.file_type = header::file_t::PLAIN_PPM;
		}
		else if (magic_number[1] == '4')
		{
			file_header.file_type = header::file_t::PBM;
		}
		else if (magic_number[1] == '5')
		{
			file_header.file_type = header::file_t::PGM;
		}
		else if (magic_number[1] == '6')
		{
			file_header.file_type = header::file_t::PPM;
		}
		else
		{
			file_header.file_type = header::file_t::INVALID;
		}
	}
	else
	{
		file_header.file_type = header::file_t::INVALID;
	}

	if (file_header.file_type == header::file_t::INVALID)
	{
		throw std::runtime_error("Invalid file format");
	}

	// Read extents (width, height, max. value)
	std::array<unsigned int, 3> extents;
	char next_char;

	for (std::size_t i = 0; i < extents.size(); ++i)
	{
		std::vector<char> buffer;
		buffer.reserve(256);

		enum state_t
		{
			READY,		// Ready to read in the value
			COMMENT,	// Inside a comment
			READING,	// Currently reading (part of) the value
			FINISHED	// Finished reading the value
		} state;

		state = READY;

		while (state != FINISHED)
		{
			stream.read(&next_char, 1);

			switch (state)
			{
			case READY:
				if (next_char == '#')
				{
					state = COMMENT;
				}
				else if (next_char != '\n' && next_char != '\r' && next_char != '\t' && next_char != ' ')
				{
					state = READING;
					buffer.push_back(next_char);
				}

				break;
			case COMMENT:
				if (next_char == '\n' || next_char == '\r')
				{
					state = READY;
				}

				break;
			case READING:
				if (next_char != '\n' && next_char != '\r' && next_char != '\t' && next_char != ' ')
				{
					buffer.push_back(next_char);
				}
				else
				{
					state = FINISHED;
				}
			}
		}

		std::string buffer_string(buffer.begin(), buffer.end());
		extents[i] = static_cast<unsigned int>(std::stoul(buffer_string));
	}

	file_header.width = extents[0];
	file_header.height = extents[1];
	file_header.max_value = extents[2];

	return file_header;
}

void cg::image_io::save_header(std::ofstream& stream, const cg::image_io::header& file_header) const
{
	// Save magic number
	switch (file_header.file_type)
	{
	case header::file_t::PBM:
		stream << "P4" << std::endl;

		break;
	case header::file_t::PGM:
		stream << "P5" << std::endl;

		break;
	case header::file_t::PPM:
		stream << "P6" << std::endl;

		break;
	default:
		throw std::runtime_error("Invalid header");
	}

	// Save extents
	stream << file_header.width << " " << file_header.height << "\n" << file_header.max_value << std::endl;
}