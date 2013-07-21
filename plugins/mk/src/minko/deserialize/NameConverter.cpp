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

#include "NameConverter.hpp"

using namespace minko;
using namespace minko::deserialize;



NameConverter::NameConverter()
{
	initialize();
}

			
void
NameConverter::initialize()
{
	_asNameToCppName["diffuseColor"]	= "material.diffuseColor";
	_asNameToCppName["diffuseMap"]		= "material.diffuseMap";
	_asNameToCppName["blending"]		= "material.blending";
	_asNameToCppName["normalMap"]		= "material.normalMap";
	_asNameToCppName["specular"]		= "material.specular";
	_asNameToCppName["alphaMap"]		= "material.alphaMap";
    _asNameToCppName["alphaThreshold"]	= "material.alphaThreshold";
}

std::string
NameConverter::convertString(std::string original)
{
	if (_asNameToCppName.find(original) != _asNameToCppName.end())
		return _asNameToCppName[original];
	return "";
}
