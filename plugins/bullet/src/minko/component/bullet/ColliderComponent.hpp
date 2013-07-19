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
#include "minko/component/AbstractComponent.hpp"

namespace minko
{
	namespace component
	{
		namespace bullet
		{
			class Collider;
			class PhysicsWorld;

			class ColliderComponent:
				public AbstractComponent,
				public std::enable_shared_from_this<ColliderComponent>
			{
			public:
				typedef std::shared_ptr<ColliderComponent> Ptr;

			private:
				typedef std::shared_ptr<AbstractComponent>		AbsCtrlPtr;
				typedef std::shared_ptr<scene::Node>			NodePtr;
				typedef std::shared_ptr<scene::NodeSet>			NodeSetPtr;
				typedef std::shared_ptr<math::Vector3>			Vector3Ptr;
				typedef std::shared_ptr<math::Matrix4x4>		Matrix4x4Ptr;
				typedef std::shared_ptr<Collider>				ColliderPtr;
				typedef std::shared_ptr<Transform>				TransformPtr;
				typedef std::shared_ptr<PhysicsWorld>			PhysicsWorldPtr;
				

			private:
				ColliderPtr		_collider;
				PhysicsWorldPtr	_physicsWorld;
				TransformPtr	_targetTransform;

				Signal<AbsCtrlPtr, NodePtr>::Slot		_targetAddedSlot;
				Signal<AbsCtrlPtr, NodePtr>::Slot		_targetRemovedSlot;
				Signal<NodePtr, NodePtr, NodePtr>::Slot	_addedSlot;
				Signal<NodePtr, NodePtr, NodePtr>::Slot	_removedSlot;
				Signal<ColliderPtr>::Slot				_colliderTrfChangedSlot;

			public:
				inline static
				Ptr
				create(ColliderPtr collider)
				{
					Ptr ColliderComponent(new ColliderComponent(collider));

					ColliderComponent->initialize();

					return ColliderComponent;
				}

				void
				prependLocalTranslation(Vector3Ptr);

				void
				prependRotationY(float);

				void
				applyRelativeImpulse(Vector3Ptr);

			private:
				ColliderComponent(ColliderPtr);

				void
				initialize();

				void
				initializeFromTarget(NodePtr);

				void
				targetAddedHandler(AbsCtrlPtr, NodePtr);

				void
				targetRemovedHandler(AbsCtrlPtr, NodePtr);

				void 
				addedHandler(NodePtr, NodePtr, NodePtr);

				void
				removedHandler(NodePtr, NodePtr, NodePtr);

				void
				colliderTransformChangedHandler(ColliderPtr);

				void 
				updateColliderWorldTransform();
			};
		}
	}
}
