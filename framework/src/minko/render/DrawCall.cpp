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

#include "DrawCall.hpp"

#include "minko/render/AbstractContext.hpp"
#include "minko/render/CompareMode.hpp"
#include "minko/render/WrapMode.hpp"
#include "minko/render/TextureFilter.hpp"
#include "minko/render/MipFilter.hpp"
#include "minko/render/Blending.hpp"
#include "minko/render/TriangleCulling.hpp"
#include "minko/render/VertexBuffer.hpp"
#include "minko/render/IndexBuffer.hpp"
#include "minko/render/Texture.hpp"
#include "minko/render/Program.hpp"
#include "minko/render/States.hpp"
#include "minko/data/Container.hpp"
#include "minko/math/Matrix4x4.hpp"

using namespace minko;
using namespace minko::data;
using namespace minko::math;
using namespace minko::render;

SamplerState DrawCall::_defaultSamplerState = SamplerState(WrapMode::CLAMP, TextureFilter::NEAREST, MipFilter::NONE);
/*static*/ const unsigned int	DrawCall::MAX_NUM_TEXTURES		= 8;
/*static*/ const unsigned int	DrawCall::MAX_NUM_VERTEXBUFFERS	= 8;

DrawCall::DrawCall(const data::BindingMap&	attributeBindings,
				   const data::BindingMap&	uniformBindings,
				   const data::BindingMap&	stateBindings,
                   States::Ptr              states) :
	_program(nullptr),
	_attributeBindings(attributeBindings),
	_uniformBindings(uniformBindings),
	_stateBindings(stateBindings),
    _states(states),
    _textures(MAX_NUM_TEXTURES, -1),
    _textureLocations(MAX_NUM_TEXTURES, -1),
    _textureWrapMode(MAX_NUM_TEXTURES, WrapMode::CLAMP),
    _textureFilters(MAX_NUM_TEXTURES, TextureFilter::NEAREST),
    _textureMipFilters(MAX_NUM_TEXTURES, MipFilter::NONE),
    _vertexBuffers(MAX_NUM_VERTEXBUFFERS, -1),
    _vertexBufferLocations(MAX_NUM_VERTEXBUFFERS, -1),
    _vertexSizes(MAX_NUM_VERTEXBUFFERS, -1),
    _vertexAttributeSizes(MAX_NUM_VERTEXBUFFERS, -1),
    _vertexAttributeOffsets(MAX_NUM_VERTEXBUFFERS, -1),
	_target(nullptr),
	_referenceChangedSlots()
{
}

void
DrawCall::configure(std::shared_ptr<Program>  program,
                    ContainerPtr              data,
                    ContainerPtr              rootData)
{
    _program = program;
    bind(data, rootData);
}

void
DrawCall::bind(ContainerPtr data, ContainerPtr rootData)
{
	reset();

	_data		= data;
	_rootData	= rootData;

	auto indexBuffer	= getDataProperty<IndexBuffer::Ptr>("geometry.indices");

    _indexBuffer	= indexBuffer->id();
    _numIndices		= indexBuffer->data().size();

	bindProgramInputs();
	bindStates();
}

void
DrawCall::bindProgramInputs()
{
	if (_program == nullptr || _program->inputs() == nullptr)
		return;

	auto							programInputs		= _program->inputs();
	const std::vector<std::string>&	inputNames			= programInputs->names();
	unsigned int					numTextures			= 0;
    unsigned int					numVertexBuffers	= 0;
	
	unsigned int					vertexBufferId		= 0;
	unsigned int					textureId			= 0;
	for (unsigned int inputId = 0; inputId < inputNames.size(); ++inputId)
	{
		const std::string&	inputName	= inputNames[inputId];
		const auto			type		= programInputs->type(inputName);
		const int			location	= programInputs->location(inputName);

		switch (type)
		{
		case ProgramInputs::Type::attribute:
			{
				auto				propertyNameIt	= _attributeBindings.find(inputName);
				const std::string&	propertyName	= propertyNameIt == _attributeBindings.end() ? inputName : propertyNameIt->second;
				Container::Ptr		container		= getDataContainer(propertyName);
				if (container)
					bindVertexAttribute(container, propertyName, location, vertexBufferId);
				++vertexBufferId;
				break;
			}
	
		case ProgramInputs::Type::sampler2d:
			{
				auto				propertyNameIt	= _uniformBindings.find(inputName);
				const std::string&	propertyName	= propertyNameIt == _uniformBindings.end() ? inputName : propertyNameIt->second;
				Container::Ptr		container		= getDataContainer(propertyName);
				if (container)
					bindTextureSampler2D(container, inputName, propertyName, location, textureId);
				++textureId;
				break;
			}
	
		default:
			{
				auto				propertyNameIt	= _uniformBindings.find(inputName);
				const std::string&	propertyName	= propertyNameIt == _uniformBindings.end() ? inputName : propertyNameIt->second;
				Container::Ptr		container		= getDataContainer(propertyName);
				if (container)
					bindUniform(container, propertyName, type, location);
				break;
			}
	
		case ProgramInputs::Type::unknown:
			break;
		}
	}
}

