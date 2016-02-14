#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include <cstdint>
#include <istream>
#include <memory>
#include <stdexcept>
#include <string>

#include <gtl/ogl/program.h>
#include <gtl/ogl/shader.h>
#include <gtl/ogl/texture.h>

#include "resourcecache.h"


class ResourceNotFoundException : public std::runtime_error
{
public:
	ResourceNotFoundException(const std::string &file, const std::string &msg = std::string());
};

class ResourceLoader
{
public:
	ResourceLoader(const std::string &searchpath);
	virtual ~ResourceLoader();

	bool exists(const std::string &name) const;
	std::unique_ptr<std::istream> open(const std::string &name) const;
	std::string load(const std::string &name) const;

	gtl::ogl::Texture loadTexture(const std::string &name) const;
	std::shared_ptr<const gtl::ogl::Texture> getTexture(const std::string &name);

	gtl::ogl::Texture loadArrayTexture(const std::string names[], std::size_t len) const;
	std::shared_ptr<const gtl::ogl::Texture> getArrayTexture(const std::string names[], std::size_t len);

	gtl::ogl::Shader loadShader(const std::string &name) const;
	std::shared_ptr<const gtl::ogl::Shader> getShader(const std::string &name);

	gtl::ogl::Program loadShaderProgram(const std::string &name) const;
	std::shared_ptr<const gtl::ogl::Program> getShaderProgram(const std::string &name);

private:
	std::string mSearchpath;
	ResourceCache<gtl::ogl::Texture,ResourceLoader,&ResourceLoader::loadTexture> textureCache;
	ResourceCache<gtl::ogl::Program,ResourceLoader,&ResourceLoader::loadShaderProgram> programCache;

};

#endif // RESOURCELOADER_H
