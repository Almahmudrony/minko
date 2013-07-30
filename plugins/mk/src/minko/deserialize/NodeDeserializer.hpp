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

#pragma once

#include "minko/Common.hpp"
#include "minko/scene/Node.hpp"
#include "minko/deserialize/TypeDeserializer.hpp"
#include "minko/file/MkOptions.hpp"
#include "minko/component/Surface.hpp"
#include "minko/component/Transform.hpp"
#include "minko/geometry/CubeGeometry.hpp"
#include "minko/math/Matrix4x4.hpp"
#include "minko/deserialize/GeometryDeserializer.hpp"

namespace minko
{
	namespace deserialize
	{
		class NodeDeserializer
		{

		private:
			typedef	std::map<std::string, Any>															NodeInfo;
			typedef std::map<std::shared_ptr<scene::Node>, std::vector<component::AbstractComponent>>	ControllerMap;
			typedef std::map<std::shared_ptr<scene::Node>, uint>										NodeMap;
			typedef std::shared_ptr<file::MkOptions>													OptionsPtr;

		private:
			inline static
			std::string&
			extractName(NodeInfo& nodeInfo)
			{
				return Any::cast<std::string&>(nodeInfo["name"]);
			}

		public:
			
			inline static
			std::shared_ptr<scene::Node>
			deserializeGroup(NodeInfo&		nodeInfo,
							 OptionsPtr		options,
							 ControllerMap&	controllerMap,
							 NodeMap&		nodeMap)
			{
				std::shared_ptr<scene::Node>		group			= scene::Node::create(extractName(nodeInfo));
				std::shared_ptr<math::Matrix4x4>	transformMatrix = TypeDeserializer::matrix4x4(nodeInfo["transformation"]);

				group->addComponent(component::Transform::create());
				group->component<component::Transform>()->transform()->copyFrom(transformMatrix);

				return group;
			}

			inline static
			std::shared_ptr<scene::Node>
			deserializeMesh(NodeInfo&		nodeInfo,
							OptionsPtr		options,
							ControllerMap&	controllerMap,
							NodeMap&		nodeMap)
			{
				std::shared_ptr<scene::Node>		mesh			= scene::Node::create(extractName(nodeInfo));
				std::shared_ptr<math::Matrix4x4>	transformMatrix = TypeDeserializer::matrix4x4(nodeInfo["transform"]);

                //if (!transformMatrix->isIdentity())
                //{
				    mesh->addComponent(component::Transform::create());
				    mesh->component<component::Transform>()->transform()->copyFrom(transformMatrix);
                //    std::cout << "transform:" << std::to_string(transformMatrix) << std::endl;
                //}
                //else
                //{
                //    std::cout << "no transform" << std::endl;
                //}

				Qark::ByteArray		geometryObject;
				int					copyId			= -1;
				std::string			geometryName	= "";
				bool				technique		= false;
				bool				iscopy			= false;

				if (nodeInfo.find("technique") != nodeInfo.end())
					technique = Any::cast<bool&>(nodeInfo["technique"]);

				if (nodeInfo.find("copyId") != nodeInfo.end())
				{
					iscopy = true;
					copyId = Any::cast<int&>(nodeInfo["copyId"]);
				}
				else
				{
					geometryObject	= Any::cast<Qark::ByteArray&>(nodeInfo["geometry"]);
					copyId			= Any::cast<int&>(nodeInfo["geometryId"]);
				}
				if (nodeInfo.find("geometryName") != nodeInfo.end())
					geometryName = Any::cast<std::string&>(nodeInfo["geometryName"]);


				std::vector<Any>&	bindingsId	= Any::cast<std::vector<Any>&>(nodeInfo["bindingsIds"]);
				int					materialId	= Any::cast<int&>(bindingsId[0]);

				bool computeTangent = false;
				if (options->deserializedAssets()->material(materialId)->hasProperty("material.normalMap"))
					computeTangent = true;

				GeometryDeserializer::deserializeGeometry(
                                                          iscopy,
                                                          geometryName,
                                                          copyId,
                                                          geometryObject,
                                                          options->assetLibrary(),
                                                          mesh,
                                                          options->parseOptions(),
                                                          computeTangent,
                                                          options->deserializedAssets()->material(materialId),
                                                          options->parseOptions()->effect());

				return mesh;
			}

			inline static
			std::shared_ptr<scene::Node>
			deserializeLight(NodeInfo&		nodeInfo,
							 OptionsPtr		options,
							 ControllerMap&	controllerMap,
							 NodeMap&		nodeMap)
			{
				std::shared_ptr<scene::Node> light = scene::Node::create(extractName(nodeInfo));

				return light;
			}

			inline static
			std::shared_ptr<scene::Node>
			deserializeCamera(NodeInfo&		    nodeInfo,
							  OptionsPtr	    options,
							  ControllerMap&	controllerMap,
							  NodeMap&		    nodeMap)
			{
				std::shared_ptr<scene::Node>		camera			= scene::Node::create(extractName(nodeInfo));
				std::shared_ptr<math::Matrix4x4>	transformMatrix = TypeDeserializer::matrix4x4(nodeInfo["transform"]);

				camera->addComponent(component::Transform::create());
				camera->component<component::Transform>()->transform()->copyFrom(transformMatrix);
				camera->component<component::Transform>()->transform()->prependRotationY(PI); // otherwise the camera points the other way
				return camera;
			}


		};
	}
}
