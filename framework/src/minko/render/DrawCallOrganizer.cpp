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

#include "minko/render/DrawCallOrganizer.hpp"
#include "minko/render/DrawCall.hpp"
#include "minko/component/Renderer.hpp"
#include "minko/component/Surface.hpp"
#include "minko/render/Effect.hpp"
#include "minko/render/Program.hpp"
#include "minko/data/Container.hpp"
#include "minko/scene/Node.hpp"

using namespace minko;
using namespace minko::render;

const unsigned int DrawCallOrganizer::NUM_FALLBACK_ATTEMPTS = 32;

DrawCallOrganizer::DrawCallOrganizer(RendererPtr renderer):
	_renderer(renderer)
{
}

std::list<std::shared_ptr<DrawCall>>
DrawCallOrganizer::drawCalls()
{
	unsigned int _numToCollect				= _toCollect.size();
	
	if (_numToCollect == 0)
		return _drawCalls;
	
	unsigned int _lastFrameDrawCallsSize 	= _drawCalls.size();

	for (uint surfaceIndex = 0; surfaceIndex < _numToCollect; ++surfaceIndex)
	{
		
		auto& newDrawCalls = generateDrawCall(_toCollect[surfaceIndex], NUM_FALLBACK_ATTEMPTS);
		_drawCalls.insert(_drawCalls.end(), newDrawCalls.begin(), newDrawCalls.end());
	}
	
	_drawCalls.sort(&DrawCallOrganizer::compareDrawCalls);

	_toCollect.erase(_toCollect.begin(), _toCollect.begin() + _numToCollect);

	return _drawCalls;
}


bool
DrawCallOrganizer::compareDrawCalls(DrawCallPtr& a, DrawCallPtr& b)
{
	if (a->priority() == b->priority())
		return a->target() && (!b->target() || (a->target()->id() > b->target()->id()));

    return a->priority() > b->priority();
}

void
DrawCallOrganizer::addSurface(SurfacePtr surface)
{
	_surfaceToTechniqueChangedSlot[surface] = surface->techniqueChanged()->connect(std::bind(
		&DrawCallOrganizer::techniqueChanged,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3));

	_techniqueToMacroNames[surface].clear();

	Effect::Ptr effect = (_renderer->effect() ? _renderer->effect() : surface->effect());

	for (auto& technique : effect->techniques())
	{
		auto& techniqueName = technique.first;

		for (auto& pass : technique.second)
			for (auto& macroBinding : pass->macroBindings())
				_techniqueToMacroNames[surface][techniqueName].insert(std::get<0>(macroBinding.second));
	}
	_toCollect.push_back(surface);
}

void
DrawCallOrganizer::removeSurface(SurfacePtr surface)
{
	deleteDrawCalls(surface);

	_surfaceToTechniqueChangedSlot.erase(surface);
	_surfaceToDrawCalls[surface].clear();
	_macroAddedOrRemovedSlots[surface].clear();
	_macroChangedSlots[surface].clear();
	_numMacroListeners[surface].clear();
}

void
DrawCallOrganizer::techniqueChanged(SurfacePtr surface, const std::string& technique, bool updateDrawCall)
{
	removeSurface(surface);

	if (updateDrawCall)
		_toCollect.push_back(surface);
}

DrawCallOrganizer::DrawCallList
DrawCallOrganizer::generateDrawCall(SurfacePtr	surface,
								  unsigned int	numAttempts)
{
	if (_surfaceToDrawCalls.find(surface) != _surfaceToDrawCalls.end())
		deleteDrawCalls(surface);

	bool							mustFallbackTechnique	= false;
	std::shared_ptr<render::Effect> drawCallEffect			= surface->effect();
	std::string						technique				= surface->technique();

	if (_renderer->effect())
	{
		drawCallEffect	= _renderer->effect();
		technique		= _renderer->effect()->techniques().begin()->first;
	}

	_surfaceToDrawCalls[surface] = std::list<DrawCall::Ptr>();

	for (const auto& pass : drawCallEffect->technique(technique))
	{
		auto drawCall = initializeDrawCall(pass, surface);

		if (drawCall)
		{
			_surfaceToDrawCalls[surface].push_back(drawCall);
			//_drawCallAdded->execute(shared_from_this(), surface, drawCall);
		}
		else
		{
			// one pass failed without any viable fallback, fallback the whole technique then.
			mustFallbackTechnique = true;
			break;
		}
	}

	if (mustFallbackTechnique)
	{
		_surfaceToDrawCalls[surface].clear();

		if (numAttempts > 0 && drawCallEffect->hasFallback(technique))
		{
			surface->setTechnique(drawCallEffect->fallback(technique), false);

			return generateDrawCall(surface, numAttempts - 1);
		}
	}
	else
		watchMacroAdditionOrDeletion(surface);

	return _surfaceToDrawCalls[surface];
}

