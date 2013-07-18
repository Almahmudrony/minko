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

#include "Transform.hpp"

#include "minko/math/Matrix4x4.hpp"
#include "minko/scene/Node.hpp"
#include "minko/scene/NodeSet.hpp"
#include "minko/data/Container.hpp"
#include "minko/data/Provider.hpp"

using namespace minko::component;
using namespace minko::math;

Transform::Transform() :
	minko::component::AbstractComponent(),
	_transform(Matrix4x4::create()),
	_modelToWorld(Matrix4x4::create()),
	_data(data::Provider::create())
{
}

void
Transform::initialize()
{
	_targetAddedSlot = targetAdded()->connect(std::bind(
		&Transform::targetAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	));

	_targetRemovedSlot = targetRemoved()->connect(std::bind(
		&Transform::targetRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	));

	_data->set<Matrix4x4::Ptr>("transform.modelToWorldMatrix", _modelToWorld);
	//_data->set("transform/worldToModelMatrix", _worldToModel);
}

void
Transform::targetAddedHandler(AbstractComponent::Ptr	ctrl,
							  scene::Node::Ptr			target)
{
	if (targets().size() > 1)
		throw std::logic_error("Transform cannot have more than one target.");
	if (target->component<Transform>(1) != nullptr)
		throw std::logic_error("A node cannot have more than one Transform.");

	target->data()->addProvider(_data);

	auto callback = std::bind(
		&Transform::addedOrRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	);

	_addedSlot = target->added()->connect(callback);
	_removedSlot = target->removed()->connect(callback);

	addedOrRemovedHandler(nullptr, target, target->parent());
}

void
Transform::addedOrRemovedHandler(scene::Node::Ptr node,
								 scene::Node::Ptr target,
								 scene::Node::Ptr parent)
{
	if (target == targets()[0] && !target->root()->component<RootTransform>()
		&& (target != target->root() || target->children().size() != 0))
		target->root()->addComponent(RootTransform::create());
}

void
Transform::targetRemovedHandler(AbstractComponent::Ptr ctrl,
								scene::Node::Ptr 		target)
{
	target->data()->removeProvider(_data);

	_addedSlot = nullptr;
	_removedSlot = nullptr;
}

void
Transform::RootTransform::initialize()
{
	_targetSlots.push_back(targetAdded()->connect(std::bind(
		&Transform::RootTransform::targetAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	)));

	_targetSlots.push_back(targetRemoved()->connect(std::bind(
		&Transform::RootTransform::targetRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	)));
}

void
Transform::RootTransform::targetAddedHandler(AbstractComponent::Ptr 	ctrl,
											 scene::Node::Ptr			target)
{
	_targetSlots.push_back(target->added()->connect(std::bind(
		&Transform::RootTransform::addedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	)));
	_targetSlots.push_back(target->removed()->connect(std::bind(
		&Transform::RootTransform::removedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	)));
	_targetSlots.push_back(target->componentAdded()->connect(std::bind(
		&Transform::RootTransform::componentAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	)));
	_targetSlots.push_back(target->componentRemoved()->connect(std::bind(
		&Transform::RootTransform::componentRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	)));

	addedHandler(nullptr, target, target->parent());
}

void
Transform::RootTransform::targetRemovedHandler(AbstractComponent::Ptr 	ctrl,
											   scene::Node::Ptr			target)
{
	_targetSlots.clear();
	_enterFrameSlots.clear();
}

void
Transform::RootTransform::componentAddedHandler(scene::Node::Ptr			node,
												 scene::Node::Ptr 			target,
												 AbstractComponent::Ptr	ctrl)
{
	auto renderingCtrl = std::dynamic_pointer_cast<Rendering>(ctrl);

	if (renderingCtrl != nullptr)
		_enterFrameSlots[renderingCtrl] = renderingCtrl->enterFrame()->connect(std::bind(
			&Transform::RootTransform::enterFrameHandler,
			shared_from_this(),
			std::placeholders::_1
		));
	else if (std::dynamic_pointer_cast<Transform>(ctrl) != nullptr)
		_invalidLists = true;
}

void
Transform::RootTransform::componentRemovedHandler(scene::Node::Ptr			node,
												   scene::Node::Ptr 		target,
												   AbstractComponent::Ptr	ctrl)
{
	auto renderingCtrl = std::dynamic_pointer_cast<Rendering>(ctrl);

	if (renderingCtrl != nullptr)
		_enterFrameSlots.erase(renderingCtrl);
	else if (std::dynamic_pointer_cast<Transform>(ctrl) != nullptr)
		_invalidLists = true;
}

