/*,
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

#include "minko/render/DrawCallPool.hpp"
#include "minko/render/DrawCallZSorter.hpp"

using namespace minko;
using namespace minko::render;

DrawCallPool::DrawCallIteratorPair
DrawCallPool::addDrawCalls(Effect::Ptr                                          effect,
                           const std::unordered_map<std::string, std::string>&  variables,
                           const std::string&                                   techniqueName,
                           data::Store&                                         rootData,
                           data::Store&                                         rendererData,
                           data::Store&                                         targetData)
{
    const auto& technique = effect->technique(techniqueName);
    
    for (const auto& pass : technique)
    {
        _drawCalls.emplace_back(pass, variables, rootData, rendererData, targetData);
        initializeDrawCall(_drawCalls.back());
    }

    return DrawCallIteratorPair(std::prev(_drawCalls.end(), technique.size()), std::prev(_drawCalls.end()));
}

void
DrawCallPool::removeDrawCalls(const DrawCallIteratorPair& iterators)
{
    auto end = std::next(iterators.second);

    for (auto it = iterators.first; it != end; ++it)
    {
        DrawCall& drawCall = *it;

        // FIXME: avoid const_cast
        unwatchProgramSignature(
            drawCall,
            drawCall.pass()->macroBindings(),
            drawCall.rootData(),
            drawCall.rendererData(),
            drawCall.targetData()
        );

        _invalidDrawCalls.erase(&drawCall);
    }

    _drawCalls.erase(iterators.first, end);
}

void
DrawCallPool::watchProgramSignature(DrawCall&                       drawCall,
                                    const data::MacroBindingMap&    macroBindings,
                                    data::Store&                    rootData,
                                    data::Store&                    rendererData,
                                    data::Store&                    targetData)
{
    for (const auto& macroNameAndBinding : macroBindings.bindings)
    {
        const auto& macroBinding = macroNameAndBinding.second;
        auto& store = macroBinding.source == data::Binding::Source::ROOT ? rootData
            : macroBinding.source == data::Binding::Source::RENDERER ? rendererData
            : targetData;
        auto bindingKey = MacroBindingKey(&macroBinding, &store);
        auto& drawCalls = _macroToDrawCalls[bindingKey];

        drawCalls.push_back(&drawCall);

        if (macroBinding.type != data::MacroBinding::Type::UNSET)
        {
            addMacroCallback(
                store.propertyChanged(),
                store,
                std::bind(&DrawCallPool::macroPropertyChangedHandler, this, std::ref(drawCalls))
            );
        }
        else
        {
            auto propertyName = Store::getActualPropertyName(drawCall.variables(), macroBinding.propertyName);
            
            if (store.hasProperty(propertyName))
            {
                addMacroCallback(
                    store.propertyRemoved(),
                    store,
                    std::bind(&DrawCallPool::macroPropertyRemovedHandler, this, std::ref(store), std::ref(drawCalls))
                );
            }
            else
            {
                addMacroCallback(
                    store.propertyAdded(),
                    store,
                    std::bind(&DrawCallPool::macroPropertyAddedHandler, this, std::ref(store), std::ref(drawCalls))
                );
            }

        }
    }
}

void
DrawCallPool::addMacroCallback(PropertyChanged&     key,
                               data::Store&         store,
                               const MacroCallback& callback)
{
    if (_macroChangedSlot.count(&key) == 0)
        _macroChangedSlot.emplace(&key, ChangedSlot(key.connect(callback), 1));
    else
        _macroChangedSlot[&key].second++;
}

void
DrawCallPool::removeMacroCallback(PropertyChanged& key)
{
    _macroChangedSlot[&key].second--;
    if (_macroChangedSlot[&key].second == 0)
        _macroChangedSlot.erase(&key);
}

void
DrawCallPool::macroPropertyChangedHandler(const std::list<DrawCall*>& drawCalls)
{
    _invalidDrawCalls.insert(drawCalls.begin(), drawCalls.end());
}

void
DrawCallPool::macroPropertyAddedHandler(data::Store&                store,
                                        const std::list<DrawCall*>& drawCalls)
{
    removeMacroCallback(store.propertyAdded());

    addMacroCallback(
        store.propertyRemoved(),
        store,
        std::bind(&DrawCallPool::macroPropertyRemovedHandler, this, store, drawCalls)
    );

    macroPropertyChangedHandler(drawCalls);
}

void
DrawCallPool::macroPropertyRemovedHandler(data::Store&                  store,
                                          const std::list<DrawCall*>&   drawCalls)
{
    removeMacroCallback(store.propertyRemoved());

    addMacroCallback(
        store.propertyAdded(),
        store,
        std::bind(&DrawCallPool::macroPropertyAddedHandler, this, store, drawCalls)
    );

    macroPropertyChangedHandler(drawCalls);
}

void
DrawCallPool::unwatchProgramSignature(DrawCall&                     drawCall,
                                      const data::MacroBindingMap&  macroBindings,
                                      data::Store&                  rootData,
                                      data::Store&                  rendererData,
                                      data::Store&                  targetData)
{
    for (const auto& macroNameAndBinding : macroBindings.bindings)
    {
        const auto& macroBinding = macroNameAndBinding.second;
        auto& store = macroBinding.source == data::Binding::Source::ROOT ? rootData
            : macroBinding.source == data::Binding::Source::RENDERER ? rendererData
            : targetData;
        auto bindingKey = MacroBindingKey(&macroBinding, &store);
        auto& drawCalls = _macroToDrawCalls.at(bindingKey);

        drawCalls.remove(&drawCall);

        if (drawCalls.size() == 0)
            _macroToDrawCalls.erase(bindingKey);
        
        if (macroBinding.type != data::MacroBinding::Type::UNSET)
            removeMacroCallback(store.propertyChanged());
        else
        {
            auto propertyName = Store::getActualPropertyName(drawCall.variables(), macroBinding.propertyName);

            if (store.hasProperty(propertyName))
                removeMacroCallback(store.propertyRemoved());
            else
                removeMacroCallback(store.propertyAdded());
        }
    }
}

void
DrawCallPool::initializeDrawCall(DrawCall& drawCall)
{
    auto pass = drawCall.pass();
    auto programAndSignature = pass->selectProgram(
        drawCall.variables(), drawCall.targetData(), drawCall.rendererData(), drawCall.rootData()
    );

    if (programAndSignature.first == drawCall.program())
        return;

    drawCall.bind(programAndSignature.first);

    // FIXME: avoid const_cast
    if (programAndSignature.second != nullptr)
        watchProgramSignature(
            drawCall,
            drawCall.pass()->macroBindings(),
            drawCall.rootData(),
            drawCall.rendererData(),
            drawCall.targetData()
        );
}

void
DrawCallPool::update()
{
    for (auto* drawCallPtr : _invalidDrawCalls)
        initializeDrawCall(*drawCallPtr);

    _invalidDrawCalls.clear();

    _drawCalls.sort(
        std::bind(
            &DrawCallPool::compareDrawCalls,
            this,
            std::placeholders::_1,
            std::placeholders::_2
        )
    );
}

bool
DrawCallPool::compareDrawCalls(DrawCall& a, DrawCall& b)
{
    // FIXME: Sort according to render targets

    const float aPriority = a.priority();
    const float bPriority = b.priority();
    const bool arePrioritiesEqual = fabsf(aPriority - bPriority) < 1e-3f;

    if (!arePrioritiesEqual)
        return aPriority > bPriority;

    if (a.zSorted() || b.zSorted())
    {
        auto aPosition = getDrawcallEyePosition(a);
        auto bPosition = getDrawcallEyePosition(b);

        return aPosition.z > bPosition.z;
    }
    /*
    else
    {
        // FIXME
        // ordered by target texture id, if any
        //return a->target() && (!b->target() || (a->target()->id() > b->target()->id()));

        return false;
    }*/
}

math::vec3
DrawCallPool::getDrawcallEyePosition(DrawCall& drawcall)
{
    return drawcall.getEyeSpacePosition();
}

void
DrawCallPool::invalidateDrawCalls(const DrawCallIteratorPair&                         iterators,
                                  const std::unordered_map<std::string, std::string>& variables)
{
    auto end = std::next(iterators.second);

    for (auto it = iterators.first; it != end; ++it)
    {
        auto& drawCall = *it;

        _invalidDrawCalls.insert(&drawCall);
        drawCall.variables().clear();
        drawCall.variables().insert(variables.begin(), variables.end());
    }
}
