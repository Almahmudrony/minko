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

#include "minko/Minko.hpp"
#include "minko/MinkoSDL.hpp"

using namespace minko;
using namespace minko::component;

const uint WINDOW_WIDTH = 800;
const uint WINDOW_HEIGHT = 600;

int
main(int argc, char** argv)
{
    auto canvas = Canvas::create("Minko Tutorial - Rotating the camera around an object with the mouse", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto sceneManager = component::SceneManager::create(canvas);

    // We replace the basic effect by the Phong effect to have light effects
    sceneManager->assets()->loader()->queue("effect/Phong.effect");
    auto complete = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
    {
        auto root = scene::Node::create("root")
            ->addComponent(sceneManager);

        auto camera = scene::Node::create("camera")
            ->addComponent(Renderer::create(0x7f7f7fff))
            ->addComponent(Transform::create(
            math::inverse(math::lookAt(math::vec3(0., 0., -5.f), math::vec3(), math::vec3(0, 1, 0)))
            ))
            ->addComponent(PerspectiveCamera::create(
            (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, float(M_PI) * 0.25f, .1f, 1000.f)
            );
        root->addChild(camera);

        // Add a simple directional light to really see the camera rotation
        auto directionalLight = scene::Node::create("directionalLight")
            ->addComponent(DirectionalLight::create())
            ->addComponent(Transform::create(
                math::inverse(
                    math::lookAt(
                       math::vec3(), math::vec3(-0.33f, -0.33f, 0.33f), math::vec3(0, 1, 0))
                    )
                )
            );
        root->addChild(directionalLight);

        // Replace the cube by a sphere to inscrease light visibility
        auto sphereMaterial = material::BasicMaterial::create();
        sphereMaterial->diffuseColor(math::vec4(0.f, 0.f, 1.f, 1.f));

        auto sphere = scene::Node::create("sphere")
            ->addComponent(Surface::create(
                geometry::SphereGeometry::create(canvas->context()),
                sphereMaterial,
			    sceneManager->assets()->effect("effect/Phong.effect")
                )
            );

        root->addChild(sphere);

        Signal<input::Mouse::Ptr, int, int>::Slot mouseMove;
        float cameraRotationSpeed = 0.f;

        auto mouseDown = canvas->mouse()->leftButtonDown()->connect([&](input::Mouse::Ptr mouse)
        {
            mouseMove = canvas->mouse()->move()->connect([&](input::Mouse::Ptr mouse, int dx, int dy)
            {
                cameraRotationSpeed = (float) -dx * .01f;
            });
        });

        auto mouseUp = canvas->mouse()->leftButtonUp()->connect([&](input::Mouse::Ptr mouse)
        {
            mouseMove = nullptr;
        });

        auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, float t, float dt)
        {
            camera->component<Transform>()->matrix(math::rotate(cameraRotationSpeed, math::vec3(0, 1, 0)) * camera->component<Transform>()->matrix());
            cameraRotationSpeed *= .99f;

            sceneManager->nextFrame(t, dt);
        });

        canvas->run();
    });

    sceneManager->assets()->loader()->load();

    return 0;
}