void
Transform::RootTransform::addedHandler(scene::Node::Ptr node,
									   scene::Node::Ptr target,
									   scene::Node::Ptr parent)
{
	auto enterFrameCallback = std::bind(
		&Transform::RootTransform::enterFrameHandler,
		shared_from_this(),
		std::placeholders::_1
	);

	auto descendants = scene::NodeSet::create(target)->descendants(true);
	for (auto descendant : descendants->nodes())
	{
		auto rootTransformCtrl = descendant->component<RootTransform>();

		if (rootTransformCtrl && rootTransformCtrl != shared_from_this())
			descendant->removeComponent(rootTransformCtrl);

		for (auto renderingCtrl : descendant->components<Rendering>())
			_enterFrameSlots[renderingCtrl] = renderingCtrl->enterFrame()->connect(enterFrameCallback);
	}

	_invalidLists = true;
}

void
Transform::RootTransform::removedHandler(scene::Node::Ptr node,
									     scene::Node::Ptr target,
										 scene::Node::Ptr parent)
{
	auto descendants = scene::NodeSet::create(target)->descendants(true);

	for (auto descendant : descendants->nodes())
		for (auto renderingCtrl : descendant->components<Rendering>())
			_enterFrameSlots.erase(renderingCtrl);

	_invalidLists = true;
}

void
Transform::RootTransform::updateTransformsList()
{
	unsigned int nodeId = 0;

	_idToNode.clear();
	_transform.clear();
	_modelToWorld.clear();
	_numChildren.clear();
	_firstChildId.clear();

	auto descendants = scene::NodeSet::create(targets())
		->descendants(true, false)
		->where([](scene::Node::Ptr node)
		{
			return node->hasComponent<Transform>();
		});

	for (auto node : descendants->nodes())
	{
		auto transformCtrl  = node->component<Transform>();

		_nodeToId[node] = nodeId;

		_idToNode.push_back(node);
		_transform.push_back(transformCtrl->_transform);
		_modelToWorld.push_back(transformCtrl->_modelToWorld);
		_numChildren.push_back(0);
		_firstChildId.push_back(0);

		auto ancestor = node->parent();
		while (ancestor != nullptr && _nodeToId.count(ancestor) == 0)
			ancestor = ancestor->parent();

		if (ancestor != nullptr)
		{
			auto ancestorId = _nodeToId[ancestor];

			_parentId.push_back(ancestorId);
			if (_numChildren[ancestorId] == 0)
				_firstChildId[ancestorId] = nodeId;
			_numChildren[ancestorId]++;
		}
		else
			_parentId.push_back(-1);

		++nodeId;
	}

	_invalidLists = false;
}

void
Transform::RootTransform::updateTransforms()
{
	unsigned int numNodes 	= _transform.size();
	unsigned int nodeId 	= 0;

	while (nodeId < numNodes)
	{
		auto parentModelToWorldMatrix 	= _modelToWorld[nodeId];
		auto numChildren 				= _numChildren[nodeId];
		auto firstChildId 				= _firstChildId[nodeId];
		auto lastChildId 				= firstChildId + numChildren;
		auto parentId 					= _parentId[nodeId];
        auto parentTransformChanged     = parentModelToWorldMatrix->_hasChanged;

        parentModelToWorldMatrix->_hasChanged = false;

		if (parentId == -1)
		{
            auto parentTransform = _transform[nodeId];

            if (parentTransform->_hasChanged)
            {
                parentTransformChanged = true;

			    parentModelToWorldMatrix->copyFrom(parentTransform);
                parentTransform->_hasChanged = false;
            }
		}

		for (auto childId = firstChildId; childId < lastChildId; ++childId)
		{
			auto modelToWorld   = _modelToWorld[childId];
            auto transform      = _transform[childId];

            if (transform->_hasChanged || parentTransformChanged)
            {
			    modelToWorld->copyFrom(transform)->append(parentModelToWorldMatrix);
			    transform->_hasChanged = false;
            }
		}

		++nodeId;
	}
}

void
Transform::RootTransform::forceUpdate(scene::Node::Ptr node)
{
	if (_invalidLists)
		updateTransformsList();

	auto				targetNodeId	= _nodeToId[node];
	int					nodeId			= targetNodeId;
	auto				dirtyRoot		= -1;
	std::vector<uint>	path;

	// find the "dirty" root and build the path to get back to the target node
	while (nodeId >= 0)
	{
		if ((_transform[nodeId]->_hasChanged)
			|| (nodeId != targetNodeId && _modelToWorld[nodeId]->_hasChanged))
			dirtyRoot = nodeId;

		path.push_back(nodeId);

		nodeId = _parentId[nodeId];
	}

	// update that path starting from the dirty root
	for (int i = path.size() - 1; i >= 0; --i)
	{
		auto dirtyNodeId	= path[i];
		auto parentId		= _parentId[dirtyNodeId];
		auto modelToWorld	= _modelToWorld[dirtyNodeId];

		modelToWorld->copyFrom(_transform[dirtyNodeId]);
		if (parentId != -1)
			modelToWorld->append(_modelToWorld[parentId]);
		modelToWorld->_hasChanged = false;
	}
}

void
Transform::RootTransform::enterFrameHandler(std::shared_ptr<Rendering> ctrl)
{
	if (_invalidLists)
		updateTransformsList();

	updateTransforms();
}
