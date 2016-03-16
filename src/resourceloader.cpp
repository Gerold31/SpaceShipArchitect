#include "resourceloader.h"

#include <cassert>
#include <fstream>
#include <istream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>

// TODO remove
#include <iostream>

#include <gtl/ogl/program.h>
#include <gtl/ogl/shader.h>
#include <gtl/ogl/shaderexception.h>
#include <gtl/ogl/texture.h>

#include <SOIL.h>

#include "utils.h"

using gtl::ogl::Program;
using gtl::ogl::Shader;
using gtl::ogl::ShaderException;
using gtl::ogl::Texture;
using std::getline;
using std::ifstream;
using std::istream;
using std::regex;
using std::regex_match;
using std::shared_ptr;
using std::size_t;
using std::string;
using std::stringstream;
using std::unique_ptr;


ResourceLoader::ResourceLoader(const string &searchpath) :
	mSearchpath(searchpath),
	textureCache(this),
	programCache(this)
{
}

ResourceLoader::~ResourceLoader()
{
}

/**
 * @brief Checks whether a resource exists.
 *
 * @param name The name of the resource.
 * @return <code>true</code> if the resource exists, <code>false</code> otherwise.
 */
bool ResourceLoader::exists(const string &name) const
{
	ifstream f(mSearchpath + "/" + name);
	return f.good();
}

/**
 * @brief Gets an imput stream of a resource.
 *
 * This function opens a resource and renturns it's input stream. The badbid of
 * the exception mask is set. Use ResourceLoader::load to get the content of the
 * resource.
 *
 * @see ResourceLoader::load
 * @param name The name of the resource to open.
 * @return The input stream for the given resource name.
 * @throws ResourceNotFoundException If the resource does not exist or could not be opened.
 */
unique_ptr<istream> ResourceLoader::open(const string &name) const
{
	unique_ptr<ifstream> f(new ifstream(mSearchpath + "/" + name));
	errno = 0;
	if (f->good()) {
		f->exceptions(ifstream::badbit);
		return f;
	} else {
		std::string msg;
		if (errno)
			msg = strerror(errno);
		throw ResourceNotFoundException(name, msg);
	}
}

/**
 * @brief Reads a resource and returnes the content.
 *
 * @see ResourceLoader::open
 * @param name The name of the resource.
 * @return The content of the resource.
 * @throws ResourceNotFoundException If the resource does not exist or could not be opened.
 * @throws std::ios_base::failure If an error occurred while reading the resource.
 */
string ResourceLoader::load(const string &name) const
{
	unique_ptr<istream> f = open(name);
	stringstream buffer;
	buffer << f->rdbuf();
	return buffer.str();
}

Texture ResourceLoader::loadTexture(const string &name) const
{
	string data = load(name);

	int width, height, channels;
	auto img = SOIL_load_image_from_memory(
			reinterpret_cast<const unsigned char*>(data.data()), data.size(),
			&width, &height, &channels,
			SOIL_LOAD_AUTO);

	if (img == nullptr) {
		throw InvalidResourceException(name, SOIL_last_result());
	} else {
		Texture t(Texture::Target::T_2D);

		GLenum format;
		GLenum internalFormat;
		switch (channels) {
		case 1:
			format = GL_RED;
			internalFormat = GL_R8;
		{
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
			t.setParameter(GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		}
			break;
		case 2:
			format = GL_RG;
			internalFormat = GL_RG8;
		{
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_GREEN};
			t.setParameter(GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		}
			break;
		case 3:
			format = GL_RGB;
			internalFormat = GL_RGB8;
			break;
		case 4:
			format = GL_RGBA;
			internalFormat = GL_RGBA8;
			break;
		default:
			assert(false);
		}

		t.storage(1, internalFormat, width, height);
		t.setSubImage(0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, img);

		SOIL_free_image_data(img);
		return t;
	}
}

shared_ptr<const Texture> ResourceLoader::getTexture(const string &name)
{
	return textureCache.get(name);
}

Texture ResourceLoader::loadArrayTexture(const string names[], size_t len) const
{
	// TODO
}

shared_ptr<const Texture> ResourceLoader::getArrayTexture(const string names[], size_t len)
{
	// no cache implemented yet
	return std::make_shared<Texture>(loadArrayTexture(names, len));
}

Shader ResourceLoader::loadShader(const string &name) const
{
	Shader::Type type;
	string ext = name.substr(name.find_last_of('.'));

	if (ext == ".vert")
		type = Shader::Type::VERTEX;
	else if (ext == ".frag")
		type = Shader::Type::FRAGMENT;
	else if (ext == ".geom")
		type = Shader::Type::GEOMETRY;
	else if (ext == ".tcs")
		type = Shader::Type::TESS_CONTROL;
	else if (ext == ".tes")
		type = Shader::Type::TESS_EVALUATION;

	std::string source = load(name);
	Shader s(type, source);
	try {
		s.compile();
	} catch (ShaderException &e) {
		// TODO log error
		std::cerr << s.getInfoLog() << std::flush;
		throw;
	}

#ifndef NDEBUG
	// TODO check p.getInfoLog()
#endif

	return s;
}

shared_ptr<const Shader> ResourceLoader::getShader(const string &name)
{
	// no cache implemented yet
	return std::make_shared<Shader>(loadShader(name));
}

Program ResourceLoader::loadShaderProgram(const string &name) const
{
	//using namespace std::regex_constants;
	//static regex rgx_comment("", optimize);

	size_t lastSlash = name.find_last_of('/');
	std::string dir = lastSlash == string::npos ? "" : name.substr(0, lastSlash + 1);
	unique_ptr<istream> in = open(name);

	Program p(true);
	std::string line;
	while (getline(*in, line)) {
		line = trim(line.substr(0, line.find('#')));
		if (!line.empty()) {

			if (line[0] == '/') {
				line.erase(0, 1);
			} else {
				line = dir + line;
			}

			try {
				p.attachShader(loadShader(line));
			} catch (ResourceNotFoundException &e) {
				throw InvalidResourceException(name, string("Missing shader: ") + e.what());
			}
		}
	}

	try {
		p.link();
#ifndef NDEBUG
		p.validate();
#endif
	} catch (ShaderException &e) {
		// TODO log error
		std::cerr << p.getInfoLog() << std::flush;
		throw;
	}

#ifndef NDEBUG
	// TODO check p.getInfoLog()
#endif

	return p;
}

shared_ptr<const Program> ResourceLoader::getShaderProgram(const string &name)
{
	return programCache.get(name);
}

ResourceNotFoundException::ResourceNotFoundException(const string &file, const string &msg) :
	runtime_error(msg.empty() ? file : file + " (" + msg + ")")
{
}

InvalidResourceException::InvalidResourceException(const string &file, const string &msg) :
	runtime_error(msg.empty() ? file : file + " (" + msg + ")")
{
}
