/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "AssetLibrary.hpp"

#include "minko/scene/Node.hpp"
#include "minko/file/Loader.hpp"
#include "minko/file/Options.hpp"
#include "minko/file/AbstractParser.hpp"
#include "minko/file/EffectParser.hpp"
#include "minko/render/Texture.hpp"

using namespace minko;
using namespace minko::render;
using namespace minko::geometry;
using namespace minko::file;

AssetLibrary::Ptr
AssetLibrary::create(AbsContextPtr context)
{
	auto al = std::shared_ptr<AssetLibrary>(new AssetLibrary(context));

	al->registerParser<file::EffectParser>("effect");

	return al;
}

AssetLibrary::AssetLibrary(std::shared_ptr<AbstractContext> context) :
	_context(context),
	_complete(Signal<Ptr>::create()),
	_defaultOptions(file::Options::create(context))
{
}

AssetLibrary::GeometryPtr
AssetLibrary::geometry(const std::string& name)
{
	return _geometries[name];
}

AssetLibrary::Ptr
AssetLibrary::geometry(const std::string& name, std::shared_ptr<Geometry> geometry)
{
	_geometries[name] = geometry;

	return shared_from_this();
}

render::Texture::Ptr
AssetLibrary::texture(const std::string& name)
{
	return _textures[name];
}

AssetLibrary::Ptr
AssetLibrary::texture(const std::string& name, render::Texture::Ptr texture)
{
	_textures[name] = texture;

	return shared_from_this();
}

scene::Node::Ptr
AssetLibrary::node(const std::string& name)
{
	return _nodes[name];
}

AssetLibrary::Ptr
AssetLibrary::node(const std::string& name, scene::Node::Ptr node)
{
	_nodes[name] = node;

	return shared_from_this();
}

AssetLibrary::EffectPtr
AssetLibrary::effect(const std::string& name)
{
	return _effects[name];
}

AssetLibrary::Ptr
AssetLibrary::effect(const std::string& name, std::shared_ptr<Effect> effect)
{
	_effects[name] = effect;

	return shared_from_this();
}

const std::vector<unsigned char>&
AssetLibrary::blob(const std::string& name)
{
	return _blobs[name];
}

AssetLibrary::Ptr
AssetLibrary::blob(const std::string& name, const std::vector<unsigned char>& blob)
{
	_blobs[name] = blob;

	return shared_from_this();
}

const unsigned int
AssetLibrary::layer(const std::string& name)
{
	if (_layers.count(name) == 0)
	{
		unsigned int existingMask = 0;

		for (auto layer : _layers)
			existingMask |= layer.second;

		auto mask = 1;
		for (auto i = 0; i < 32 && (existingMask & mask); ++i, mask <<= 1)
			continue;

		if (mask == 0)
			throw;

		_layers[name] = mask;
	}

	return _layers[name];
}

AssetLibrary::Ptr
AssetLibrary::layer(const std::string& name, const unsigned int mask)
{
	_layers[name] = mask;

	return shared_from_this();
}

AssetLibrary::Ptr
AssetLibrary::queue(const std::string& filename, std::shared_ptr<file::Options> options)
{
	_filesQueue.push_back(filename);
	_filenameToOptions[filename] = options ? options : _defaultOptions;

	return shared_from_this();
}

AssetLibrary::Ptr
AssetLibrary::load(const std::string& filename, std::shared_ptr<file::Options> options)
{
	queue(filename, options);
	load();

	return shared_from_this();
}

AssetLibrary::Ptr
AssetLibrary::load()
{
	std::list<std::string> queue = _filesQueue;

	for (auto& filename : queue)
	{
		if (_filenameToLoader.count(filename) == 0)
		{
			auto loader = file::Loader::create();

			_filenameToLoader[filename] = loader;
			_loaderSlots.push_back(loader->error()->connect(std::bind(
				&AssetLibrary::loaderErrorHandler, shared_from_this(), std::placeholders::_1
			)));
			_loaderSlots.push_back(loader->complete()->connect(std::bind(
				&AssetLibrary::loaderCompleteHandler, shared_from_this(), std::placeholders::_1
			)));
			loader->load(filename, _defaultOptions);
		}
	}

	return shared_from_this();
}

void
AssetLibrary::loaderErrorHandler(std::shared_ptr<file::Loader> loader)
{
	throw std::invalid_argument(loader->filename());
}

void
AssetLibrary::loaderCompleteHandler(std::shared_ptr<file::Loader> loader)
{
	auto filename = loader->filename();
	auto extension = filename.substr(filename.find_last_of('.') + 1);

	_filesQueue.erase(std::find(_filesQueue.begin(), _filesQueue.end(), filename));
	_filenameToLoader.erase(filename);

	if (_parsers.count(extension))
	{
		auto parser = _parsers[extension]();
		
		parser->parse(
            loader->filename(),
            loader->resolvedFilename(),
            _filenameToOptions[filename],
            loader->data(),
            shared_from_this()
        );
	}
	else
		blob(filename, loader->data());

	if (_filesQueue.size() == 0)
	{
		_loaderSlots.clear();
		_filenameToLoader.clear();
		_filenameToOptions.clear();

		_complete->execute(shared_from_this());
	}
}

AssetLibrary::AbsParserPtr
AssetLibrary::parser(std::string extension)
{
	return _parsers[extension]();
}
