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

#include "minko/data/Provider.hpp"

namespace minko
{
	namespace data
	{
		class ArrayProvider :
			public Provider
		{
		public:
			typedef std::shared_ptr<ArrayProvider> Ptr;

		private:
			std::string										_name;
			unsigned int									_index;
			std::unordered_map<std::string, std::string>	_propertyNameToArrayPropertyName;

		public:
			inline static
			Ptr
			create(const std::string& name, uint index = 0)
			{
				return std::shared_ptr<ArrayProvider>(new ArrayProvider(name, index));
			}

			inline
			const std::string&
			arrayName() const
			{
				return _name;
			}

			inline
			unsigned int
			index() const
			{
				return _index;
			}

			void
			index(unsigned int index);

		private:
			ArrayProvider(const std::string& name, uint index);

			std::string
			formatPropertyName(const std::string&) const;

			std::string
			unformatPropertyName(const std::string&) const;
		};
	}
}
