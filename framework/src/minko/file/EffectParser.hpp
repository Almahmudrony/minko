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

#pragma once

#include "minko/Common.hpp"

#include "minko/Signal.hpp"
#include "minko/file/AbstractParser.hpp"
#include "minko/file/Loader.hpp"
#include "minko/render/Blending.hpp"
#include "minko/render/Shader.hpp"

namespace Json
{
	class Value;
}

namespace minko
{
	namespace file
	{
		class EffectParser :
			public std::enable_shared_from_this<EffectParser>,
			public AbstractParser
		{
		public:
			typedef std::shared_ptr<EffectParser>	Ptr;

		private:
			typedef std::shared_ptr<AbstractLoader>						LoaderPtr;
			typedef std::shared_ptr<render::Effect>						EffectPtr;
			typedef std::shared_ptr<render::Pass>						PassPtr;
			typedef std::shared_ptr<render::Shader>						ShaderPtr;
			typedef std::shared_ptr<render::Texture>					TexturePtr;
			typedef std::unordered_map<std::string, TexturePtr>			TexturePtrMap;
			typedef std::unordered_map<std::string, std::vector<float>>	StringToFloatsMap;

		private:
			static std::unordered_map<std::string, unsigned int>		_blendFactorMap;
			static std::unordered_map<std::string, render::CompareMode>	_depthFuncMap;

            std::string                                                 _filename;
			std::shared_ptr<render::Effect>								_effect;
			std::string													_effectName;
			
			std::string													_defaultTechnique;
			std::shared_ptr<render::States>								_defaultStates;

            data::BindingMap				                            _defaultAttributeBindings;
			data::BindingMap				                            _defaultUniformBindings;
			data::BindingMap				                            _defaultStateBindings;
			data::MacroBindingMap                              			_defaultMacroBindings;
			std::unordered_map<std::string, std::vector<float>>			_defaultUniformValues;

			unsigned int												_numDependencies;
			unsigned int												_numLoadedDependencies;
			std::shared_ptr<AssetLibrary>								_assetLibrary;
			std::vector<LoaderPtr>										_effectIncludes;
			std::unordered_map<PassPtr, std::vector<LoaderPtr>> 		_passIncludes;
			std::unordered_map<ShaderPtr, std::vector<LoaderPtr>> 		_shaderIncludes;

			std::vector<PassPtr>										_globalPasses;
			std::unordered_map<std::string, TexturePtr>					_globalTargets;
			std::unordered_map<std::string, TexturePtrMap>				_techniqueTargets;
			std::unordered_map<std::string, std::vector<PassPtr>>		_techniquePasses;
			std::unordered_map<std::string, std::string>				_techniqueFallback;
			
			std::unordered_map<LoaderPtr, Signal<LoaderPtr>::Slot>		_loaderCompleteSlots;
			std::unordered_map<LoaderPtr, Signal<LoaderPtr>::Slot>		_loaderErrorSlots;

		public:
			inline static
			Ptr
			create()
			{
				return std::shared_ptr<EffectParser>(new EffectParser());
			}

			inline
			std::shared_ptr<render::Effect>
			effect()
			{
				return _effect;
			}

			inline
			const std::string&
			effectName()
			{
				return _effectName;
			}

			void
			parse(const std::string&				filename,
				  const std::string&                resolvedFilename,
                  std::shared_ptr<Options>          options,
				  const std::vector<unsigned char>&	data,
				  std::shared_ptr<AssetLibrary>		assetLibrary);

			void
			finalize();

		private:
			EffectParser();

			std::shared_ptr<render::States>
			parseRenderStates(Json::Value&								root,
							  std::shared_ptr<render::AbstractContext>	context,
							  TexturePtrMap&							targets,
							  std::shared_ptr<render::States>			defaultStates);

			void
			parseDefaultValues(Json::Value& root);

			void
			parsePasses(Json::Value& 								root,
						const std::string& 							resolvedFilename,
						std::shared_ptr<file::Options> 				options,
						std::shared_ptr<render::AbstractContext>	context,
						std::vector<PassPtr>&						passes,
						TexturePtrMap&								targets,
						data::BindingMap&							defaultAttributeBindings,
						data::BindingMap&							defaultUniformBindings,
						data::BindingMap&							defaultStateBindings,
						data::MacroBindingMap&						defaultMacroBindings,
						std::shared_ptr<render::States>				defaultStates,
						StringToFloatsMap&							defaultUniformDefaultValues);

			std::shared_ptr<render::Shader>
			parseShader(Json::Value& 					shaderNode,
						const std::string&				resolvedFilename,
						std::shared_ptr<file::Options>  options,
						render::Shader::Type 			type);

			void
			parseBindings(Json::Value&				contextNode,
						  data::BindingMap&			attributeBindings,
						  data::BindingMap&			uniformBindings,
						  data::BindingMap&			stateBindings,
						  data::MacroBindingMap&	macroBindings,
						  StringToFloatsMap&		uniformDefaultValues);

			void
			parseBlendMode(Json::Value&						contextNode,
						   render::Blending::Source&		srcFactor,
						   render::Blending::Destination&	dstFactor);

			void
			parseDepthTest(Json::Value&			contextNode,
						   bool&				depthMask,
						   render::CompareMode&	depthFunc);

            void
            parseTriangleCulling(Json::Value&               contextNode,
                                 render::TriangleCulling&   triangleCulling);

            void
            parseSamplerStates(Json::Value&                                             contextNode,
                               std::unordered_map<std::string, render::SamplerState>&   samplerStates);

            std::shared_ptr<render::Texture>
            parseTarget(Json::Value&                                contextNode,
                        std::shared_ptr<render::AbstractContext>    context,
                        TexturePtrMap&								targets);

			void
			parseDependencies(Json::Value& 						root,
							  const std::string& 				filename,
							  std::shared_ptr<file::Options> 	options,
							  std::vector<LoaderPtr>& 			store);

			void
			parseTechniques(Json::Value&								root,
							const std::string&							filename,
							std::shared_ptr<file::Options>				options,
							std::shared_ptr<render::AbstractContext>	context);

			void
			dependencyCompleteHandler(std::shared_ptr<AbstractLoader> loader);


			void
			dependencyErrorHandler(std::shared_ptr<AbstractLoader> loader);

			std::string
			concatenateIncludes(std::vector<LoaderPtr>& store);

			static
			std::unordered_map<std::string, unsigned int>
			initializeBlendFactorMap();

			static
			std::unordered_map<std::string, render::CompareMode>
			initializeDepthFuncMap();
		};
	}
}
