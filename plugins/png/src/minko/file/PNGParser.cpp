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

#include "PNGParser.hpp"

#include "minko/file/Options.hpp"
#include "minko/file/AssetLibrary.hpp"
#include "minko/render/Texture.hpp"

#include "lodepng.h"

using namespace minko::file;

void
PNGParser::parse(const std::string&                 filename,
                 const std::string&                 resolvedFilename,
                 std::shared_ptr<Options>           options,
                 const std::vector<unsigned char>&  data,
                 std::shared_ptr<AssetLibrary>      AssetLibrary)
{
	std::vector<unsigned char> out;
	unsigned int width;
	unsigned int height;

	lodepng::decode(out, width, height, &data[0], data.size());

	auto texture = render::Texture::create(options->context(), width, height, options->generateMipmaps(), false, options->resizeSmoothly());

	texture->data(&out[0]);
	texture->upload();

	AssetLibrary->texture(filename, texture);

	complete()->execute(shared_from_this());
}