std::shared_ptr<DrawCall>
DrawCallOrganizer::initializeDrawCall(std::shared_ptr<render::Pass>		pass, 
									std::shared_ptr<component::Surface>	surface,
									std::shared_ptr<DrawCall>			drawcall)
{
#ifdef DEBUG
	if (pass == nullptr)
		throw std::invalid_argument("pass");
#endif // DEBUG

	const auto rendererData = _renderer->targets()[0]->data();
	const float	priority	= drawcall ? drawcall->priority() : pass->states()->priority();
	const auto	target		= surface->targets()[0];
	const auto	targetData	= target->data();
	const auto	rootData	= target->root()->data();

	std::list<data::ContainerProperty>	booleanMacros;
	std::list<data::ContainerProperty>	integerMacros;
	std::list<data::ContainerProperty>	incorrectIntegerMacros;

	auto program = getWorkingProgram(
		surface,
		pass, 
		targetData, 
		rendererData, 
		rootData, 
		booleanMacros, 
		integerMacros,
		incorrectIntegerMacros
	);

	if (!program)
		return nullptr;
	
	if (drawcall == nullptr)
	{
		drawcall = DrawCall::create(
			pass->attributeBindings(),
			pass->uniformBindings(),
			pass->stateBindings(),
			pass->states()
		);

		_drawCallToPass[surface][drawcall]			= pass;
		_drawCallToRendererData[surface][drawcall]	= rendererData;

		for (const auto& binding : pass->macroBindings())
		{
			data::ContainerProperty macro(binding.second, targetData, rendererData, rootData);

			_macroNameToDrawCalls[surface][macro.name()].push_back(drawcall);

			if (macro.container())
			{
				auto&		listeners		= _numMacroListeners;
				const int	numListeners	= listeners[surface].count(macro) == 0 ? 0 : listeners[surface][macro];

				if (numListeners == 0)
					macroChangedHandler(macro.container(), macro.name(), surface, MacroChange::ADDED);
			}
		}
	}

	drawcall->configure(program, targetData, rendererData, rootData);
	drawcall->priority(priority);

	return drawcall;
}

std::shared_ptr<Program>
DrawCallOrganizer::getWorkingProgram(std::shared_ptr<component::Surface>	surface,
								   std::shared_ptr<Pass>				pass,
								   std::shared_ptr<data::Container>		targetData,
								   std::shared_ptr<data::Container>		rendererData,
								   std::shared_ptr<data::Container>		rootData,
								   std::list<data::ContainerProperty>&	booleanMacros,
								   std::list<data::ContainerProperty>&	integerMacros,
								   std::list<data::ContainerProperty>&	incorrectIntegerMacros)
{
	Program::Ptr program = nullptr;

	do
	{
		program = pass->selectProgram(
			targetData, 
			rendererData, 
			rootData, 
			booleanMacros, 
			integerMacros, 
			incorrectIntegerMacros
		);

#ifdef DEBUG_FALLBACK
	assert(incorrectIntegerMacros.empty() != (program==nullptr));
#endif // DEBUG_FALLBACK

		forgiveMacros	(surface, booleanMacros, integerMacros,	TechniquePass(surface->technique(), pass));
		blameMacros		(surface, incorrectIntegerMacros,		TechniquePass(surface->technique(), pass));

		break;

		/*
		if (program)
			break;
		else
		{
#ifdef DEBUG_FALLBACK
			std::cout << "fallback:\tpass '" << pass->name() << "'\t-> pass '" << pass->fallback() << "'" << std::endl;
#endif // DEBUG_FALLBACK

			const std::vector<Pass::Ptr>& passes = _effect->technique(_technique);
			auto fallbackIt = std::find_if(passes.begin(), passes.end(), [&](const Pass::Ptr& p)
			{
				return p->name() == pass->fallback();
			});

			if (fallbackIt == passes.end())
				break;
			else
				pass = *fallbackIt;
		}
		*/
	}
	while(true);

	return program;
}

