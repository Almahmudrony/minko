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

#include "QuadGeometry.hpp"

#include "minko/render/VertexBuffer.hpp"
#include "minko/render/IndexBuffer.hpp"

using namespace minko;
using namespace minko::geometry;

void
QuadGeometry::initialize(std::shared_ptr<render::AbstractContext> context)
{
    float xyzData[]         = {
    	-.5f, .5f, 0.f,		0.f, 0.f,
    	-.5f, -.5f, 0.f,	0.f, 1.f,
    	.5f, .5f, 0.f,		1.f, 0.f,
    	.5f, -.5f, 0.f,		1.f, 1.f
    };
    unsigned short ind[]    = {0, 1, 2, 2, 1, 3};
    auto vertexBuffer       = render::VertexBuffer::create(context, std::begin(xyzData), std::end(xyzData));
    auto indexBuffer        = render::IndexBuffer::create(context, std::begin(ind), std::end(ind));

    vertexBuffer->addAttribute("position", 3);
    vertexBuffer->addAttribute("uv", 2);
    addVertexBuffer(vertexBuffer);

    indices(indexBuffer);
}
