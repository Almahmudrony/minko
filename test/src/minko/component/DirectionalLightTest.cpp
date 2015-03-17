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

#include "DirectionalLightTest.hpp"

using namespace minko;
using namespace minko::component;

TEST_F(DirectionalLightTest, Create)
{
    try
    {
        auto al = DirectionalLight::create();
    }
    catch (...)
    {
        ASSERT_TRUE(false);
    }
}

TEST_F(DirectionalLightTest, AddLight)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    lights->addComponent(DirectionalLight::create(.1f, .3f));
    root->addChild(lights);

    ASSERT_TRUE(root->data().hasProperty("directionalLight.length"));
    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 1);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].color"), math::vec3(1.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].diffuse"), .1f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].specular"), .3f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].direction"), math::vec3(0.f, 0.f, -1.f));
}

TEST_F(DirectionalLightTest, RemoveSingleLight)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");
    auto al = DirectionalLight::create();

    lights->addComponent(al);
    root->addChild(lights);
    lights->removeComponent(al);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 0);
    ASSERT_FALSE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_FALSE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_FALSE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_FALSE(root->data().hasProperty("directionalLight[0].direction"));
}

TEST_F(DirectionalLightTest, AddMultipleLights)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 1);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].color"), math::vec3(1.f, 0.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].diffuse"), .1f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].specular"), .2f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].direction"), math::vec3(0.f, 0.f, -1.f));

    auto al2 = DirectionalLight::create(.3f, .4f);
    al2->color(math::vec3(0.f, 1.f, 0.f));
    lights->addComponent(al2);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 2);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].color"), math::vec3(0.f, 1.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].diffuse"), .3f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].specular"), .4f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].direction"), math::vec3(0.f, 0.f, -1.f));

    auto al3 = DirectionalLight::create(.5f, .6f);
    al3->color(math::vec3(0.f, 0.f, 1.f));
    lights->addComponent(al3);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 3);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[2].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[2].color"), math::vec3(0.f, 0.f, 1.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[2].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[2].diffuse"), .5f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[2].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[2].specular"), .6f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[2].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[2].direction"), math::vec3(0.f, 0.f, -1.f));
}

TEST_F(DirectionalLightTest, RemoveFirstLight)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    auto al2 = DirectionalLight::create(.3f, .4f);
    al2->color(math::vec3(0.f, 1.f, 0.f));
    lights->addComponent(al2);

    auto al3 = DirectionalLight::create(.5f, .6f);
    al3->color(math::vec3(0.f, 0.f, 1.f));
    lights->addComponent(al3);

    lights->removeComponent(al1);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 2);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].color"), math::vec3(0.f, 1.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].diffuse"), .3f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].specular"), .4f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].direction"), math::vec3(0.f, 0.f, -1.f));

    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].color"), math::vec3(0.f, 0.f, 1.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].diffuse"), .5f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].specular"), .6f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].direction"), math::vec3(0.f, 0.f, -1.f));
}

TEST_F(DirectionalLightTest, RemoveNthLight)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    auto al2 = DirectionalLight::create(.3f, .4f);
    al2->color(math::vec3(0.f, 1.f, 0.f));
    lights->addComponent(al2);

    auto al3 = DirectionalLight::create(.5f, .6f);
    al3->color(math::vec3(0.f, 0.f, 1.f));
    lights->addComponent(al3);

    lights->removeComponent(al2);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 2);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].color"), math::vec3(1.f, 0.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].diffuse"), .1f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].specular"), .2f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].direction"), math::vec3(0.f, 0.f, -1.f));

    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].color"), math::vec3(0.f, 0.f, 1.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].diffuse"), .5f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].specular"), .6f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].direction"), math::vec3(0.f, 0.f, -1.f));
}

TEST_F(DirectionalLightTest, RemoveLastLight)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    auto al2 = DirectionalLight::create(.3f, .4f);
    al2->color(math::vec3(0.f, 1.f, 0.f));
    lights->addComponent(al2);

    auto al3 = DirectionalLight::create(.5f, .6f);
    al3->color(math::vec3(0.f, 0.f, 1.f));
    lights->addComponent(al3);

    lights->removeComponent(al3);

    ASSERT_EQ(root->data().get<int>("directionalLight.length"), 2);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].color"), math::vec3(1.f, 0.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].diffuse"), .1f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[0].specular"), .2f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[0].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[0].direction"), math::vec3(0.f, 0.f, -1.f));

    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].color"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].color"), math::vec3(0.f, 1.f, 0.f));
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].diffuse"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].diffuse"), .3f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].specular"));
    ASSERT_EQ(root->data().get<float>("directionalLight[1].specular"), .4f);
    ASSERT_TRUE(root->data().hasProperty("directionalLight[1].direction"));
    ASSERT_EQ(root->data().get<math::vec3>("directionalLight[1].direction"), math::vec3(0.f, 0.f, -1.f));
}

TEST_F(DirectionalLightTest, RotateXPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::pi<float>(), math::vec3(1.f, 0.f, 0.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
        root->data().get<math::vec3>("directionalLight[0].direction"),
        math::vec3(0.f, 0.f, 1.f),
        math::epsilon<float>()
        ),
        math::bvec3(true)
        );
}

TEST_F(DirectionalLightTest, RotateXHalfPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::half_pi<float>(), math::vec3(1.f, 0.f, 0.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(0.f, 1.f, 0.f),
            math::epsilon<float>()
        ),
        math::bvec3(true)
    );
}

TEST_F(DirectionalLightTest, RotateYPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::pi<float>(), math::vec3(0.f, 1.f, 0.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(0.f, 0.f, 1.f),
            math::epsilon<float>()
       ),
       math::bvec3(true)
    );
}

TEST_F(DirectionalLightTest, RotateYHalfPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::half_pi<float>(), math::vec3(0.f, 1.f, 0.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(-1.f, 0.f, 0.f),
            math::epsilon<float>()
        ),
        math::bvec3(true)
    );
}

TEST_F(DirectionalLightTest, RotateZPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::pi<float>(), math::vec3(0.f, 0.f, 1.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(0.f, 0.f, -1.f),
            math::epsilon<float>()
        ),
        math::bvec3(true)
    );
}

TEST_F(DirectionalLightTest, RotateZHalfPi)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::rotate(math::half_pi<float>(), math::vec3(0.f, 0.f, 1.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(0.f, 0.f, -1.f),
            math::epsilon<float>()
        ),
        math::bvec3(true)
    );
}

TEST_F(DirectionalLightTest, TranslateXYZ)
{
    auto root = scene::Node::create("root");
    auto lights = scene::Node::create("lights");

    root->addChild(lights);

    auto al1 = DirectionalLight::create(.1f, .2f);
    al1->color(math::vec3(1.f, 0.f, 0.f));
    lights->addComponent(al1);

    lights->addComponent(Transform::create(math::translate(math::sphericalRand(100.f))));
    lights->component<Transform>()->updateModelToWorldMatrix();

    ASSERT_EQ(
        math::epsilonEqual(
            root->data().get<math::vec3>("directionalLight[0].direction"),
            math::vec3(0.f, 0.f, -1.f),
            math::epsilon<float>()
        ),
        math::bvec3(true)
    );
}