void
DrawCallOrganizer::deleteDrawCalls(SurfacePtr surface)
{
	auto& drawCalls = _surfaceToDrawCalls[surface];

	while (!drawCalls.empty())
	{
		auto drawCall = drawCalls.front();

		//_drawCallRemoved->execute(shared_from_this(), surface, drawCall);
		drawCalls.pop_front();

		_drawCallToPass[surface].erase(drawCall);
		_drawCallToRendererData[surface].erase(drawCall);

		for (auto& drawcallsIt : _macroNameToDrawCalls[surface])
		{
			auto& macroName			= drawcallsIt.first;
			auto& macroDrawcalls	= drawcallsIt.second;
			auto  drawcallIt		= std::find(macroDrawcalls.begin(), macroDrawcalls.end(), drawCall);
			
			if (drawcallIt != macroDrawcalls.end())
				macroDrawcalls.erase(drawcallIt);
		}

		_drawCalls.remove(drawCall);
	}

	// erase in a subsequent step the entries corresponding to macro names which do not monitor any drawcall anymore.
	for (auto drawcallsIt = _macroNameToDrawCalls[surface].begin();
		drawcallsIt != _macroNameToDrawCalls[surface].end();
		)
		if (drawcallsIt->second.empty())
			drawcallsIt = _macroNameToDrawCalls[surface].erase(drawcallsIt);
		else
			++drawcallsIt;
}

void
DrawCallOrganizer::macroChangedHandler(ContainerPtr		container, 
									 const std::string& propertyName, 
									 SurfacePtr			surface, 
									 MacroChange		change)
{
	const data::ContainerProperty	macro(propertyName, container);
	Effect::Ptr						effect = (_renderer->effect() ? _renderer->effect() : surface->effect());


	if (change == MacroChange::REF_CHANGED && !_surfaceToDrawCalls[surface].empty())
	{
		const DrawCallList							drawCalls		= _macroNameToDrawCalls[surface][macro.name()];
		std::unordered_set<data::Container::Ptr>	failedDrawcallRendererData;

		for (std::shared_ptr<DrawCall> drawCall : drawCalls)
		{
			auto	rendererData	= _drawCallToRendererData[surface][drawCall];
			auto	pass			= _drawCallToPass[surface][drawCall];
	
			if (!initializeDrawCall(pass, surface, drawCall))
				failedDrawcallRendererData.insert(rendererData);
		}

		if (!failedDrawcallRendererData.empty())
		{
			// at least, one pass failed for good. must fallback the whole technique.
			//for (auto& rendererData : failedDrawcallRendererData)
			//	if (_drawCallToRendererData[surface].count(rendererData) > 0)
			//		deleteDrawCalls(surface);

			if (effect->hasFallback(surface->technique()))
				surface->setTechnique(effect->fallback(surface->technique()), true);
		}
	}
	else if (_techniqueToMacroNames[surface].count(surface->technique()) != 0 
		&&   _techniqueToMacroNames[surface][surface->technique()].find(macro.name()) != _techniqueToMacroNames[surface][surface->technique()].end())
	{
		int numListeners = _numMacroListeners[surface].count(macro) == 0 ? 0 : _numMacroListeners[surface][macro];

		if (change == MacroChange::ADDED)
		{
			if (numListeners == 0)
				_macroChangedSlots[surface][macro] = macro.container()->propertyReferenceChanged(macro.name())->connect(std::bind(
					&DrawCallOrganizer::macroChangedHandler,
					shared_from_this(),
					macro.container(),
					macro.name(),
					surface,
					MacroChange::REF_CHANGED
				));

			_numMacroListeners[surface][macro] = numListeners + 1;
		}
		else if (change == MacroChange::REMOVED)
		{
			macroChangedHandler(macro.container(), macro.name(), surface, MacroChange::REF_CHANGED);

			_numMacroListeners[surface][macro] = numListeners - 1;

			if (_numMacroListeners[surface][macro] == 0)
				_macroChangedSlots[surface].erase(macro);
		}
	}
}


