/*
Copyright (c) 2014 Aerys

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

#include "minko/file/EffectParser.hpp"

#include "minko/file/Loader.hpp"
#include "minko/data/Provider.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Program.hpp"
#include "minko/render/States.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Blending.hpp"
#include "minko/render/CompareMode.hpp"
#include "minko/render/WrapMode.hpp"
#include "minko/render/TextureFilter.hpp"
#include "minko/render/MipFilter.hpp"
#include "minko/render/TriangleCulling.hpp"
#include "minko/render/AbstractTexture.hpp"
#include "minko/render/Texture.hpp"
#include "minko/render/CubeTexture.hpp"
#include "minko/render/Pass.hpp"
#include "minko/render/Priority.hpp"
#include "minko/file/FileProtocol.hpp"
#include "minko/file/Options.hpp"
#include "minko/file/AssetLibrary.hpp"
#include "json/json.h"

using namespace minko;
using namespace minko::data;
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

std::unordered_map<std::string, render::CompareMode> EffectParser::_compareFuncMap = EffectParser::initializeCompareFuncMap();
std::unordered_map<std::string, render::CompareMode>
EffectParser::initializeCompareFuncMap()
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

std::unordered_map<std::string, render::StencilOperation> EffectParser::_stencilOpMap = EffectParser::initializeStencilOperationMap();
std::unordered_map<std::string, render::StencilOperation>
EffectParser::initializeStencilOperationMap()
{
	std::unordered_map<std::string, render::StencilOperation> m;

	m["keep"]			= render::StencilOperation::KEEP;
	m["zero"]			= render::StencilOperation::ZERO;
	m["replace"]		= render::StencilOperation::REPLACE;
	m["incr"]			= render::StencilOperation::INCR;
	m["incr_wrap"]		= render::StencilOperation::INCR_WRAP;
	m["decr"]			= render::StencilOperation::DECR;
	m["decr_wrap"]		= render::StencilOperation::DECR_WRAP;
	m["invert"]			= render::StencilOperation::INVERT;

	return m;
}

std::unordered_map<std::string, float> EffectParser::_priorityMap = EffectParser::initializePriorityMap();
std::unordered_map<std::string, float>
EffectParser::initializePriorityMap()
{
	std::unordered_map<std::string, float> m;

	// The higher the priority, the earlier the drawcall is rendered.
	m["first"]			= Priority::FIRST;
	m["background"]		= Priority::BACKGROUND;
	m["opaque"]			= Priority::OPAQUE;
	m["transparent"]	= Priority::TRANSPARENT;
	m["last"]			= Priority::LAST;

	return m;
}

std::string
EffectParser::typeToString(DefaultValue::Type t)
{
    if (t == DefaultValue::Type::BOOL)
        return "bool";
    if (t == DefaultValue::Type::INT)
        return "integer";
    if (t == DefaultValue::Type::FLOAT)
        return "float";
    if (t == DefaultValue::Type::TEXTURE)
        return "texture";

    throw;
}

float
EffectParser::getPriorityValue(const std::string& name)
{
	auto foundPriorityIt = _priorityMap.find(name);

	return foundPriorityIt != _priorityMap.end()
		? foundPriorityIt->second
		: _priorityMap["opaque"];
}

EffectParser::EffectParser() :
	_effect(nullptr),
	_numDependencies(0),
	_numLoadedDependencies(0)
{
}

void
EffectParser::parse(const std::string&				    filename,
				    const std::string&                  resolvedFilename,
                    std::shared_ptr<Options>            options,
				    const std::vector<unsigned char>&	data,
				    std::shared_ptr<AssetLibrary>	    assetLibrary)
{
	Json::Value root;
	Json::Reader reader;

	if (!reader.parse((const char*)&data[0], (const char*)&data[data.size() - 1], root, false))
		throw file::ParserError(resolvedFilename + ": " + reader.getFormattedErrorMessages());
    
    int pos	= resolvedFilename.find_last_of("/\\");

	_options = file::Options::create(options);

    if (pos != std::string::npos)
    {
        _options->includePaths().clear();
		_options->includePaths().push_front(resolvedFilename.substr(0, pos));
    }

	_filename = filename;
	_resolvedFilename = resolvedFilename;
	_assetLibrary = assetLibrary;
	_effectName	= root.get("name", filename).asString();

    parseGlobalScope(root, _globalScope);

    _effect = render::Effect::create(_effectName);
    if (_numDependencies == _numLoadedDependencies)
        finalize();
}

void
EffectParser::parseGlobalScope(const Json::Value& node, Scope& scope)
{
    parseAttributes(node, scope, scope.attributes);
    parseUniforms(node, scope, scope.uniforms);
    parseMacros(node, scope, scope.macros);
    parseStates(node, scope, scope.states);
    parsePasses(node, scope, scope.passes);
    parseTechniques(node, scope, scope.techniques);
}

bool
EffectParser::parseConfiguration(const Json::Value& node)
{
    auto confValue = node.get("configuration", 0);
    auto platforms = _options->platforms();
    auto userFlags = _options->userFlags();
    auto r = false;

    if (confValue.isArray())
    {
        for (auto value : confValue)
        {
            // if the config. token is a string and we can find it in the list of platforms,
            // then the configuration is ok and we return true
            if (value.isString() &&
                (std::find(platforms.begin(), platforms.end(), value.asString()) != platforms.end() ||
                std::find(userFlags.begin(), userFlags.end(), value.asString()) != userFlags.end()))
            {
                return true;
            }
            else if (value.isArray())
            {
                // if the config. token is an array, we check that *all* the string tokens are in
                // the platforms list; if a single of them is not there then the config. token
                // is considered to be false
                for (auto str : value)
                {
                        if (str.isString() &&
                            (std::find(platforms.begin(), platforms.end(), str.asString()) == platforms.end() ||
                            std::find(userFlags.begin(), userFlags.end(), str.asString()) != userFlags.end()))
                    {
                        r = r || false;
                        break;
                    }
                }
            }
        }
    }
    else
        return true;

    return r;
}

void
EffectParser::parseTechniques(const Json::Value& node, Scope& scope, Techniques& techniques)
{
    auto techniquesNode = node.get("techniques", 0);
    auto firstTechnique = true;

    if (techniquesNode.isArray())
    {
        for (auto techniqueNode : techniquesNode)
        {
            // FIXME: switch to fallback instead of ignoring
            if (!parseConfiguration(techniqueNode))
                continue;

            auto techniqueNameNode = techniqueNode.get("name", "technique" + std::to_string(techniques.size()));
            auto techniqueName = techniqueNameNode.asString();

            Scope techniqueScope(scope, scope);

            parseAttributes(node, techniqueScope, techniqueScope.attributes);
            parseUniforms(node, techniqueScope, techniqueScope.uniforms);
            parseMacros(node, techniqueScope, techniqueScope.macros);
            parseStates(node, techniqueScope, techniqueScope.states);
            parsePasses(node, techniqueScope, techniqueScope.passes);
            parsePasses(techniqueNode, techniqueScope, techniques[techniqueName]);

            if (firstTechnique)
            {
                firstTechnique = false;
                techniques["default"] = techniques[techniqueName];
            }
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parsePasses(const Json::Value& node, Scope& scope, std::vector<PassPtr>& passes)
{
    auto passesNode = node.get("passes", 0);

    if (passesNode.isArray())
    {
        for (auto passNode : passesNode)
        {
            // FIXME: switch to fallback instead of ignoring
            if (!parseConfiguration(passNode))
                continue;

            parsePass(passNode, scope, passes);
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parsePass(const Json::Value& node, Scope& scope, std::vector<PassPtr>& passes)
{
    // If the pass is just a string, we assume it's referencing an existing pass defined in an
    // ancestor scope. Thus, we loop up to the root global scope to find the pass by its name.
    if (node.isString())
    {
        auto passName = node.asString();
        const Scope* searchScope = &scope;
        Pass::Ptr pass = nullptr;

        do
        {
            auto passIt = std::find_if(searchScope->passes.begin(), searchScope->passes.end(), [&](PassPtr p)
            {
                return p->name() == passName;
            });

            if (passIt != searchScope->passes.end())
                pass = *passIt;
            else
                searchScope = searchScope->parent;
        }
        while (searchScope != nullptr && pass == nullptr);

        if (pass == nullptr)
            throw std::runtime_error("Undefined pass with name '" + passName + "'.");

        passes.push_back(pass);
    }
    else
    {
        // If the pass is an actual pass object, we parse all its data, create the correspondin
        // Pass object and add it to the vector.

        Scope passScope(scope, scope);

        parseAttributes(node, passScope, passScope.attributes);
        parseUniforms(node, passScope, passScope.uniforms);
        parseMacros(node, passScope, passScope.macros);
        parseStates(node, passScope, passScope.states);

        auto passName = "pass" + std::to_string(scope.passes.size());
        auto nameNode = node.get("name", 0);
        if (nameNode.isString())
            passName = nameNode.asString();
        // FIXME: throw otherwise

        auto vertexShader = parseShader(node.get("vertexShader", 0), passScope, Shader::Type::VERTEX_SHADER);
        auto fragmentShader = parseShader(node.get("fragmentShader", 0), passScope, Shader::Type::FRAGMENT_SHADER);

        passes.push_back(Pass::create(
            passName,
            Program::create(_options->context(), vertexShader, fragmentShader),
            passScope.attributes.bindings,
            passScope.uniforms.bindings,
            passScope.states.bindings,
            passScope.macros.bindings,
            States::create()
        ));
    }
}

void
EffectParser::parseDefaultValue(const Json::Value&    node,
                                const Scope&          scope,
                                DefaultValue&         value,
                                DefaultValue::Type    expectedType,
                                uint                  expectedSize)
{
    if (node.isArray() && node.size() != expectedSize)
        throw std::runtime_error(
            "Unexpected default value size: expected " + std::to_string(expectedSize) + ", got "
            + std::to_string(node.size())
        );
    
    parseDefaultValue(node, scope, value, expectedType);
}

void
EffectParser::parseDefaultValue(const Json::Value&    node,
                                const Scope&          scope,
                                DefaultValue&         value,
                                DefaultValue::Type    expectedType)
{
    if (!node.isObject() || node.get("default", 0).empty())
        return;

    auto defaultValueNode = node.get("default", 0);
    auto testNode = defaultValueNode.isArray() ? defaultValueNode[0] : defaultValueNode;

    if ((testNode.isBool() && expectedType != DefaultValue::Type::BOOL)
        || (testNode.isInt() && expectedType != DefaultValue::Type::INT)
        || (testNode.isDouble() && expectedType != DefaultValue::Type::FLOAT)
        || (testNode.isString() && expectedType != DefaultValue::Type::TEXTURE))
        throw std::runtime_error("Unexpected default value type.");

    parseDefaultValue(node, scope, value);
}

void
EffectParser::parseDefaultValue(const Json::Value& node, const Scope& scope, DefaultValue& value)
{
    if (!node.isObject() || node.get("default", 0).empty())
        return;
    auto defaultValueNode = node.get("default", 0);

    // if the value is an array we will recursively call parseDefaultValue and push each of them
    // in our DefaultValue data
    if (defaultValueNode.isArray())
    {
        for (auto valueNode : defaultValueNode)
            parseDefaultValue(valueNode, scope, value);

        return;
    }

    if (defaultValueNode.isBool())
    {
        if (value.values.size() != 0 && value.type != DefaultValue::Type::BOOL)
            throw std::runtime_error(
                "Inconsistent vector data: expected " + typeToString(value.type) + ", got "
                + typeToString(DefaultValue::Type::BOOL) + "."
            );

        value.values.push_back(defaultValueNode.asBool());
        value.type = DefaultValue::Type::BOOL;
    }
    else if (defaultValueNode.isInt())
    {
        if (value.values.size() != 0 && value.type != DefaultValue::Type::INT)
            throw std::runtime_error(
                "Inconsistent vector data: expected " + typeToString(value.type) + ", got "
                + typeToString(DefaultValue::Type::INT) + "."
            );

        value.values.push_back(defaultValueNode.asInt());
        value.type = DefaultValue::Type::INT;
    }
    else if (defaultValueNode.isDouble())
    {
        if (value.values.size() != 0 && value.type != DefaultValue::Type::FLOAT)
            throw std::runtime_error(
                "Inconsistent vector data: expected " + typeToString(value.type) + ", got "
                + typeToString(DefaultValue::Type::FLOAT) + "."
            );

        value.values.push_back(defaultValueNode.asFloat());
        value.type = DefaultValue::Type::FLOAT;

    }
    else if (defaultValueNode.isString())
    {
        if (value.values.size() != 0 && value.type != DefaultValue::Type::TEXTURE)
            throw std::runtime_error(
                "Inconsistent vector data: expected " + typeToString(value.type) + ", got "
                + typeToString(DefaultValue::Type::TEXTURE) + "."
            );

        value.type = DefaultValue::Type::TEXTURE;
        //value.values.push_back(node.asString());

        loadTexture(defaultValueNode.asString(), value);
    }
}

void
EffectParser::parseAttributes(const Json::Value& node, const Scope& scope, AttributeBlock& attributes)
{
    auto attributesNode = node.get("attributes", 0);

    if (attributesNode.isObject())
    {
        for (auto attributeName : attributesNode.getMemberNames())
        {
            auto attributeNode = attributesNode[attributeName];

            parseBinding(attributeNode, scope, attributes.bindings[attributeName]);
            parseDefaultValue(attributeNode, scope, attributes.defaultValues[attributeName], DefaultValue::Type::FLOAT);
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parseUniforms(const Json::Value& node, const Scope& scope, UniformBlock& uniforms)
{
    auto uniformsNode = node.get("uniforms", 0);

    if (uniformsNode.isObject())
    {
        for (auto uniformName : uniformsNode.getMemberNames())
        {
            auto uniformNode = uniformsNode[uniformName];

            parseBinding(uniformNode, scope, uniforms.bindings[uniformName]);
            parseDefaultValue(uniformNode, scope, uniforms.defaultValues[uniformName]);
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parseMacros(const Json::Value& node, const Scope& scope, MacroBlock& macros)
{
    auto macrosNode = node.get("macros", 0);

    if (macrosNode.isObject())
    {
        for (auto macroName : macrosNode.getMemberNames())
        {
            auto macroNode = macrosNode[macroName];

            parseBinding(macroNode, scope, macros.bindings[macroName]);
            parseMacroBinding(macroNode, scope, macros.bindings[macroName]);
            parseDefaultValue(macroNode, scope, macros.defaultValues[macroName]);
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parseStates(const Json::Value& node, const Scope& scope, StateBlock& states)
{
    auto statesNode = node.get("states", 0);

    if (statesNode.isObject())
    {
        for (auto stateName : statesNode.getMemberNames())
        {
            auto stateNode = statesNode[stateName];

            parseBinding(stateNode, scope, states.bindings[stateName]);
            // FIXME: parse default value
        }
    }
    // FIXME: throw otherwise
}

void
EffectParser::parseBinding(const Json::Value& node, const Scope& scope, Binding& binding)
{
    binding.source(Binding::Source::TARGET);

    if (node.isString())
    {
        binding.propertyName(node.asString());
    }
    else
    {
        auto bindingNode = node.get("binding", 0);

        if (bindingNode.isString())
        {
            binding.propertyName(bindingNode.asString());
        }
        else if (bindingNode.isObject())
        {
            auto propertyNode = bindingNode.get("property", 0);
            auto sourceNode = bindingNode.get("source", 0);

            if (propertyNode.isString())
                binding.propertyName(propertyNode.asString());
            // FIXME: throw otherwise

            if (sourceNode.isString())
            {
                auto sourceStr = sourceNode.asString();

                if (sourceStr == "target")
                    binding.source(Binding::Source::TARGET);
                else if (sourceStr == "renderer")
                    binding.source(Binding::Source::RENDERER);
                else if (sourceStr == "root")
                    binding.source(Binding::Source::ROOT);
            }
            // FIXME: throw otherwise
        }
    }
}

void
EffectParser::parseMacroBinding(const Json::Value& node, const Scope& scope, MacroBinding& binding)
{
    parseBinding(node, scope, binding);

    if (!node.isObject())
        return;

    auto typeNode = node.get("type", 0);
    if (typeNode.isString())
    {
        auto typeStr = typeNode.asString();

        if (typeStr == "bool")
            binding.type(data::Binding::Type::BOOL);
        else if (typeStr == "int")
            binding.type(data::Binding::Type::INT);
        else if (typeStr == "float")
            binding.type(data::Binding::Type::FLOAT);

        // FIXME: handle other Binding::Type values
    }
    // FIXME: throw otherwise

    auto minNode = node.get("min", "");
    if (minNode.isInt())
        binding.minValue(minNode.asInt());
    // FIXME: throw otherwise

    auto maxNode = node.get("max", "");
    if (maxNode.isInt())
        binding.maxValue(maxNode.asInt());
    // FIXME: throw otherwise
}

render::Shader::Ptr
EffectParser::parseShader(const Json::Value& node, const Scope& scope, render::Shader::Type type)
{
    if (!node.isString())
        throw;

    std::string glsl = node.asString();

    auto shader = Shader::create(_options->context(), type, glsl);
    auto blocks = std::shared_ptr<GLSLBlockList>(new GLSLBlockList());

    blocks->push_front(GLSLBlock(GLSLBlockType::TEXT, ""));
    _shaderToGLSL[shader] = blocks;
    parseGLSL(glsl, _options, blocks, blocks->begin());

    return shader;
}

void
EffectParser::parseGLSL(const std::string&      glsl,
                        file::Options::Ptr 		options,
                        GLSLBlockListPtr		blocks,
                        GLSLBlockList::iterator	insertIt)
{
    std::string line;
    std::stringstream stream(glsl);
    auto i = 0;
    auto lastBlockEnd = 0;
    auto numIncludes = 0;

    while (std::getline(stream, line))
    {
        auto firstSharpPos = line.find_first_of('#');
        if (firstSharpPos != std::string::npos && line.substr(firstSharpPos, 16) == "#pragma include "
            && (line[firstSharpPos + 16] == '"' || line[firstSharpPos + 16] == '\''))
        {
            auto filename = line.substr(firstSharpPos + 17, line.find_last_of(line[firstSharpPos + 16]) - 17 - firstSharpPos);

            if (lastBlockEnd != i)
                insertIt = blocks->insert_after(insertIt, GLSLBlock(GLSLBlockType::TEXT, glsl.substr(lastBlockEnd, i - lastBlockEnd)));
            insertIt = blocks->insert_after(insertIt, GLSLBlock(GLSLBlockType::FILE, filename));

            lastBlockEnd = i + line.size() + 1;

            ++numIncludes;
        }
        i += line.size() + 1;
    }

    if (i != lastBlockEnd)
        insertIt = blocks->insert_after(insertIt, GLSLBlock(GLSLBlockType::TEXT, glsl.substr(lastBlockEnd)));

    if (numIncludes)
        loadGLSLDependencies(blocks, options);
}

void
EffectParser::loadGLSLDependencies(GLSLBlockListPtr blocks, file::Options::Ptr options)
{
    auto loader = Loader::create(options);

    for (auto blockIt = blocks->begin(); blockIt != blocks->end(); blockIt++)
    {
        auto& block = *blockIt;

        if (block.first == GLSLBlockType::FILE)
        {
            if (options->assetLibrary()->hasBlob(block.second))
            {
                auto& data = options->assetLibrary()->blob(block.second);

                block.first = GLSLBlockType::TEXT;
#ifdef DEBUG
                block.second = "//#pragma include(\"" + block.second + "\")\n";
#else
                block.second = "\n";
#endif
                parseGLSL(std::string((const char*)&data[0], data.size()), options, blocks, blockIt);
            }
            else
            {
                ++_numDependencies;

                _loaderCompleteSlots[loader] = loader->complete()->connect(std::bind(
                    &EffectParser::glslIncludeCompleteHandler,
                    std::static_pointer_cast<EffectParser>(shared_from_this()),
                    std::placeholders::_1,
                    blocks,
                    blockIt,
                    block.second
                ));

                _loaderErrorSlots[loader] = loader->error()->connect(std::bind(
                    &EffectParser::dependencyErrorHandler,
                    std::static_pointer_cast<EffectParser>(shared_from_this()),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    block.second
                ));

                loader->queue(block.second);
            }
        }
    }

    if (_numDependencies != _numLoadedDependencies)
        loader->load();

    if (_numDependencies == _numLoadedDependencies && _effect)
        finalize();
}

void
EffectParser::dependencyErrorHandler(std::shared_ptr<Loader> loader, const ParserError& error, const std::string& filename)
{
    /*LOG_ERROR("unable to load dependency '" << filename << "', included paths are:");
    for (auto& path : loader->options()->includePaths())
        LOG_ERROR("\t" << path);*/

    throw file::ParserError("Unable to load dependencies.");
}

