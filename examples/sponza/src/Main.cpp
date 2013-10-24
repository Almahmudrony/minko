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

#include "minko/Minko.hpp"
#include "minko/MinkoPNG.hpp"
#include "minko/MinkoJPEG.hpp"
#include "minko/MinkoMk.hpp"
#include "minko/MinkoBullet.hpp"
#include "minko/MinkoParticles.hpp"
#include "minko/MinkoSDL.hpp"

#include "minko/deserialize/TypeDeserializer.hpp"

#include "minko/component/Fire.hpp"

using namespace minko;
using namespace minko::component;
using namespace minko::math;

const float WINDOW_WIDTH		= 1024.0f;
const float WINDOW_HEIGHT		= 500.0f;

const std::string MK_NAME			= "model/Sponza_lite_sphere.mk";
const std::string DEFAULT_EFFECT	= "effect/Phong.effect";
const std::string CAMERA_NAME		= "camera";

const float CAMERA_LIN_SPEED	= 0.05f;
const float CAMERA_ANG_SPEED	= PI * 2.f / 180.0f;
const float CAMERA_MASS			= 50.0f;
const float CAMERA_FRICTION		= 0.6f;

Renderer::Ptr			renderer			= nullptr;
auto					mesh				= scene::Node::create("mesh");
auto					group				= scene::Node::create("group");
auto					camera				= scene::Node::create("camera");
auto					root				= scene::Node::create("root");
auto					speed				= 0.0f;
auto					angSpeed			= 0.0f;
float					rotationX			= 0.0f;
float					rotationY			= 0.0f;
float					mousePositionX		= 0.0f;
float					mousePositionY		= 0.0f;
Vector3::Ptr			target				= Vector3::create();
Vector3::Ptr			eye					= Vector3::create();
bullet::Collider::Ptr	cameraCollider		= nullptr;

template <typename T>
static
void
read(std::stringstream& stream, T& value)
{
	stream.read(reinterpret_cast<char*>(&value), sizeof (T));
}

template <typename T>
static
T
swap_endian(T u)
{
	union
	{
		T u;
		unsigned char u8[sizeof(T)];
	} source, dest;

	source.u = u;

	for (size_t k = 0; k < sizeof(T); k++)
		dest.u8[k] = source.u8[sizeof(T) - k - 1];

	return dest.u;
}

template <typename T>
T
readAndSwap(std::stringstream& stream)
{
	T value;
	stream.read(reinterpret_cast<char*>(&value), sizeof (T));

	return swap_endian(value);
}

bullet::AbstractPhysicsShape::Ptr
deserializeShape(Qark::Map&			shapeData,
				 scene::Node::Ptr&	node)
{
	bullet::AbstractPhysicsShape::Ptr deserializedShape;

	int type = Any::cast<int>(shapeData["type"]);

	double rx	= 0.0;
	double ry	= 0.0;
	double rz	= 0.0;
	double h	= 0.0;
	double r	= 0.0;

	std::stringstream stream;
	switch (type)
	{
	case 101: // multiprofile
		deserializedShape = deserializeShape(Any::cast<Qark::Map&>(shapeData["shape"]), node);
		break;
	case 2: // BOX
		{
			Qark::ByteArray& source = Any::cast<Qark::ByteArray&>(shapeData["data"]);
			stream.write(&*source.begin(), source.size());

			rx = readAndSwap<double>(stream);
			ry = readAndSwap<double>(stream);
			rz = readAndSwap<double>(stream);

			deserializedShape = bullet::BoxShape::create(rx, ry, rz);
		}
		break;
	case 5 : // CONE
		{
			Qark::ByteArray& source = Any::cast<Qark::ByteArray&>(shapeData["data"]);
			stream.write(&*source.begin(), source.size());

			r = readAndSwap<double>(stream);
			h = readAndSwap<double>(stream);

			deserializedShape = bullet::ConeShape::create(r, h);
		}
		break;
	case 6 : // BALL
		{
			Qark::ByteArray& source = Any::cast<Qark::ByteArray&>(shapeData["data"]);
			stream.write(&*source.begin(), source.size());

			r = readAndSwap<double>(stream);

			deserializedShape = bullet::SphereShape::create(r);
		}
		break;
	case 7 : // CYLINDER
		{
			Qark::ByteArray& source = Any::cast<Qark::ByteArray&>(shapeData["data"]);
			stream.write(&*source.begin(), source.size());

			r = readAndSwap<double>(stream);
			h = readAndSwap<double>(stream);

			deserializedShape = bullet::CylinderShape::create(r, h, r);
		}
		break;
	case 100 : // TRANSFORM
		{
			deserializedShape		= deserializeShape(Any::cast<Qark::Map&>(shapeData["subGeometry"]), node);

			auto delta				= deserialize::TypeDeserializer::matrix4x4(shapeData["delta"]);
			auto modelToWorld		= node->component<Transform>()->modelToWorldMatrix(true);

			deserializedShape->initialize(delta, modelToWorld);
		}
		break;
	default:
		deserializedShape = nullptr;
	}

	return deserializedShape;
}

