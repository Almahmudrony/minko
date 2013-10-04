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

namespace minko
{
	namespace render
	{
		class ProgramSignature
		{
		private:
			static const uint	MAX_NUM_BINDINGS;

			uint				_mask;
			std::vector<int>	_values;

		public:
			inline
			ProgramSignature():
				_mask(0),
				_values(MAX_NUM_BINDINGS, 0)
			{
			}
			
			void
			build(const data::BindingMap& macroBindings,
				  std::shared_ptr<data::Container> data,
				  std::shared_ptr<data::Container> rootData);

			bool 
			operator==(const ProgramSignature&) const;

			inline
			uint
			mask() const
			{
				return _mask;
			}

			inline
			const std::vector<int>&
			values() const
			{
				return _values;
			}
		};
	}
}

namespace std
{
	template <class T>
	inline 
	void 
	hash_combine(std::size_t & seed, const T& v)
	{
		std::hash<T> hasher;
		seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	template<> struct hash<minko::render::ProgramSignature>
	{
		inline
		size_t 
		operator()(const minko::render::ProgramSignature& x) const
		{
			size_t seed = std::hash<minko::uint>()(x.mask());

			for (unsigned int i=0; i < x.values().size(); ++i)
			{
				const int value = (x.mask() >> i) != 0 
					? x.values()[i]
					: 0;

				hash_combine(seed, value);
			}

			return seed;
		}
	};
}
