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

#include "minko/render/OpenGLES2Context.hpp"

namespace minko
{
	namespace render
	{
		class WebGLContext :
			public OpenGLES2Context
		{
		public:
			typedef std::shared_ptr<WebGLContext> Ptr;

		public:
			~WebGLContext();

			static
			Ptr
			create()
			{
				return std::shared_ptr<WebGLContext>(new WebGLContext());
			}

			void
			setShaderSource(const unsigned int shader,
							const std::string& source);

		protected:
			WebGLContext();

			void
			fillUniformInputs(const unsigned int				program,
							  std::vector<std::string>&			names,
							  std::vector<ProgramInputs::Type>&	types,
							  std::vector<unsigned int>&		locations);

			void
			fillAttributeInputs(const unsigned int					program,
								std::vector<std::string>&			names,
								std::vector<ProgramInputs::Type>&	types,
								std::vector<unsigned int>&			locations);

			void
			setUniformMatrix4x4(unsigned int	location,
								unsigned int	size,
								bool			transpose,
								const float*	values);

			void
			uploadVertexBufferData(const unsigned int	vertexBuffer,
									 const unsigned int offset,
									 const unsigned int size,
									 void* 				data);

			void
			uploaderIndexBufferData(const unsigned int 		indexBuffer,
									  const unsigned int 	offset,
									  const unsigned int 	size,
									  void*					data);

		};
	}
}
