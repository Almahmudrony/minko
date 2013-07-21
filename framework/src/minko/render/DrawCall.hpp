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
#include "minko/render/Blending.hpp"
#include "minko/data/Container.hpp"

namespace minko
{
	namespace render
	{
		class DrawCall :
            public std::enable_shared_from_this<DrawCall>
		{
		public:
			typedef std::shared_ptr<DrawCall> Ptr;

		private:
			typedef std::shared_ptr<AbstractContext>	AbsCtxPtr;
            typedef std::shared_ptr<data::Container>    ContainerPtr;

		private:
            static SamplerState                                         _defaultSamplerState;

			std::shared_ptr<Program>									_program;
			std::shared_ptr<data::Container>					        _data;
            std::shared_ptr<data::Container>					        _rootData;
			const std::unordered_map<std::string, std::string>&	        _attributeBindings;
			const std::unordered_map<std::string, std::string>&	        _uniformBindings;
			const std::unordered_map<std::string, std::string>&	        _stateBindings;
            std::shared_ptr<States>                                     _states;

			std::vector<std::function<void(AbsCtxPtr)>>			        _func;

            std::vector<int>                                            _vertexBuffers;
            std::vector<int>                                            _vertexBufferLocations;
            std::vector<int>                                            _vertexSizes;
            std::vector<int>                                            _vertexAttributeSizes;
            std::vector<int>                                            _vertexAttributeOffsets;
            std::vector<int>                                            _textures;
            std::vector<int>                                            _textureLocations;
            uint                                                        _numIndices;
            uint                                                        _indexBuffer;
            std::shared_ptr<render::Texture>                            _target;
            render::Blending::Mode                                      _blendMode;
            bool                                                        _depthMask;
            render::CompareMode                                         _depthFunc;
            render::TriangleCulling                                     _triangleCulling;

            std::list<Signal<ContainerPtr, const std::string&>::Slot>   _propertyChangedSlots;

		public:
			static inline
			Ptr
			create(std::shared_ptr<Program>								program,
				   ContainerPtr											data,
				   ContainerPtr											rootData,
				   const std::unordered_map<std::string, std::string>&	attributeBindings,
				   const std::unordered_map<std::string, std::string>&	uniformBindings,
				   const std::unordered_map<std::string, std::string>&	stateBindings,
                   std::shared_ptr<States>                              states)
			{
                auto dc = std::shared_ptr<DrawCall>(new DrawCall(
					program, attributeBindings, uniformBindings, stateBindings, states
				));

                dc->bind(data, rootData);

				return dc;
			}

            inline
            std::shared_ptr<Texture>
            target()
            {
                return _target;
            }

			void
			bind(ContainerPtr data, ContainerPtr rootData);

			void
			render(std::shared_ptr<AbstractContext> context);

			void
			initialize(ContainerPtr				                    data,
					   const std::map<std::string, std::string>&	inputNameToBindingName);

		private:
			DrawCall(std::shared_ptr<Program>								program,
				     const std::unordered_map<std::string, std::string>&	attributeBindings,
				     const std::unordered_map<std::string, std::string>&	uniformBindings,
					 const std::unordered_map<std::string, std::string>&	stateBindings,
                     std::shared_ptr<States>                                states);

			void
			bindStates();

            template <typename T>
            T
            getDataProperty(const std::string& propertyName)
            {
                //watchProperty(propertyName);

                if (_data->hasProperty(propertyName))
                    return _data->get<T>(propertyName);

                if (_rootData->hasProperty(propertyName))
                    return _rootData->get<T>(propertyName);

                throw;
            }

			template <typename T>
            T
            getDataProperty(const std::string& propertyName, T defaultValue)
            {
				if (dataHasProperty(propertyName))
					return _data->get<T>(propertyName);

				return defaultValue;
            }

            bool
            dataHasProperty(const std::string& propertyName);

            void
            watchProperty(const std::string& propertyName);

            void
            boundPropertyChangedHandler(ContainerPtr        data,
                                        const std::string&  propertyName);
		};		
	}
}
