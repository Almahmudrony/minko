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

#include "BoundingBox.hpp"

#include "minko/math/Box.hpp"
#include "minko/math/Vector3.hpp"
#include "minko/scene/Node.hpp"
#include "minko/component/Transform.hpp"
#include "minko/component/Surface.hpp"
#include "minko/geometry/Geometry.hpp"
#include "minko/data/Container.hpp"

using namespace minko;
using namespace math;
using namespace minko::component;

BoundingBox::BoundingBox(Vector3::Ptr topRight, Vector3::Ptr bottomLeft) :
	_fixed(true),
	_box(math::Box::create(topRight, bottomLeft)),
	_worldSpaceBox(math::Box::create(topRight, bottomLeft))
{

}

BoundingBox::BoundingBox() :
	_fixed(false),
	_box(math::Box::create()),
	_worldSpaceBox(math::Box::create())
{

}

void
BoundingBox::initialize()
{
	_targetAddedSlot = targetAdded()->connect([&](AbstractComponent::Ptr cmp, scene::Node::Ptr target)
	{
		if (targets().size() > 1)
			throw std::logic_error("The same BoundingBox cannot have 2 different targets");

		_modelToWorldChangedSlot = target->data()->propertyValueChanged("transform.modelToWorldMatrix")->connect(
			[&](data::Container::Ptr data, const std::string& propertyName)
			{
				updateWorldSpaceBox();
			}
		);

		auto componentAddedOrRemovedCallback = [&](scene::Node::Ptr node, scene::Node::Ptr target, AbstractComponent::Ptr cmp)
		{
			if (std::dynamic_pointer_cast<Surface>(cmp))
				update();
		};

		_componentAddedSlot = target->componentAdded()->connect(componentAddedOrRemovedCallback);
		_componentRemovedSlot = target->componentAdded()->connect(componentAddedOrRemovedCallback);

		update();
	});

	_targetRemovedSlot = targetRemoved()->connect([&](AbstractComponent::Ptr cmp, scene::Node::Ptr target)
	{
		_componentAddedSlot = nullptr;
		_componentRemovedSlot = nullptr;
	});
}

void
BoundingBox::update()
{
	auto target = targets()[0];
	auto surfaces = target->components<Surface>();

	if (!_fixed)
	{
		if (!surfaces.empty())
		{
			auto min = Vector3::create(
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max(),
				std::numeric_limits<float>::max()
			);
			auto max = Vector3::create(
				std::numeric_limits<float>::lowest(),
				std::numeric_limits<float>::lowest(),
				std::numeric_limits<float>::lowest()
			);

			for (auto& surface : surfaces)
			{
				auto geom = surface->geometry();
				auto xyzBuffer = geom->vertexBuffer("position");
				auto attr = xyzBuffer->attribute("position");
				auto offset = std::get<2>(*attr);

				for (uint i = 0; i < xyzBuffer->numVertices(); ++i)
				{
					auto x = xyzBuffer->data()[i * xyzBuffer->vertexSize() + offset];
					auto y = xyzBuffer->data()[i * xyzBuffer->vertexSize() + offset + 1];
					auto z = xyzBuffer->data()[i * xyzBuffer->vertexSize() + offset + 2];

					if (x < min->x())
						min->x(x);
					else if (x > max->x())
						max->x(x);

					if (y < min->y())
						min->y(y);
					else if (y > max->y())
						max->y(y);

					if (z < min->z())
						min->z(z);
					else if (z > max->z())
						max->z(z);
				}
			}

			_box->bottomLeft()->copyFrom(min);
			_box->topRight()->copyFrom(max);
		}
		else
		{
			_box->bottomLeft()->copyFrom(Vector3::zero());
			_box->topRight()->copyFrom(Vector3::zero());
		}
	}

	updateWorldSpaceBox();
}

void
BoundingBox::updateWorldSpaceBox()
{
	if (!targets()[0]->data()->hasProperty("transform.modelToWorldMatrix"))
	{
		_worldSpaceBox->topRight()->copyFrom(_box->topRight());
		_worldSpaceBox->bottomLeft()->copyFrom(_box->bottomLeft());
	}
	else
	{
		auto t = targets()[0]->data()->get<Matrix4x4::Ptr>("transform.modelToWorldMatrix");
		auto vertices = _box->getVertices();

		for (auto& vertex : vertices)
			t->transform(vertex, vertex);

		auto min = _worldSpaceBox->bottomLeft()->setTo(
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max(),
			std::numeric_limits<float>::max()
		);
		auto max = _worldSpaceBox->topRight()->setTo(
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest(),
			std::numeric_limits<float>::lowest()
		);

		for (auto& vertex : vertices)
		{
			if (vertex->x() > max->x())
				max->x(vertex->x());
			else if (vertex->x() < min->x())
				min->x(vertex->x());

			if (vertex->y() > max->y())
				max->y(vertex->y());
			else if (vertex->y() < min->y())
				min->y(vertex->y());

			if (vertex->z() > max->z())
				max->z(vertex->z());
			else if (vertex->z() < min->z())
				min->z(vertex->z());
		}
	}
}
