In this tutorial, we will introduce how to work with the `BasicMaterial` to achieve simple color or texture based rendering. This tutorial will also introduce the relation between a `Material` and an `Effect` and how they are expected to work together during rendering.

Step 1: Creating the material
-----------------------------

To create a new `BasicMaterial` object, we simply create the `BasicMaterial::create()` static method:

```cpp
auto material = material::BasicMaterial::create();
```


Don't forget to use the `material::` prefix since the `BasicMaterial` class is defined in the `minko::material` namespace (and assuming you are already using the `minko` namespace).

It is important to understand the difference between a `Material` - such as the `BasicMaterial` - and an `Effect` - such as the `Basic.effect`:

-   an `Effect` is a (set of) rendering function(s)/program(s) - also known as shaders - that will compute the final color of a pixel on the screen,
-   a `Material` is a set of key/value pairs that provide the input data for the `Effect` to work on.

If an `Effect` was a function, then the values provided by a `Material` would be its arguments. It also means that materials and effects are loosely coupled: despite the fact that a `BasicMaterial` is expected to work with a `Basic.effect`, you can use one without the other. For example, you could:

-   use the `BasicMaterial` with the `Phong.effect`, but you won't have code-hinting on Phong related material properties in your IDE;
-   or use a `PhongMaterial` with a `Basic.effect`, but then a lot of material properties would actually be available in the code but unused during rendering.

To put it in a nutshell, you can use any `Material` with any `Effect` as long as the first provides all the property values expected by the second.

Step 2: Setting a diffuse color
-------------------------------

The `BasicMaterial` is expected to work along with the `Basic.effect`. As such, it provides getters and setters according to the binding property expected by this effect. One of this properties is the diffuse color.

The diffuse color is a solid RGBA color that will be used as the final pixel color. It can be set using three different methods that will all have the exact same behavior:

-   `BasicMaterial::diffuseColor(math::vec4 color)`, where `color` is an RGBA Vector4 value
-   `BasicMaterial::diffuseColor(uint rgba)`, where `rgba` is an RGBA formatted unsigned integer value
-   `BasicMaterial::set({{ "diffuseColor", math::vec4 color }})`, where `color` is an RGBA Vector4 value

It is important to understand that the first two methods are just syntaxic sugar on top of the `BasicMaterial::set()` method. This method itself is inherited from the `[data::Provider`](data::Provider`) class. In the end, any material is just a "dynamic" `[data::Provider`](data::Provider`) that declares user-friendly inlined getter/setters for the properties expected by the corresponding effect.

Therefore, this code:

```cpp
material->diffuseColor(Vector4::create(1.f, 0.f, 0.f, 1.f));
```


is strictly equivalent to:

```cpp
material->set({{"diffuseColor", vec4(1.f, 0.f, 0.f, 1.f)}});
```


This approach is very useful because it mixes the flexibility of dynamic properties referenced by simple string values and the user-friendliness of code-hinting for statically declared methods. According to what you're trying to do, you can chose the method that best fits your needs.

Performance wise, the static getters/setters declared in a material are (should be) inlined so it should make no difference.

To learn more on this subject, please read the [The difference between the Material_set() method and setter methods](../article/The_difference_between_the_Material_set%28%29_method_and_setter_methods.md]) article.

Step 3: Setting a diffuse map (or texture)
------------------------------------------

The `BasicMaterial` can also sample an RGBA texture as the final color of the pixel. To do this, we just have to set the `diffuseMap` property using the `BasicMaterial::diffuseMap()` setter:

```cpp
material->diffuseMap(texture);
```


or the `BasicMaterial::set()` dynamic method directly:

```cpp
material->set({{"diffuseMap", texture}});
```


If you set the `diffuseMap`, the `diffuseColor` property will not be used anymore. This is done thanks to Minko's support for "über-shaders". This feature allows the rendering engine to use rendering programs - or shaders - that can handle many different situations such as in this case rendering with a texture or a solid color. This is a very efficient solution to make it easy to customize rendering with little effort.

You can read more about textures in the [Loading and using textures](../tutorial/15-Loading_and_using_textures.md) tutorial.

Final code
----------

```cpp
/*
Copyright (c) 2016 Aerys

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
#include "minko/MinkoPNG.hpp"

using namespace minko;
using namespace minko::math;
using namespace minko::component;

const math::uint WINDOW_WIDTH = 800;
const math::uint WINDOW_HEIGHT = 600;

const std::string TEXTURE_BOX = "texture/box.png";

int	main(int argc, char** argv)
{
	auto canvas = Canvas::create("Tutorial - Working with the BasicMaterial", WINDOW_WIDTH, WINDOW_HEIGHT);
	auto sceneManager = component::SceneManager::create(canvas);

	sceneManager->assets()->loader()->options()
		->registerParser<file::PNGParser>("png");

	sceneManager->assets()->loader()
		->queue("effect/Basic.effect")
		->queue(TEXTURE_BOX);

	auto root = scene::Node::create("root")
		->addComponent(sceneManager);

	auto camera = scene::Node::create("camera")
		->addComponent(Renderer::create(0x7f7f7fff))
		->addComponent(PerspectiveCamera::create((float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, (float)M_PI * 0.25f, .1f, 1000.f));

	root->addChild(camera);

	auto complete = sceneManager->assets()->loader()->complete()->connect([&](file::Loader::Ptr loader)
	{
		auto texturedCube = scene::Node::create("texturedCube")
			->addComponent(Transform::create(translate(vec3(-2.f, 0.f, -5.f))))
			->addComponent(Surface::create(
				geometry::CubeGeometry::create(sceneManager->assets()->context()),
				material::BasicMaterial::create()->diffuseMap(sceneManager->assets()->texture(TEXTURE_BOX)),
				sceneManager->assets()->effect("effect/Basic.effect")
				));

		root->addChild(texturedCube);

		auto coloredCube = scene::Node::create("coloredCube")
			->addComponent(Transform::create(translate(vec3(2.f, 0.f, -5.f))))
			->addComponent(Surface::create(
				geometry::CubeGeometry::create(sceneManager->assets()->context()),
				material::BasicMaterial::create()->diffuseColor(vec4(0.f, 0.f, 1.f, 1.f)),
				sceneManager->assets()->effect("effect/Basic.effect")
				));

		root->addChild(coloredCube);
	});

	sceneManager->assets()->loader()->load();

	auto enterFrame = canvas->enterFrame()->connect([&](Canvas::Ptr canvas, float t, float dt)
	{
		sceneManager->nextFrame(t, dt);
	});

	canvas->run();

	return 0;
}
```