void
EffectParser::glslIncludeCompleteHandler(LoaderPtr 			        loader,
                                         GLSLBlockListPtr 			blocks,
                                         GLSLBlockList::iterator 	blockIt,
                                         const std::string&         filename)
{
    auto& block = *blockIt;

    block.first = GLSLBlockType::TEXT;

    // FIXME: use the GLSL "line" directive instead
#ifdef DEBUG
    block.second = "//#pragma include(\"" + filename + "\")\n";
#else
    block.second = "\n";
#endif

    ++_numLoadedDependencies;

    auto file = loader->files().at(filename);
    auto resolvedFilename = file->resolvedFilename();
    auto options = _options;
    auto pos = resolvedFilename.find_last_of("/\\");

    if (pos != std::string::npos)
    {
        options = file::Options::create(options);
        options->includePaths().clear();
        options->includePaths().push_back(resolvedFilename.substr(0, pos));
    }

    parseGLSL(std::string((const char*)&file->data()[0], file->data().size()), options, blocks, blockIt);

    if (_numDependencies == _numLoadedDependencies && _effect)
        finalize();
}

void
EffectParser::loadTexture(const std::string&	textureFilename,
						  DefaultValue&	        value)
{
    auto loader = Loader::create(_options);

	_numDependencies++;

	_loaderCompleteSlots[loader] = loader->complete()->connect([&](file::Loader::Ptr loader)
	{
        auto texture = _assetLibrary->texture(textureFilename);

        //value.textureValues.push_back(texture);
		texture->upload();

        ++_numLoadedDependencies;

        if (_numDependencies == _numLoadedDependencies && _effect)
            finalize();
	});

	_loaderErrorSlots[loader] = loader->error()->connect(std::bind(
		&EffectParser::dependencyErrorHandler,
		std::static_pointer_cast<EffectParser>(shared_from_this()),
        std::placeholders::_1,
		std::placeholders::_2,
        textureFilename
	));

	loader->queue(textureFilename)->load();
}

