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

#include "AbstractDiscreteLight.hpp"

#include "minko/math/Matrix4x4.hpp"
#include "minko/scene/Node.hpp"
#include "minko/data/Container.hpp"

using namespace minko;
using namespace minko::component;

AbstractDiscreteLight::AbstractDiscreteLight(const std::string& arrayName, uint lightId) :
	AbstractLight(arrayName, lightId)
{
}

void
AbstractDiscreteLight::targetAddedHandler(AbstractComponent::Ptr cmp, std::shared_ptr<scene::Node> target)
{
	AbstractRootDataComponent::targetAddedHandler(cmp, target);

	_modelToWorldChangedSlot = target->data()->propertyChanged("transform.modelToWorldMatrix")
		->connect(std::bind(
			&AbstractDiscreteLight::modelToWorldMatrixChangedHandler,
			std::dynamic_pointer_cast<AbstractDiscreteLight>(shared_from_this()),
			std::placeholders::_1,
			std::placeholders::_2
		));
}

void
AbstractDiscreteLight::targetRemovedHandler(AbstractComponent::Ptr cmp, std::shared_ptr<scene::Node> target)
{
	AbstractRootDataComponent::targetRemovedHandler(cmp, target);

	_modelToWorldChangedSlot = nullptr;
}

void
AbstractDiscreteLight::modelToWorldMatrixChangedHandler(std::shared_ptr<data::Container> 	container,
								 						const std::string& 					propertyName)
{
	updateModelToWorldMatrix(container->get<math::Matrix4x4::Ptr>(propertyName));
}