std::shared_ptr<bullet::Collider>
deserializeBullet(Qark::Map&						nodeInformation,
				  file::MkParser::ControllerMap&	controllerMap,
				  file::MkParser::NodeMap&			nodeMap,
				  scene::Node::Ptr&					node)
{
	Qark::Map& colliderData = Any::cast<Qark::Map&>(nodeInformation["defaultCollider"]);
	Qark::Map& shapeData	= Any::cast<Qark::Map&>(colliderData["shape"]);

	bullet::AbstractPhysicsShape::Ptr shape = deserializeShape(shapeData, node);

	float mass			= 1.0f;
	double vx			= 0.0;
	double vy			= 0.0;
	double vz			= 0.0;
	double avx			= 0.0;
	double avy			= 0.0;
	double avz			= 0.0;
	bool sleep			= false;
	bool rotate			= false;
	bool trigger		= false;
	double friction		= 0.5; // bullet's advices
	double restitution	= 0.0; // bullet's advices

	if (shapeData.find("materialProfile") != shapeData.end())
	{
		Qark::ByteArray& materialProfileData = Any::cast<Qark::ByteArray&>(shapeData["materialProfile"]);
		std::stringstream	stream;
		stream.write(&*materialProfileData.begin(), materialProfileData.size());

		double density	= readAndSwap<double>(stream);
		mass			= density * shape->volume();
		friction		= readAndSwap<double>(stream);
		restitution		= readAndSwap<double>(stream);
	}

	if (shapeData.find("logicProfile") != shapeData.end())
	{
		Qark::ByteArray& logicProfileData = Any::cast<Qark::ByteArray&>(shapeData["logicProfile"]);
		std::stringstream	stream;
		stream.write(&*logicProfileData.begin(), logicProfileData.size());

		trigger	= readAndSwap<bool>(stream);
	}

	if (colliderData.find("dynamics") == colliderData.end())
		mass = 0.0; // static object
	else
	{
		Qark::ByteArray& dynamicsData = Any::cast<Qark::ByteArray&>(colliderData["dynamics"]);
		std::stringstream	stream;
		stream.write(&*dynamicsData.begin(), dynamicsData.size());

		vx		= readAndSwap<double>(stream);
		vy		= readAndSwap<double>(stream);
		vz		= readAndSwap<double>(stream);
		avx		= readAndSwap<double>(stream);
		avy		= readAndSwap<double>(stream);
		avz		= readAndSwap<double>(stream);
		sleep	= readAndSwap<bool>(stream);
		rotate	= readAndSwap<bool>(stream);
	}


	bullet::ColliderData::Ptr data = bullet::ColliderData::create(mass, shape);

	data->linearVelocity(vx, vy, vz);
	data->angularVelocity(avx, avy, avz);
	data->friction(friction);
	data->restitution(restitution);
	data->triggerCollisions(trigger);

	if (!rotate)
		data->angularFactor(0.0f, 0.0f, 0.0f);
	//collider->disableDeactivation(sleep == false);
	data->disableDeactivation(true);

	return bullet::Collider::create(data);
}

