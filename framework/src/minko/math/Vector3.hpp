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
#include "minko/math/Vector2.hpp"

namespace minko
{
	namespace math
	{
		class Vector3 :
			public Vector2
		{
		public:
			typedef std::shared_ptr<Vector3>	Ptr;
			typedef std::shared_ptr<Vector3>	ConstPtr;

		protected:
			float _z;

		public:
			inline static
			Ptr
			create(float x = 0.f, float y = 0.f, float z = 0.f)
			{
				return std::shared_ptr<Vector3>(new Vector3(x, y, z));
			}

			inline static
			Ptr
			create(Ptr v)
			{
				return std::shared_ptr<Vector3>(new Vector3(v->_x, v->_y, v->_z));
			}

			inline static
			ConstPtr
			createConst(float x = 0.f, float y = 0.f, float z = 0.f)
			{
				return std::shared_ptr<Vector3>(new Vector3(x, y, z));
			}

			inline static
			ConstPtr
			upAxis()
			{
				static ConstPtr upAxis = createConst(0.f, 1.f, 0.f);

				return upAxis;
			}

			inline static
			ConstPtr
			xAxis()
			{
				static ConstPtr xAxis = createConst(1.f, 0.f, 0.f);

				return xAxis;
			}

			inline static
			ConstPtr
			yAxis()
			{
				static ConstPtr yAxis = createConst(0.f, 1.f, 0.f);

				return yAxis;
			}

			inline static
			ConstPtr
			zAxis()
			{
				static ConstPtr zAxis = createConst(0.f, 0.f, 1.f);

				return zAxis;
			}

			inline static
			ConstPtr
			zero()
			{
				static ConstPtr zero = createConst(0.f, 0.f, 0.f);

				return zero;
			}

			inline static
			ConstPtr
			one()
			{
				static ConstPtr one = createConst(1.f, 1.f, 1.f);

				return one;
			}

			inline static
			ConstPtr
			forward()
			{
				static ConstPtr forward = createConst(0.f, 0.f, -1.f);

				return forward;
			}

			inline
			float
			z()
			{
				return _z;
			}

			inline
			void
			z(float z)
			{
				_z = z;
			}

            inline
			Ptr
			copyFrom(Ptr value)
			{
				return setTo(value->_x, value->_y, value->_z);
			}

			inline
			Ptr
			setTo(float x, float y, float z)
			{
				_x = x;
				_y = y;
				_z = z;

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			Ptr
			normalize()
			{
				float l = sqrtf(_x * _x + _y * _y + _z * _z);

				if (l != 0.)
				{
					_x /= l;
					_y /= l;
					_z /= l;
				}

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			Ptr
			cross(Ptr value)
			{
				float x = _y * value->_z - _z * value->_y;
				float y = _z * value->_x - _x * value->_z;
				float z = _x * value->_y - _y * value->_x;

				_x = x;
				_y = y;
				_z = z;

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			float
			dot(Ptr value)
			{
				return _x * value->_x + _y * value->_y + _z * value->_z;
			}

			std::string
			toString()
			{
				std::stringstream stream;

				stream << "(" << _x << ", " << _y << ", " << _z << ")";

				return stream.str();
			}


			inline
			Ptr
			operator-() const
			{
				return create(-_x, -_y, -_z);
			}

			inline
			Ptr
			operator-(Ptr value)
			{
				return create(_x - value->_x, _y - value->_y, _z - value->_z);
			}

			inline
			Ptr
			operator+(Ptr value)
			{
				return create(_x + value->_x, _y + value->_y, _z + value->_z);
			}

			inline
			Ptr
			operator+=(Ptr value)
			{
				_x += value->_x;
				_y += value->_y;
				_z += value->_z;

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			Ptr
			operator-=(Ptr value)
			{
				_x -= value->_x;
				_y -= value->_y;
				_z -= value->_z;

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			Ptr
			operator*(float value)
			{
				_x *= value;
				_y *= value;
				_z *= value;

				return std::static_pointer_cast<Vector3>(shared_from_this());
			}

			inline
			bool
			equals(Ptr v)
			{
				return _x == v->_x && _y == v->_y && _z == v->_z;
			}

			inline
			Ptr
			lerp(Ptr target, float ratio)
			{
				return setTo(
					_x + (target->_x - _x) * ratio,
					_y + (target->_y - _y) * ratio,
					_z + (target->_z - _z) * ratio
				);
			}

			inline
			float
			lengthSquared() const
			{
				return _x*_x + _y*_y + _z*_z;
			}

			inline
			float
			length() const
			{
				return sqrtf(lengthSquared());
			}

		protected:
			Vector3(float x, float y, float z) :
				Vector2(x, y),
				_z(z)
			{
			}
		};

	}
}