void
DrawCallOrganizer::watchMacroAdditionOrDeletion(std::shared_ptr<component::Surface> surface)
{
	_macroAddedOrRemovedSlots[surface].clear();

	if (surface->targets().empty())
		return;

	auto	rendererData	= _renderer->targets()[0]->data();
	auto&	target			= surface->targets().front();
	auto	targetData		= target->data();
	auto	rootData		= target->root()->data();

	_macroAddedOrRemovedSlots[surface].push_back(
		targetData->propertyAdded()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::ADDED
		))
	);

	_macroAddedOrRemovedSlots[surface].push_back(
		targetData->propertyRemoved()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::REMOVED
		))
	);

	_macroAddedOrRemovedSlots[surface].push_back(
		rendererData->propertyAdded()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::ADDED
		))
	);

	_macroAddedOrRemovedSlots[surface].push_back(
		rendererData->propertyRemoved()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::REMOVED
		))
	);

	_macroAddedOrRemovedSlots[surface].push_back(
		rootData->propertyAdded()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::ADDED
		))
	);

	_macroAddedOrRemovedSlots[surface].push_back(
		rootData->propertyRemoved()->connect(std::bind(
			&DrawCallOrganizer::macroChangedHandler,
			shared_from_this(),
			std::placeholders::_1,
			std::placeholders::_2,
			surface,
			MacroChange::REMOVED
		))
	);
}

void
DrawCallOrganizer::blameMacros(SurfacePtr									surface,
							 const std::list<data::ContainerProperty>&	incorrectIntegerMacros,
							 const TechniquePass&						pass)
{
	for (auto& macro : incorrectIntegerMacros)
	{
		auto&	failedPasses = _incorrectMacroToPasses[surface][macro];
		auto	failedPassIt = std::find(failedPasses.begin(), failedPasses.end(), pass);
	
		if (failedPassIt == failedPasses.end())
		{
			failedPasses.push_back(pass);
	
#ifdef DEBUG_FALLBACK
			for (auto& techniqueName : _incorrectMacroToPasses[macro])
				std::cout << "'" << macro.name() << "' made [technique '" << pass.first << "' | pass '" << pass.second.get() << "'] fail" << std::endl;
#endif // DEBUG_FALLBACK
		}
	
		if (_incorrectMacroChangedSlot[surface].count(macro) == 0)
		{
			_incorrectMacroChangedSlot[surface][macro] = macro.container()->propertyReferenceChanged(macro.name())->connect(std::bind(
				&DrawCallOrganizer::incorrectMacroChangedHandler,
				shared_from_this(),
				surface,
				macro
			));
		}
	}
}

void
DrawCallOrganizer::incorrectMacroChangedHandler(SurfacePtr					surface,
											  const data::ContainerProperty& macro)
{
	if (_incorrectMacroToPasses[surface].count(macro) > 0)
	{
		surface->setTechnique(_incorrectMacroToPasses[surface][macro].front().first, true); // FIXME
	}
}

void
DrawCallOrganizer::forgiveMacros(SurfacePtr											surface,
							   const std::list<data::ContainerProperty>&			booleanMacros,
							   const std::list<data::ContainerProperty>&			integerMacros,
							   const TechniquePass&									pass)
{
	for (auto& macro : integerMacros)
		if (_incorrectMacroToPasses[surface].count(macro) > 0)
		{
			auto&	failedPasses	= _incorrectMacroToPasses[surface][macro];
			auto	failedPassIt	= std::find(failedPasses.begin(), failedPasses.end(), pass);

			if (failedPassIt != failedPasses.end())
			{
				failedPasses.erase(failedPassIt);

				if (failedPasses.empty() 
					&& _incorrectMacroChangedSlot[surface].count(macro) > 0)
					_incorrectMacroChangedSlot[surface].erase(macro);
			}
		}
}