std::shared_ptr<render::States>
EffectParser::createStates(const StateBlock& block)
{
    auto statePropertyNames = {
        "blendMode",
        "colorMask",
        "depthMask",
        "depthFunc",
        "triangleCulling",
        "stencilFunc",
        "stencilRef",
        "stencilMask",
        "stencilFailOp",
        "stencilZFailOp",
        "stencilZPassOp",
        "scissorBox",
        "priority",
        "zSort"
    };

    // blendMode

    return nullptr;
}

std::string
EffectParser::concatenateGLSLBlocks(GLSLBlockListPtr blocks)
{
    std::string glsl = "";

    for (auto& block : *blocks)
        glsl += block.second;

    return glsl;
}

void
EffectParser::finalize()
{
    for (auto& technique : _globalScope.techniques)
    {
        _effect->addTechnique(technique.first, technique.second);

        for (auto pass : technique.second)
        {
            auto vs = pass->program()->vertexShader();
            auto fs = pass->program()->fragmentShader();

            vs->source("#define VERTEX_SHADER\n" + concatenateGLSLBlocks(_shaderToGLSL[vs]));
            fs->source("#define FRAGMENT_SHADER\n" + concatenateGLSLBlocks(_shaderToGLSL[fs]));
        }
    }

    _options->assetLibrary()->effect(_filename, _effect);

    _complete->execute(shared_from_this());
}