component::bullet::Collider::Ptr
initializeDefaultCameraCollider()
{
	auto shape		= bullet::BoxShape::create(0.2f, 0.3f, 0.2f);
	auto data		= bullet::ColliderData::create(CAMERA_MASS, shape);

	data->restitution(0.5f);
	data->angularFactor(0.0f, 0.0f, 0.0f);
	data->friction(CAMERA_FRICTION);
	data->disableDeactivation(true);

	return bullet::Collider::create(data);
}

void
initializeCamera(scene::Node::Ptr group)
{
	auto cameras = scene::NodeSet::create(group)
		->descendants(true)
		->where([](scene::Node::Ptr node)
				{
					return node->name() == CAMERA_NAME;
				});

	bool cameraInGroup = false;
	if (cameras->nodes().empty())
	{
		// default camera
		camera = scene::Node::create(CAMERA_NAME);

		camera->addComponent(Transform::create());
		camera->component<Transform>()->transform()
			->appendTranslation(0.0f, 0.75f, 5.0f)
			->appendRotationY(PI * 0.5);

		cameraCollider = initializeDefaultCameraCollider();
		camera->addComponent(cameraCollider);
	}
	else
	{
		// set-up camera from the mk file
		camera = cameras->nodes().front();
		cameraInGroup = true;

		if (camera->hasComponent<component::bullet::Collider>())
			cameraCollider = camera->component<component::bullet::Collider>();
	}

	if (!camera->hasComponent<Transform>())
		throw std::logic_error("Camera (deserialized or created) must have a Transform.");

	camera->addComponent(renderer);
	camera->addComponent(PerspectiveCamera::create(.785f, WINDOW_WIDTH / WINDOW_HEIGHT, .1f, 1000.f));
	root->addChild(camera);
}

void
initializePhysics()
{
	auto physicWorld = bullet::PhysicsWorld::create(renderer);

	physicWorld->setGravity(math::Vector3::create(0.f, -9.8f, 0.f));
	root->addComponent(physicWorld);
}

void
printFramerate(const unsigned int delay = 1)
{
	static auto start = time(NULL);
	static auto numFrames = 0;

	int secondTime = time(NULL);

	++numFrames;

	if ((secondTime - start) >= 1)
	{
		std::cout << numFrames << " fps." << std::endl;
		start = time(NULL);
		numFrames = 0;
	}
}

