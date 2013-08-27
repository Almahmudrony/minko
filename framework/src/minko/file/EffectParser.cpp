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

#include "EffectParser.hpp"

#include "minko/data/Provider.hpp"
#include "minko/data/Binding.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Shader.hpp"
#include "minko/render/Program.hpp"
#include "minko/render/States.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Blending.hpp"
#include "minko/render/CompareMode.hpp"
#include "minko/render/WrapMode.hpp"
#include "minko/render/TextureFilter.hpp"
#include "minko/render/MipFilter.hpp"
#include "minko/render/TriangleCulling.hpp"
#include "minko/render/Texture.hpp"
#include "minko/render/Pass.hpp"
#include "minko/file/Loader.hpp"
#include "minko/file/Options.hpp"
#include "minko/file/AssetLibrary.hpp"
#include "json/json.h"

using namespace minko;
using namespace minko::file;
using namespace minko::render;

std::unordered_map<std::string, unsigned int> EffectParser::_blendFactorMap = EffectParser::initializeBlendFactorMap();
std::unordered_map<std::string, unsigned int>
EffectParser::initializeBlendFactorMap()
{
	std::unordered_map<std::string, unsigned int> m;

	m["src_zero"]					= static_cast<uint>(render::Blending::Source::ZERO);
    m["src_one"]					= static_cast<uint>(render::Blending::Source::ONE);
    m["src_color"]					= static_cast<uint>(render::Blending::Source::SRC_COLOR);
    m["src_one_minus_src_color"]	= static_cast<uint>(render::Blending::Source::ONE_MINUS_SRC_COLOR);
    m["src_src_alpha"]				= static_cast<uint>(render::Blending::Source::SRC_ALPHA);
    m["src_one_minus_src_alpha"]	= static_cast<uint>(render::Blending::Source::ONE_MINUS_SRC_ALPHA);
    m["src_dst_alpha"]				= static_cast<uint>(render::Blending::Source::DST_ALPHA);
    m["src_one_minus_dst_alpha"]	= static_cast<uint>(render::Blending::Source::ONE_MINUS_DST_ALPHA);

    m["dst_zero"]					= static_cast<uint>(render::Blending::Destination::ZERO);
    m["dst_one"]					= static_cast<uint>(render::Blending::Destination::ONE);
	m["dst_dst_color"]				= static_cast<uint>(render::Blending::Destination::DST_COLOR);
    m["dst_one_minus_dst_color"]	= static_cast<uint>(render::Blending::Destination::ONE_MINUS_DST_COLOR);
    m["dst_src_alpha_saturate"]		= static_cast<uint>(render::Blending::Destination::SRC_ALPHA_SATURATE);
    m["dst_one_minus_src_alpha"]	= static_cast<uint>(render::Blending::Destination::ONE_MINUS_SRC_ALPHA);
    m["dst_dst_alpha"]				= static_cast<uint>(render::Blending::Destination::DST_ALPHA);
    m["dst_one_minus_dst_alpha"]	= static_cast<uint>(render::Blending::Destination::ONE_MINUS_DST_ALPHA);

	m["default"]					= static_cast<uint>(render::Blending::Mode::DEFAULT);
	m["alpha"]						= static_cast<uint>(render::Blending::Mode::ALPHA);
	m["additive"]					= static_cast<uint>(render::Blending::Mode::ADDITIVE);

	return m;
}

std::unordered_map<std::string, render::CompareMode> EffectParser::_depthFuncMap = EffectParser::initializeDepthFuncMap();
std::unordered_map<std::string, render::CompareMode>
EffectParser::initializeDepthFuncMap()
{
	std::unordered_map<std::string, render::CompareMode> m;

	m["always"]			= render::CompareMode::ALWAYS;
	m["equal"]			= render::CompareMode::EQUAL;
	m["greater"]		= render::CompareMode::GREATER;
	m["greater_equal"]	= render::CompareMode::GREATER_EQUAL;
	m["less"]			= render::CompareMode::LESS;
	m["less_equal"]		= render::CompareMode::LESS_EQUAL;
	m["never"]			= render::CompareMode::NEVER;
	m["not_equal"]		= render::CompareMode::NOT_EQUAL;

	return m;
}

EffectParser::EffectParser() :
	_numDependencies(0),
	_numLoadedDependencies(0),
    _defaultPriority(0.f),
    _defaultBlendSrcFactor(Blending::Source::ONE),
    _defaultBlendDstFactor(Blending::Destination::ZERO),
    _defaultDepthMask(true),
    _defaultDepthFunc(CompareMode::LESS),
    _defaultTriangleCulling(TriangleCulling::BACK)
{
}

