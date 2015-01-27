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

#pragma once

#include "minko/Common.hpp"
#include "minko/component/AbstractComponent.hpp"
#include "minko/Signal.hpp"
#include "minko/scene/Layout.hpp"

namespace minko
{
	namespace component
	{
		class Culling :
			public AbstractComponent
		{
		public:
			typedef std::shared_ptr<Culling>    Ptr;

		private:
			typedef std::shared_ptr<scene::Node>			    		NodePtr;
			typedef std::shared_ptr<math::AbstractShape>	    		ShapePtr;
            typedef std::shared_ptr<data::Provider>             		ProviderPtr;
            typedef Flyweight<std::string>		                		String;
            typedef Signal<data::Store&, ProviderPtr, const String&>   	PropertyChangedSignal;

		private:
			std::shared_ptr<math::OctTree>			        _octTree;

			std::string										_bindProperty;
			std::shared_ptr<math::AbstractShape>			_frustum;

			Signal<AbstractComponent::Ptr, NodePtr>::Slot	_targetAddedSlot;
            Signal<AbstractComponent::Ptr, NodePtr>::Slot	_targetRemovedSlot;
			Signal<NodePtr, NodePtr, NodePtr>::Slot			_addedSlot;
			Signal<NodePtr, NodePtr, NodePtr>::Slot			_addedToSceneSlot;
			Signal<NodePtr, NodePtr>::Slot					_layoutChangedSlot;
			PropertyChangedSignal::Slot	                    _viewMatrixChangedSlot;

		public:
			inline static
			Ptr
			create(ShapePtr	shape, const std::string& bindPropertyName)
			{
                return std::shared_ptr<Culling>(new Culling(shape, bindPropertyName));
			}

        protected:
			void
            targetAdded(NodePtr target);

            void
            targetRemoved(NodePtr target);

		private:
			Culling(ShapePtr shape, const std::string& bindProperty);

            void
            addedHandler(NodePtr node, NodePtr target, NodePtr ancestor);

			void
			layoutChangedHandler(NodePtr node, NodePtr target);

			void
			worldToScreenChangedHandler(data::Store&    data,
                                        const String&	propertyName);

			void
			targetAddedToSceneHandler(NodePtr node, NodePtr target, NodePtr ancestor);
		};
	}
}