void
DrawCall::bindVertexAttribute(Container::Ptr		container,
							  const std::string&	propertyName,
							  int					location,
							  int					vertexBufferId)
{
#ifdef DEBUG
	if (location < 0)
		throw std::invalid_argument("location");
	if (vertexBufferId < 0 || vertexBufferId >= MAX_NUM_VERTEXBUFFERS)
		throw std::invalid_argument("vertexBufferId");
	if (container==nullptr || !container->hasProperty(propertyName))
		throw;
#endif // DEBUG

	auto vertexBuffer	= container->get<VertexBuffer::Ptr>(propertyName);
	auto attributeName  = propertyName.substr(propertyName.find_last_of('.') + 1);
	auto attribute		= vertexBuffer->attribute(attributeName);
	
	_vertexBuffers[vertexBufferId]			= vertexBuffer->id();
	_vertexBufferLocations[vertexBufferId]	= location;
	_vertexAttributeSizes[vertexBufferId]	= std::get<1>(*attribute);
	_vertexSizes[vertexBufferId]			= vertexBuffer->vertexSize();
	_vertexAttributeOffsets[vertexBufferId]	= std::get<2>(*attribute);

	if (_referenceChangedSlots.count(propertyName) == 0)
		_referenceChangedSlots[propertyName] = container->propertyReferenceChanged(propertyName)->connect(std::bind(
			&DrawCall::bindVertexAttribute,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			location,
			vertexBufferId
		));
}

void
DrawCall::bindTextureSampler2D(Container::Ptr		container,
							   const std::string&	inputName,
							   const std::string&	propertyName,
							   int					location,
							   int					textureId)
{
#ifdef DEBUG
	if (location < 0)
		throw std::invalid_argument("location");
	if (textureId < 0 || textureId >= MAX_NUM_TEXTURES)
		throw std::invalid_argument("textureId");
	if (container==nullptr || !container->hasProperty(propertyName))
		throw;
#endif // DEBUG
	
	auto texture        = container->get<Texture::Ptr>(propertyName)->id();
	auto& samplerState  = _states->samplers().count(inputName)
	    ? _states->samplers().at(inputName)
	    : _defaultSamplerState;
	
	_textures[textureId]			= texture;
	_textureLocations[textureId]	= location;
	_textureWrapMode[textureId]		= std::get<0>(samplerState);
	_textureFilters[textureId]		= std::get<1>(samplerState);
	_textureMipFilters[textureId]	= std::get<2>(samplerState);

	if (_referenceChangedSlots.count(propertyName) == 0)
		_referenceChangedSlots[propertyName] = container->propertyReferenceChanged(propertyName)->connect(std::bind(
			&DrawCall::bindTextureSampler2D,
			shared_from_this(),
			std::placeholders::_1,
			inputName,
			std::placeholders::_2,
			location,
			textureId
		));
}

void
DrawCall::bindUniform(Container::Ptr		container,
					  const std::string&	propertyName,
					  ProgramInputs::Type	type,
					  int					location)
{
#ifdef DEBUG
	if (type == ProgramInputs::Type::sampler2d || type == ProgramInputs::Type::attribute)
		throw std::invalid_argument("type");
	if (location < 0)
		throw std::invalid_argument("location");
	if (container==nullptr || !container->hasProperty(propertyName))
		throw;
#endif // DEBUG

	if (type == ProgramInputs::Type::float1)
	     _uniformFloat[location] = container->get<float>(propertyName);
	else if (type == ProgramInputs::Type::float2)
	     _uniformFloat2[location] = container->get<std::shared_ptr<Vector2>>(propertyName);
	else if (type == ProgramInputs::Type::float3)
	     _uniformFloat3[location] = container->get<std::shared_ptr<Vector3>>(propertyName);
	else if (type == ProgramInputs::Type::float4)
	     _uniformFloat4[location] = container->get<std::shared_ptr<Vector4>>(propertyName);
	else if (type == ProgramInputs::Type::float16)
	     _uniformFloat16[location] = &(container->get<Matrix4x4::Ptr>(propertyName)->data()[0]);
	else
		throw std::logic_error("unsupported uniform type.");

	if (_referenceChangedSlots.count(propertyName) == 0)
		_referenceChangedSlots[propertyName] = container->propertyReferenceChanged(propertyName)->connect(std::bind(
			&DrawCall::bindUniform,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			type,
			location
		));
}

