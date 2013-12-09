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

#include "minko/component/Renderer.hpp"

#include "minko/scene/Node.hpp"
#include "minko/scene/NodeSet.hpp"
#include "minko/component/Surface.hpp"
#include "minko/render/DrawCall.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Pass.hpp"
#include "minko/render/Texture.hpp"
#include "minko/render/AbstractContext.hpp"
#include "minko/component/SceneManager.hpp"
#include "minko/file/AssetLibrary.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::scene;
using namespace minko::render;

Renderer::Renderer() :
    _backgroundColor(0),
	_renderingBegin(Signal<Ptr>::create()),
	_renderingEnd(Signal<Ptr>::create()),
	_surfaceDrawCalls(),
	_surfaceTechniqueChangedSlot()
{
}

void
Renderer::initialize()
{
	_targetAddedSlot = targetAdded()->connect(std::bind(
		&Renderer::targetAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	));	

	_targetRemovedSlot = targetRemoved()->connect(std::bind(
		&Renderer::targetRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	));
}

void
Renderer::targetAddedHandler(std::shared_ptr<AbstractComponent> ctrl,
							 std::shared_ptr<Node> 			target)
{
	if (target->components<Renderer>().size() > 1)
		throw std::logic_error("There cannot be two Renderer on the same node.");

	_addedSlot = target->added()->connect(std::bind(
		&Renderer::addedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_removedSlot = target->removed()->connect(std::bind(
		&Renderer::removedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	addedHandler(target->root(), target, target->parent());
}

void
Renderer::targetRemovedHandler(std::shared_ptr<AbstractComponent> 	ctrl,
							    std::shared_ptr<Node> 				target)
{
	_addedSlot = nullptr;
	_removedSlot = nullptr;

	removedHandler(target->root(), target, target->parent());
}

void
Renderer::addedHandler(std::shared_ptr<Node> node,
						std::shared_ptr<Node> target,
						std::shared_ptr<Node> parent)
{
	findSceneManager();

	_rootDescendantAddedSlot = target->root()->added()->connect(std::bind(
		&Renderer::rootDescendantAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_rootDescendantRemovedSlot = target->root()->removed()->connect(std::bind(
		&Renderer::rootDescendantRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_componentAddedSlot = target->root()->componentAdded()->connect(std::bind(
		&Renderer::componentAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_componentRemovedSlot = target->root()->componentRemoved()->connect(std::bind(
		&Renderer::componentRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	rootDescendantAddedHandler(nullptr, target->root(), nullptr);
}

void
Renderer::removedHandler(std::shared_ptr<Node> node,
						  std::shared_ptr<Node> target,
						  std::shared_ptr<Node> parent)
{
	findSceneManager();

	_rootDescendantAddedSlot = nullptr;
	_rootDescendantRemovedSlot = nullptr;
	_componentAddedSlot = nullptr;
	_componentRemovedSlot = nullptr;

	rootDescendantRemovedHandler(nullptr, target->root(), nullptr);
}

void
Renderer::rootDescendantAddedHandler(std::shared_ptr<Node> node,
									  std::shared_ptr<Node> target,
									  std::shared_ptr<Node> parent)
{
    auto surfaceNodes = NodeSet::create(target)
		->descendants(true)
        ->where([](scene::Node::Ptr node)
        {
            return node->hasComponent<Surface>();
        });

	for (auto surfaceNode : surfaceNodes->nodes())
		for (auto surface : surfaceNode->components<Surface>())
			addSurface(surface);
}

void
Renderer::rootDescendantRemovedHandler(std::shared_ptr<Node> node,
									    std::shared_ptr<Node> target,
									    std::shared_ptr<Node> parent)
{
	auto surfaceNodes = NodeSet::create(target)
		->descendants(true)
        ->where([](scene::Node::Ptr node)
        {
            return node->hasComponent<Surface>();
        });

	for (auto surfaceNode : surfaceNodes->nodes())
		for (auto surface : surfaceNode->components<Surface>())
			removeSurface(surface);
}

void
Renderer::componentAddedHandler(std::shared_ptr<Node>				node,
								 std::shared_ptr<Node>				target,
								 std::shared_ptr<AbstractComponent>	ctrl)
{
	auto surfaceCtrl = std::dynamic_pointer_cast<Surface>(ctrl);
	auto sceneManager = std::dynamic_pointer_cast<SceneManager>(ctrl);
	
	if (surfaceCtrl)
		addSurface(surfaceCtrl);
	else if (sceneManager)
		setSceneManager(sceneManager);
}

void
Renderer::componentRemovedHandler(std::shared_ptr<Node>					node,
								  std::shared_ptr<Node>					target,
								  std::shared_ptr<AbstractComponent>	ctrl)
{
	auto surfaceCtrl = std::dynamic_pointer_cast<Surface>(ctrl);
	auto sceneManager = std::dynamic_pointer_cast<SceneManager>(ctrl);

	if (surfaceCtrl)
		removeSurface(surfaceCtrl);
	else if (sceneManager)
		setSceneManager(nullptr);
}

void
Renderer::addSurface(Surface::Ptr surface)
{
	_surfaceTechniqueChangedSlot[surface]	= surface->techniqueChanged()->connect(std::bind(
		&Renderer::surfaceTechniqueChanged,
		shared_from_this(),
		surface,
		std::placeholders::_2
	));

	_toCollect.insert(surface);
}

void
Renderer::addSurfaceDrawcalls(Surface::Ptr surface)
{
	removeSurfaceDrawcalls(surface);

	auto& drawcalls = surface->createDrawCalls(targets()[0]->data());

	_surfaceDrawCalls[surface]	= drawcalls;

	if (!drawcalls.empty())
		_drawCalls.insert(
			_drawCalls.end(), 
			_surfaceDrawCalls[surface].begin(), 
			_surfaceDrawCalls[surface].end()
		);
}

void
Renderer::removeSurface(Surface::Ptr surface)
{
	removeSurfaceDrawcalls(surface);

	_surfaceTechniqueChangedSlot.erase(surface);

	if (_surfaceDrawCalls.count(surface) > 0)
		_surfaceDrawCalls.erase(surface);
}

void
Renderer::removeSurfaceDrawcalls(Surface::Ptr surface)
{
	auto foundDrawcallsIt	= _surfaceDrawCalls.find(surface);

	if (foundDrawcallsIt == _surfaceDrawCalls.end())
		return;

	const DrawCallList& surfaceDrawcalls = foundDrawcallsIt->second;
	if (surfaceDrawcalls.empty())
		return;

	for (DrawCallList::iterator it = _drawCalls.begin(); it != _drawCalls.end(); )
	{
		auto foundSurfaceDrawcallIt = std::find(surfaceDrawcalls.begin(), surfaceDrawcalls.end(), *it);

		if (foundSurfaceDrawcallIt != surfaceDrawcalls.end())
			it = _drawCalls.erase(it);
		else
			++it;
	}

	_surfaceDrawCalls.erase(surface);
}

void
Renderer::render(std::shared_ptr<render::AbstractContext> context, std::shared_ptr<render::Texture> renderTarget)
{
	std::set<Surface::Ptr> toCollect;
	_toCollect.swap(toCollect); // _toCollect now empty

	for (auto surface : toCollect)
		addSurfaceDrawcalls(surface); // warning: may add drawcalls in the _toCollect structure, hence the separate toCollect.

	_renderingBegin->execute(shared_from_this());

	if (!renderTarget)
		renderTarget = _renderTarget;

	context->clear(
		((_backgroundColor >> 24) & 0xff) / 255.f,
		((_backgroundColor >> 16) & 0xff) / 255.f,
		((_backgroundColor >> 8) & 0xff) / 255.f,
		(_backgroundColor & 0xff) / 255.f
	);

    _drawCalls.sort(&Renderer::compareDrawCalls);
	for (auto& drawCall : _drawCalls)
		drawCall->render(context, renderTarget);

	context->present();

	_renderingEnd->execute(shared_from_this());
}

bool
Renderer::compareDrawCalls(DrawCallPtr& a, DrawCallPtr& b)
{
	if (a->priority() == b->priority())
		return a->target() && (!b->target() || (a->target()->id() > b->target()->id()));

    return a->priority() < b->priority();
}

void
Renderer::findSceneManager()
{
	NodeSet::Ptr roots = NodeSet::create(targets())
		->roots()
		->where([](NodePtr node)
		{
			return node->hasComponent<SceneManager>();
		});

	if (roots->nodes().size() > 1)
		throw std::logic_error("Renderer cannot be in two separate scenes.");
	else if (roots->nodes().size() == 1)
		setSceneManager(roots->nodes()[0]->component<SceneManager>());		
	else
		setSceneManager(nullptr);
}

void
Renderer::setSceneManager(std::shared_ptr<SceneManager> sceneManager)
{
	if (sceneManager != _sceneManager)
	{
		if (sceneManager)
		{
			_sceneManager = sceneManager;
			_renderingBeginSlot = _sceneManager->renderingBegin()->connect(std::bind(
				&Renderer::sceneManagerRenderingBeginHandler,
				shared_from_this(),
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3
			));
		}
		else
		{
			_sceneManager = nullptr;
			_renderingBeginSlot = nullptr;
		}
	}
}

void
Renderer::sceneManagerRenderingBeginHandler(std::shared_ptr<SceneManager>		sceneManager,
										    uint								frameId,
										    std::shared_ptr<render::Texture>	renderTarget)
{
	render(sceneManager->assets()->context(), renderTarget);
}

void
Renderer::surfaceTechniqueChanged(Surface::Ptr			surface, 
								  const std::string&	technique)
{
	removeSurfaceDrawcalls(surface);
	_toCollect.insert(surface);
}