int
main(int argc, char** argv)
{
	MinkoSDL::initialize("Minko Example - Sponza", WINDOW_WIDTH, WINDOW_HEIGHT);

	file::MkParser::registerController(
		"colliderController",
		std::bind(
			deserializeBullet,
			std::placeholders::_1,
			std::placeholders::_2,
			std::placeholders::_3,
			std::placeholders::_4
		)
	);

	auto sceneManager = SceneManager::create(MinkoSDL::context());

	sceneManager->assets()
		->registerParser<file::PNGParser>("png")
		->registerParser<file::JPEGParser>("jpg")
		->registerParser<file::MkParser>("mk")
		->geometry("cube", geometry::CubeGeometry::create(MinkoSDL::context()));

	auto options = sceneManager->assets()->defaultOptions();

	options->material(material::Material::create()->set("triangleCulling", render::TriangleCulling::FRONT));
	options->generateMipmaps(true);

	// load sponza lighting effect and set it as the default effect
	sceneManager->assets()
		->load("effect/Phong.effect")
		->load("effect/Basic.effect");

	options->effect(sceneManager->assets()->effect("effect/Basic.effect"));

	// load other assets
	sceneManager->assets()
		->queue("texture/firefull.jpg")
		->queue("effect/Particles.effect")
		->queue(MK_NAME);

	renderer = Renderer::create();

	initializePhysics();

	auto _ = sceneManager->assets()->complete()->connect([=](file::AssetLibrary::Ptr assets)
	{
		scene::Node::Ptr mk = assets->node(MK_NAME);
		initializeCamera(mk);

		auto lights = scene::Node::create();

		lights
			->addComponent(component::AmbientLight::create())
			->addComponent(component::DirectionalLight::create())
			->addComponent(component::Transform::create());
		lights->component<Transform>()->transform()->lookAt(Vector3::zero(), Vector3::create(-1.f, -1.f, -1.f));
		//root->addChild(lights);

		root->addChild(group);
		root->addComponent(sceneManager);

		group->addComponent(Transform::create());
		group->addChild(mk);
		scene::NodeSet::Ptr fireNodes = scene::NodeSet::create(group)
			->descendants()
			->where([](scene::Node::Ptr node)
		{
			return node->name() == "fire";
		});

		auto fire = Fire::create(assets);
		for (auto fireNode : fireNodes->nodes())
			fireNode->addComponent(fire);

		auto keyDown = MinkoSDL::keyDown()->connect([&](const Uint8* keyboard)
		{
			auto collider = true;
			auto cameraTransform = camera->component<Transform>()->transform();

			if (!collider)
			{
				if (keyboard[SDL_SCANCODE_UP] ||
					keyboard[SDL_SCANCODE_W] ||
					keyboard[SDL_SCANCODE_Z])
					cameraTransform->prependTranslation(0.f, 0.f, -CAMERA_LIN_SPEED);
				else if (keyboard[SDL_SCANCODE_DOWN] ||
					keyboard[SDL_SCANCODE_S])
					cameraTransform->prependTranslation(0.f, 0.f, CAMERA_LIN_SPEED);
				if (keyboard[SDL_SCANCODE_LEFT] ||
					keyboard[SDL_SCANCODE_A] ||
					keyboard[SDL_SCANCODE_Q])
					cameraTransform->prependRotation(-CAMERA_ANG_SPEED, Vector3::yAxis());
				else if (keyboard[SDL_SCANCODE_RIGHT] ||
					keyboard[SDL_SCANCODE_D])
					cameraTransform->prependRotation(CAMERA_ANG_SPEED, Vector3::yAxis());
			}
			else
			{
				if (keyboard[SDL_SCANCODE_UP] ||
					keyboard[SDL_SCANCODE_W] ||
					keyboard[SDL_SCANCODE_Z])
					// go forward
					cameraTransform->prependTranslation(Vector3::create(0.0f, 0.0f, -CAMERA_LIN_SPEED));
				else if (keyboard[SDL_SCANCODE_DOWN] ||
					keyboard[SDL_SCANCODE_S])
					// go backward
					cameraTransform->prependTranslation(Vector3::create(0.0f, 0.0f, CAMERA_LIN_SPEED));
				if (keyboard[SDL_SCANCODE_LEFT] ||
					keyboard[SDL_SCANCODE_A] ||
					keyboard[SDL_SCANCODE_Q])
					cameraTransform->prependTranslation(-CAMERA_LIN_SPEED, 0.0f, 0.0f);
				else if (keyboard[SDL_SCANCODE_RIGHT] ||
					keyboard[SDL_SCANCODE_D])
					cameraTransform->prependTranslation(CAMERA_LIN_SPEED, 0.0f, 0.0f);

				eye = cameraTransform->translation();

				if (keyboard[SDL_SCANCODE_SPACE] && eye->y() <= 0.5f)
					cameraTransform->prependTranslation(0.0f, 4 * CAMERA_LIN_SPEED, 0.0f);
			}
		});

		auto enterFrame = MinkoSDL::enterFrame()->connect([&]()
		{
			sceneManager->nextFrame();
		});

		MinkoSDL::run();
	});

	sceneManager->assets()->load();

	return 0;
}