void
EffectParser::parse(const std::string&				    filename,
				    const std::string&                  resolvedFilename,
                    std::shared_ptr<Options>            options,
				    const std::vector<unsigned char>&	data,
				    std::shared_ptr<AssetLibrary>	    AssetLibrary)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse((const char*)&data[0], (const char*)&data[data.size() - 1],	root, false))
    {
        std::cerr << resolvedFilename << ":" << reader.getFormattedErrorMessages() << std::endl;

		throw std::invalid_argument("data");
    }

    _filename = filename;
	_AssetLibrary = AssetLibrary;
	_effectName = root.get("name", filename).asString();

	_defaultPriority = root.get("priority", 0.f).asFloat();
	parseDefaultValues(root);
	parsePasses(root, options);
	parseDependencies(root, resolvedFilename, options);
	
	if (_numDependencies == 0)
		finalize();
}

void
EffectParser::parseDefaultValues(Json::Value& root)
{
	parseBindings(
		root,
		_defaultAttributeBindings,
		_defaultUniformBindings,
		_defaultStateBindings,
		_defaultMacroBindings
	);

	parseBlendMode(root, _defaultBlendSrcFactor, _defaultBlendDstFactor);

	parseDepthTest(root, _defaultDepthMask, _defaultDepthFunc);

    parseTriangleCulling(root, _defaultTriangleCulling);

    parseSamplerStates(root, _defaultSamplerStates);
}

void
EffectParser::parsePasses(Json::Value& root, file::Options::Ptr options)
{
	std::vector<std::shared_ptr<render::Pass>> passes;
    std::unordered_map<std::string, std::shared_ptr<Texture>> targets;
	auto passId = 0;

	for (auto pass : root.get("passes", 0))
	{
		auto name = pass.get("name", std::to_string(passId++)).asString();

		// pass bindings
		data::BindingMap	attributeBindings(_defaultAttributeBindings);
		data::BindingMap	uniformBindings(_defaultUniformBindings);
		data::BindingMap	stateBindings(_defaultStateBindings);
		data::BindingMap	macroBindings(_defaultMacroBindings);
        
		parseBindings(pass, attributeBindings, uniformBindings, stateBindings, macroBindings);

		// pass priority
		auto priority = pass.get("priority", _defaultPriority).asFloat();

		// blendMode
		auto blendSrcFactor	= _defaultBlendSrcFactor;
		auto blendDstFactor	= _defaultBlendDstFactor;

		parseBlendMode(pass, blendSrcFactor, blendDstFactor);

		// depthTest
		auto depthMask	= _defaultDepthMask;
		auto depthFunc	= _defaultDepthFunc;

		parseDepthTest(pass, depthMask, depthFunc);

        // triangle culling
        auto triangleCulling  = _defaultTriangleCulling;

        parseTriangleCulling(pass, triangleCulling);

        // sampler states
        std::unordered_map<std::string, SamplerState>   samplerStates(_defaultSamplerStates);

        parseSamplerStates(pass, samplerStates);

		// program
		auto vertexShader	= Shader::create(
			options->context(), Shader::Type::VERTEX_SHADER, pass.get("vertexShader", 0).asString()
		);
		auto fragmentShader	= Shader::create(
			options->context(), Shader::Type::FRAGMENT_SHADER, pass.get("fragmentShader", 0).asString()
		);

        std::string targetName;
        auto target = parseTarget(pass, options->context(), targetName);

        if (!targetName.empty())
            targets[targetName] = target;

        passes.push_back(render::Pass::create(
			name,
			Program::create(options->context(), vertexShader, fragmentShader),
			attributeBindings,
			uniformBindings,
			stateBindings,
			macroBindings,
            States::create(
                samplerStates,
			    priority,
			    blendSrcFactor,
			    blendDstFactor,
			    depthMask,
			    depthFunc,
                triangleCulling,
                target
            )
		));
	}

	_effect = render::Effect::create(passes);
    for (auto target : targets)
        _effect->data()->set(target.first, target.second);
}