void
DrawCall::reset()
{
	_target = nullptr;

	_uniformFloat.clear();
	_uniformFloat2.clear();
	_uniformFloat3.clear();
	_uniformFloat4.clear();
	_uniformFloat16.clear();

	_textures			.clear();
	_textureLocations	.clear();
	_textureWrapMode	.clear();
	_textureFilters		.clear();
	_textureMipFilters	.clear();

	_textures			.resize(MAX_NUM_TEXTURES, -1);
	_textureLocations	.resize(MAX_NUM_TEXTURES, -1);
	_textureWrapMode	.resize(MAX_NUM_TEXTURES, WrapMode::CLAMP);
	_textureFilters		.resize(MAX_NUM_TEXTURES, TextureFilter::NEAREST);
	_textureMipFilters	.resize(MAX_NUM_TEXTURES, MipFilter::NONE);

	_vertexBuffers			.clear();
	_vertexBufferLocations	.clear();
	_vertexSizes			.clear();
	_vertexAttributeSizes	.clear();
	_vertexAttributeOffsets	.clear();

	_vertexBuffers			.resize(MAX_NUM_VERTEXBUFFERS, -1);
	_vertexBufferLocations	.resize(MAX_NUM_VERTEXBUFFERS, -1);
	_vertexSizes			.resize(MAX_NUM_VERTEXBUFFERS, -1);
	_vertexAttributeSizes	.resize(MAX_NUM_VERTEXBUFFERS, -1);
	_vertexAttributeOffsets	.resize(MAX_NUM_VERTEXBUFFERS, -1);

	_propertyChangedSlots.clear();
	_referenceChangedSlots.clear();
}

void
DrawCall::bindStates()
{
	_blendMode = getDataProperty<Blending::Mode>(
        _stateBindings.count("blendMode") ? _stateBindings.at("blendMode") : "blendMode",
        _states->blendingSourceFactor() | _states->blendingDestinationFactor()
    );

	_depthMask = getDataProperty<bool>(
		_stateBindings.count("depthMask") ? _stateBindings.at("depthMask") : "depthMask",
        _states->depthMask()
	);
	_depthFunc = getDataProperty<CompareMode>(
		_stateBindings.count("depthFunc") ? _stateBindings.at("depthFunc") : "depthFunc",
        _states->depthFun()
	);

    _triangleCulling = getDataProperty<TriangleCulling>(
		_stateBindings.count("triangleCulling") ? _stateBindings.at("triangleCulling") : "triangleCulling",
        _states->triangleCulling()
	);

    _target = getDataProperty<Texture::Ptr>(
    	_stateBindings.count("target") ? _stateBindings.at("target") : "target",
        _states->target()
    );
	
    if (_target && !_target->isReady())
        _target->upload();
}

void
DrawCall::render(const AbstractContext::Ptr& context)
{
    if (_target)
        context->setRenderToTexture(_target->id(), true);
    else
        context->setRenderToBackBuffer();

    context->setProgram(_program->id());

    for (auto& uniformFloat : _uniformFloat)
        context->setUniform(uniformFloat.first, uniformFloat.second);
    for (auto& uniformFloat2 : _uniformFloat2)
    {
        auto& float2 = uniformFloat2.second;

        context->setUniform(uniformFloat2.first, float2->x(), float2->y());
    }
    for (auto& uniformFloat3 : _uniformFloat3)
    {
        auto& float3 = uniformFloat3.second;

        context->setUniform(uniformFloat3.first, float3->x(), float3->y(), float3->z());
    }
    for (auto& uniformFloat4 : _uniformFloat4)
    {
        auto& float4 = uniformFloat4.second;

        context->setUniform(uniformFloat4.first, float4->x(), float4->y(), float4->z(), float4->w());
    }
    for (auto& uniformFloat16 : _uniformFloat16)
        context->setUniform(uniformFloat16.first, 1, true, uniformFloat16.second);

    for (uint textureId = 0; textureId < _textures.size(); ++textureId)
    {
        auto texture = _textures[textureId];

        context->setTextureAt(textureId, texture, _textureLocations[textureId]);
        if (texture > 0)
            context->setSamplerStateAt(
                textureId, _textureWrapMode[textureId], _textureFilters[textureId], _textureMipFilters[textureId]
            );
    }

    for (uint vertexBufferId = 0; vertexBufferId < _vertexBuffers.size(); ++vertexBufferId)
    {
        auto vertexBuffer = _vertexBuffers[vertexBufferId];

        if (vertexBuffer > 0)
            context->setVertexBufferAt(
                _vertexBufferLocations[vertexBufferId],
                vertexBuffer,
                _vertexAttributeSizes[vertexBufferId],
                _vertexSizes[vertexBufferId],
                _vertexAttributeOffsets[vertexBufferId]
            );
    }

	context->setBlendMode(_blendMode);
	context->setDepthTest(_depthMask, _depthFunc);
    context->setTriangleCulling(_triangleCulling);

    context->drawTriangles(_indexBuffer, _numIndices / 3);
}

void
DrawCall::watchProperty(const std::string& propertyName)
{
    /*
    _propertyChangedSlots.push_back(_data->propertyChanged(propertyName)->connect(std::bind(
        &DrawCall::boundPropertyChangedHandler,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
    )));
    */
}



void
DrawCall::propertyChangedHandler(std::shared_ptr<data::Container>  data,
                                 const std::string&                propertyName)
{

}

Container::Ptr
DrawCall::getDataContainer(const std::string& propertyName) const
{
	if (_data == nullptr || _rootData == nullptr)
		return nullptr;
	else if (_data->hasProperty(propertyName))
		return _data;
	else if (_rootData->hasProperty(propertyName))
		return _rootData;
	else
		return nullptr;
}

bool
DrawCall::dataHasProperty(const std::string& propertyName)
{
    return _data->hasProperty(propertyName) || _rootData->hasProperty(propertyName);
}
