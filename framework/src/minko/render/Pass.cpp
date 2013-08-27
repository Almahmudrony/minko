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

#include "Pass.hpp"

#include "minko/data/Container.hpp"
#include "minko/render/Program.hpp"
#include "minko/render/Shader.hpp"
#include "minko/render/DrawCall.hpp"
#include "minko/render/States.hpp"

using namespace minko;
using namespace minko::render;

Pass::Pass(const std::string&				name,
		   std::shared_ptr<render::Program>	program,
		   data::BindingMap&				attributeBindings,
		   data::BindingMap&				uniformBindings,
		   data::BindingMap&				stateBindings,
		   data::BindingMap&				macroBindings,
           std::shared_ptr<States>          states) :
	_name(name),
	_programTemplate(program),
	_attributeBindings(attributeBindings),
	_uniformBindings(uniformBindings),
	_stateBindings(stateBindings),
	_macroBindings(macroBindings),
    _states(states)
{
}

std::shared_ptr<DrawCall>
Pass::createDrawCall(std::shared_ptr<data::Container> data, std::shared_ptr<data::Container> rootData)
{
	return DrawCall::create(
        selectProgram(data, rootData),
        data,
        rootData,
        _attributeBindings,
        _uniformBindings,
        _stateBindings,
        _states
    );
}

std::shared_ptr<Program>
Pass::selectProgram(std::shared_ptr<data::Container> data, std::shared_ptr<data::Container> rootData)
{
	Program::Ptr program;

	if (_macroBindings.size() == 0)
		program = _programTemplate;
	else
	{
		auto signature	= buildSignature(data, rootData);

		program = _signatureToProgram[signature];

		// this instance does not exist... yet!
		if (!program)
		{
			std::string defines = "";
			uint i = 0;

			// create shader header with #defines
			for (auto& macroBinding : _macroBindings)
            {
				if (signature & (1 << i++))
				{
					auto& propertyName = macroBinding.second;
					auto& container = data->hasProperty(propertyName) ? data : rootData;

					defines += "#define " + macroBinding.first;
					if (container->propertyHasType<int>(propertyName))
						defines += " " + std::to_string(container->get<int>(propertyName));
					defines += "\n";
				}
            }

			// for program template by adding #defines
			auto vs = Shader::create(
				_programTemplate->context(),
				Shader::Type::VERTEX_SHADER,
				defines + _programTemplate->vertexShader()->source()
			);
			auto fs = Shader::create(
				_programTemplate->context(),
				Shader::Type::FRAGMENT_SHADER,
				defines + _programTemplate->fragmentShader()->source()
			);

			program = Program::create(_programTemplate->context(), vs, fs);
            program->vertexShader()->upload();
		    program->fragmentShader()->upload();
		    program->upload();

			// register the program to this signature
			_signatureToProgram[signature] = program;
		}
	}

	return program;
}

const unsigned int
Pass::buildSignature(std::shared_ptr<data::Container> data, std::shared_ptr<data::Container> rootData)
{
	unsigned int signature = 0;
	unsigned int i = 0;

	for (auto& macroBinding : _macroBindings)
    {
        auto& propertyName = macroBinding.second;

        if (data->hasProperty(propertyName) || rootData->hasProperty(propertyName))
		{
			// WARNING: we do not support more than 32 macro bindings
			if (i == 32)
				throw;

			signature |= 1 << i;
		}
        ++i;
    }

	return signature;
}