void
EffectParser::parseBlendMode(Json::Value&					contextNode,
						     render::Blending::Source&		srcFactor,
						     render::Blending::Destination&	dstFactor)
{
	auto blendModeArray	= contextNode.get("blendMode", 0);
	
	if (blendModeArray.isArray())
	{
		auto blendSrcFactorString = "src_" + blendModeArray[0].asString();
		if (_blendFactorMap.count(blendSrcFactorString))
			srcFactor = static_cast<render::Blending::Source>(_blendFactorMap[blendSrcFactorString]);

		auto blendDstFactorString = "dst_" + blendModeArray[1].asString();
		if (_blendFactorMap.count(blendDstFactorString))
			dstFactor = static_cast<render::Blending::Destination>(_blendFactorMap[blendDstFactorString]);
	}
	else if (blendModeArray.isString())
	{
		auto blendModeString = blendModeArray.asString();

		if (_blendFactorMap.count(blendModeString))
		{
			auto blendMode = _blendFactorMap[blendModeString];

			srcFactor = static_cast<render::Blending::Source>(blendMode & 0x00ff);
			dstFactor = static_cast<render::Blending::Destination>(blendMode & 0xff00);
		}
	}
}

void
EffectParser::parseDepthTest(Json::Value& contextNode, bool& depthMask, render::CompareMode& depthFunc)
{
	auto depthTest	= contextNode.get("depthTest", 0);
	
	if (depthTest.isObject())
	{
        auto depthMaskValue = depthTest.get("depthMask", 0);
        auto depthFuncValue = depthTest.get("depthFunc", 0);

        if (depthMaskValue.isBool())
            depthMask = depthMaskValue.asBool();

        if (depthFuncValue.isString())
    		depthFunc = _depthFuncMap[depthFuncValue.asString()];
	}
    else if (depthTest.isArray())
    {
        depthMask = depthTest[0].asBool();
		depthFunc = _depthFuncMap[depthTest[1].asString()];
    }
}

void
EffectParser::parseTriangleCulling(Json::Value& contextNode, TriangleCulling& triangleCulling)
{
    auto triangleCullingValue   = contextNode.get("triangleCulling", 0);

    if (triangleCullingValue.isString())
    {
        auto triangleCullingString = triangleCullingValue.asString();

        if (triangleCullingString == "back")
            triangleCulling = TriangleCulling::BACK;
        else if (triangleCullingString == "front")
            triangleCulling = TriangleCulling::FRONT;
        else if (triangleCullingString == "both")
            triangleCulling = TriangleCulling::BOTH;
        else if (triangleCullingString == "none")
            triangleCulling = TriangleCulling::NONE;
    }
}

void
EffectParser::parseBindings(Json::Value&	    contextNode,
						    data::BindingMap&   attributeBindings,
						    data::BindingMap&	uniformBindings,
						    data::BindingMap&	stateBindings,
							data::BindingMap&	macroBindings)
{
	auto attributeBindingsValue = contextNode.get("attributeBindings", 0);
	if (attributeBindingsValue.isObject())
		for (auto propertyName : attributeBindingsValue.getMemberNames())
			attributeBindings[propertyName] = parseBinding(attributeBindingsValue.get(propertyName, 0));

	auto uniformBindingsValue = contextNode.get("uniformBindings", 0);
	if (uniformBindingsValue.isObject())
		for (auto propertyName : uniformBindingsValue.getMemberNames())
			uniformBindings[propertyName] = parseBinding(uniformBindingsValue.get(propertyName, 0));

	auto stateBindingsValue = contextNode.get("stateBindings", 0);
	if (stateBindingsValue.isObject())
		for (auto propertyName : stateBindingsValue.getMemberNames())
			stateBindings[propertyName] = parseBinding(stateBindingsValue.get(propertyName, 0));

	auto macroBindingsValue = contextNode.get("macroBindings", 0);
	if (macroBindingsValue.isObject())
		for (auto propertyName : macroBindingsValue.getMemberNames())
			macroBindings[propertyName] = parseBinding(macroBindingsValue.get(propertyName, 0));
}

data::Binding::Ptr
EffectParser::parseBinding(Json::Value& contextNode)
{
    if (contextNode.isString())
        return data::Binding::create(
            contextNode.asString(),
            data::Binding::Flag::NONE
        );
    else if (contextNode.isObject())
        return data::Binding::create(
            contextNode.get("propertyName", 0).asString(),
            parseBindingFlags(contextNode.get("flags", 0))
        );
}

data::Binding::Flag
EffectParser::parseBindingFlags(Json::Value& contextNode)
{
    data::Binding::Flag flags = data::Binding::Flag::NONE;

    if (contextNode.isString())
        flags = stringToBindingFlag(contextNode.asString());
    else if (contextNode.isArray())
        for (auto flagValue : contextNode)
            flags = static_cast<data::Binding::Flag>(flags | stringToBindingFlag(flagValue.asString()));

    return flags;
}

