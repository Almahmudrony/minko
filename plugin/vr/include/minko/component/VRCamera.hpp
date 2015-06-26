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

#include "minko/Minko.hpp"
#include "minko/OculusCommon.hpp"
#include "minko/Signal.hpp"
#include "minko/component/AbstractComponent.hpp"
#include "minko/component/PerspectiveCamera.hpp"

namespace minko
{
    namespace component
    {
        class VRCamera :
            public AbstractComponent
        {
        public:
            typedef std::shared_ptr<VRCamera> Ptr;

        private:
            typedef std::shared_ptr<scene::Node>				NodePtr;
            typedef std::shared_ptr<AbstractComponent>			AbsCmpPtr;
            typedef std::shared_ptr<SceneManager>				SceneMgrPtr;
            typedef std::shared_ptr<render::AbstractTexture>    AbsTexturePtr;

        private:
            SceneMgrPtr                                         _sceneManager;

            Signal<NodePtr, NodePtr, NodePtr>::Slot             _addedSlot;
            Signal<NodePtr, NodePtr, NodePtr>::Slot             _removedSlot;
            Signal<SceneMgrPtr, uint, AbsTexturePtr>::Slot      _renderBeginSlot;

            std::shared_ptr<oculus::VRImpl>                     _VRImpl;

            std::shared_ptr<component::Renderer>                _leftRenderer;
            std::shared_ptr<component::Renderer>                _rightRenderer;
            std::shared_ptr<scene::Node>                        _leftCameraNode;
            std::shared_ptr<scene::Node>                        _rightCameraNode;

            float                                               _viewportWidth;
            float                                               _viewportHeight;

        public:
            inline static
            Ptr
            create(int viewportWidth,
                   int viewportHeight,
                   float zNear  = 0.1f,
                   float zFar   = 1000.0f,
                   void* window = nullptr)
            {
                auto ptr = std::shared_ptr<VRCamera>(new VRCamera(
                    viewportWidth, viewportHeight, zNear, zFar
                ));

                if (!ptr)
                    return nullptr;

                ptr->initialize(viewportWidth, viewportHeight, zNear, zFar, window);

                return ptr;
            }

            void
            updateViewport(int viewportWidth, int viewportHeight);

            static
            bool
            detected();

            Signal<>::Ptr
            actionButtonPressed();

        public:
            ~VRCamera();

        private:
            VRCamera(int viewportWidth, int viewportHeight, float zNear, float zFar);

            void
            initialize(int viewportWidth, int viewportHeight, float zNear, float zFar, void* window);

            void
            updateCameraOrientation();

            void
            targetAdded(NodePtr target);

            void
            targetRemoved(NodePtr target);

            void
            findSceneManager();

            void
            setSceneManager(SceneMgrPtr);
        };
    }
}

