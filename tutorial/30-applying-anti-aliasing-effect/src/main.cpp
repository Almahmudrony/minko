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
    auto canvas = Canvas::create("Minko Tutorial - Applying antialiasing effect", WINDOW_WIDTH, WINDOW_HEIGHT);
    auto sceneManager = SceneManager::create(canvas);

	sceneManager->assets()->loader()
        ->queue("effect/Basic.effect")
        ->queue("effect/FXAA/FXAA.effect");

	auto complete = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
    {
        auto effect = sceneManager->assets()->effect("effect/FXAA/FXAA.effect");

        if (!effect)
            throw std::logic_error("The FXAA effect has not been loaded.");

        auto root = scene::Node::create("root")
            ->addComponent(sceneManager);

        auto camera = scene::Node::create("camera")
            ->addComponent(Renderer::create(0x00000000))
            ->addComponent(PerspectiveCamera::create(
                (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT, float(M_PI) * 0.25f, .1f, 1000.f)
            )
            ->addComponent(Transform::create(
                math::inverse(math::lookAt(math::vec3(0.f, 0.f, -5.f), math::vec3(), math::vec3(0.f, 1.f, 0.f))))
            );
        root->addChild(camera);

        auto material = material::BasicMaterial::create();
        material->diffuseColor(math::vec4(0.f, 0.f, 1.f, 1.f));

        auto cube = scene::Node::create("cube")
            ->addComponent(Transform::create())
            ->addComponent(Surface::create(
                geometry::CubeGeometry::create(canvas->context()),
                material,
    			sceneManager->assets()->effect("effect/Basic.effect")
            ));
        root->addChild(cube);

        auto renderTarget = render::Texture::create(canvas->context(), math::clp2(WINDOW_WIDTH), math::clp2(WINDOW_HEIGHT), false, true);
        renderTarget->upload();

        auto ppMaterial = material::BasicMaterial::create();
        ppMaterial->diffuseMap(renderTarget);

        effect->data()->set("invertedDiffuseMapSize", math::vec2(1.f / float(renderTarget->width()), 1.f / float(renderTarget->height())));

        auto renderer = Renderer::create();
        auto postProcessingScene = scene::Node::create()
        ->addComponent(renderer)
        ->addComponent(
            Surface::create(
                geometry::QuadGeometry::create(sceneManager->assets()->context()),
                ppMaterial,
                effect
            )
        );

        auto resized = canvas->resized()->connect([&](AbstractCanvas::Ptr canvas, uint width, uint height)
        {
            camera->component<PerspectiveCamera>()->aspectRatio((float) width / (float) height);

			renderTarget = render::Texture::create(sceneManager->assets()->context(), math::clp2(width), math::clp2(height), false, true);
            renderTarget->upload();

            ppMaterial->diffuseMap(renderTarget);
            effect->data()->set("invertedDiffuseMapSize", math::vec2(1.f / float(renderTarget->width()), 1.f / float(renderTarget->height())));
        });


        auto enableFXAA = true;

        auto keyDown = canvas->keyboard()->keyDown()->connect([&](input::Keyboard::Ptr k)
        {
            if (k->keyIsDown(input::Keyboard::SPACE))
            {
                enableFXAA = !enableFXAA;

                if (enableFXAA)
                    std::cout << "Enable FXAA" << std::endl;
                else
                    std::cout << "Disable FXAA" << std::endl;
            }
        });

        auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, float t, float dt)
        {
            cube->component<Transform>()->matrix(cube->component<Transform>()->matrix() * math::rotate(.01f, math::vec3(0.f, 1.f, 0.f)));

            if (enableFXAA)
            {
                sceneManager->nextFrame(t, dt, renderTarget);
				renderer->render(sceneManager->assets()->context());
            }
            else
            {
                sceneManager->nextFrame(t, dt);
            }
        });

        canvas->run();
    });

	sceneManager->assets()->loader()->load();

    return 0;
}