data::Binding::Flag
EffectParser::stringToBindingFlag(const std::string& str)
{
    if (str == "property_exists")
        return data::Binding::Flag::PROPERTY_EXISTS;
    else if (str == "push")
        return data::Binding::Flag::PUSH;

    return data::Binding::Flag::NONE;
}

void
EffectParser::parseSamplerStates(Json::Value&                                           contextNode,
                                 std::unordered_map<std::string, render::SamplerState>& samplerStates)
{
    auto samplerStatesValue = contextNode.get("samplerStates", 0);

    if (samplerStatesValue.isObject())
        for (auto propertyName : samplerStatesValue.getMemberNames())
        {
            auto samplerStateValue = samplerStatesValue.get(propertyName, 0);

            if (samplerStateValue.isObject())
            {
                auto wrapModeStr        = samplerStateValue.get("wrapMode", "clamp").asString();
                auto textureFilterStr   = samplerStateValue.get("textureFilter", "nearest").asString();
                auto mipFilterStr       = samplerStateValue.get("mipFilter", "linear").asString();

                auto wrapMode           = wrapModeStr == "repeat" ? WrapMode::REPEAT : WrapMode::CLAMP;
                auto textureFilter      = textureFilterStr == "linear"
                    ? TextureFilter::LINEAR
                    : TextureFilter::NEAREST;
                auto mipFilter          = mipFilterStr == "linear"
                    ? MipFilter::LINEAR
                    : (mipFilterStr == "nearest" ? MipFilter::NEAREST : MipFilter::NONE);

                samplerStates[propertyName] = SamplerState(wrapMode, textureFilter, mipFilter);
            }
        }
}

std::shared_ptr<render::Texture>
EffectParser::parseTarget(Json::Value&                      contextNode,
                          std::shared_ptr<AbstractContext>  context,
                          std::string&                      name)
{
    auto targetValue = contextNode.get("target", 0);

    if (targetValue.isObject())
    {
        auto nameValue  = targetValue.get("name", 0);

        if (nameValue.isString())
        {
            name = nameValue.asString();
            if (_AssetLibrary->texture(name))
                return _AssetLibrary->texture(name);
        }

        auto sizeValue  = targetValue.get("size", 0);
        auto width      = 0;
        auto height     = 0;

        if (!sizeValue.empty())
            width = height = sizeValue.asUInt();
        else
        {
            width = targetValue.get("width", 0).asUInt();
            height = targetValue.get("height", 0).asUInt();
        }

        auto target     = render::Texture::create(context, width, height, true);

        _AssetLibrary->texture(name, target);

        return target;
    }

    return nullptr;
}

void
EffectParser::parseDependencies(Json::Value& root, const std::string& filename, file::Options::Ptr options)
{
	auto require	= root.get("includes", 0);
	int pos			= filename.find_last_of("/");

	if (pos > 0)
	{
		options = file::Options::create(options);
		options->includePaths().insert(filename.substr(0, pos));
	}

	if (require.isArray())
	{
		_numDependencies = require.size();

		for (unsigned int requireId = 0; requireId < _numDependencies; requireId++)
		{
			auto loader = Loader::create();

			_loaderCompleteSlots[loader] = loader->complete()->connect(std::bind(
				&EffectParser::dependencyCompleteHandler, shared_from_this(), std::placeholders::_1
			));
			_loaderErrorSlots[loader] = loader->error()->connect(std::bind(
				&EffectParser::dependencyErrorHandler, shared_from_this(), std::placeholders::_1
			));

			loader->load(require[requireId].asString(), options);
		}
	}
}

void
EffectParser::dependencyCompleteHandler(std::shared_ptr<Loader> loader)
{
	++_numLoadedDependencies;

	_dependenciesCode += std::string((char*)&loader->data()[0], loader->data().size()) + "\r\n";

	if (_numDependencies == _numLoadedDependencies)
		finalize();
}

void
EffectParser::dependencyErrorHandler(std::shared_ptr<Loader> loader)
{
	std::cout << "error" << std::endl;
}

void
EffectParser::finalize()
{
	for (auto& pass : _effect->passes())
    {
		auto program = pass->program();

		program->vertexShader()->source(_dependenciesCode + program->vertexShader()->source());
		program->fragmentShader()->source(_dependenciesCode + program->fragmentShader()->source());
    }

	_AssetLibrary->effect(_effectName, _effect);
    _AssetLibrary->effect(_filename, _effect);

	_complete->execute(shared_from_this());
}
