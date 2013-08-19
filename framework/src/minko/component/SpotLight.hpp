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

#include "minko/component/AbstractDiscreteLight.hpp"

namespace minko
{
	namespace component
	{
		class SpotLight :
			public AbstractDiscreteLight
		{
		public:
			typedef std::shared_ptr<SpotLight> Ptr;
	
		private:
			static uint						_counter;

			std::shared_ptr<math::Vector3>	_color;
			std::shared_ptr<math::Vector3>	_worldPosition;
			std::shared_ptr<math::Vector3>	_worldDirection;

		public:
			inline static
			Ptr
			create()
			{
				auto light = std::shared_ptr<SpotLight>(new SpotLight());

                light->initialize();

			    return light;
			}
	
		protected:
			void
            updateModelToWorldMatrix(std::shared_ptr<math::Matrix4x4> modelToWorld);

		private:
			SpotLight();
		};
	}
}
