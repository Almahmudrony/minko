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

#include "minko/render/AbstractContext.hpp"
#include "minko/render/ProgramInputs.hpp"
#include "minko/render/Blending.hpp"

namespace minko
{
	namespace render
	{
		class OpenGLES2Context :
			public AbstractContext,
			public std::enable_shared_from_this<OpenGLES2Context>
		{
		public:
			typedef std::shared_ptr<OpenGLES2Context> Ptr;

        private:
            typedef std::unordered_map<unsigned int, unsigned int>	BlendFactorsMap;
			typedef std::unordered_map<CompareMode, unsigned int>	DepthFuncsMap;
            typedef std::unordered_map<unsigned int, unsigned int>  TextureToBufferMap;
			typedef std::pair<uint, uint>							TextureSize;

		protected:
			bool									_errorsEnabled;

	        static BlendFactorsMap					_blendingFactors;
			static DepthFuncsMap					_depthFuncs;

			std::list<unsigned int>	                _textures;
            std::unordered_map<uint, TextureSize>	_textureSizes;
            std::unordered_map<uint, bool>          _textureHasMipmaps;

            std::string                             _driverInfo;

			std::list<unsigned int>	                _vertexBuffers;
			std::list<unsigned int>	                _indexBuffers;
			std::list<unsigned int>                 _programs;
			std::list<unsigned int>                 _vertexShaders;
			std::list<unsigned int>                 _fragmentShaders;

            TextureToBufferMap                      _frameBuffers;
            TextureToBufferMap                      _renderBuffers;

			unsigned int			                _viewportX;
			unsigned int			                _viewportY;
			unsigned int			                _viewportWidth;
			unsigned int			                _viewportHeight;

            unsigned int                            _currentTarget;
			int						                _currentIndexBuffer;
			std::vector<int>		                _currentVertexBuffer;
			std::vector<int>		                _currentVertexSize;
			std::vector<int>		                _currentVertexStride;
			std::vector<int>		                _currentVertexOffset;
			std::vector<int>		                _currentTexture;
            std::unordered_map<uint, WrapMode>      _currentWrapMode;
            std::unordered_map<uint, TextureFilter> _currentTextureFilter;
            std::unordered_map<uint, MipFilter>     _currentMipFilter;
			int						                _currentProgram;
			Blending::Mode			                _currentBlendMode;
			bool					                _currentDepthMask;
			CompareMode				                _currentDepthFunc;
            TriangleCulling                         _currentTriangleCulling;

		public:
			~OpenGLES2Context();

			static
			Ptr
			create()
			{
				return std::shared_ptr<OpenGLES2Context>(new OpenGLES2Context());
			}

			inline
			bool
			errorsEnabled()
			{
				return _errorsEnabled;
			}

			inline
			void
			errorsEnabled(bool errorsEnabled)
			{
				_errorsEnabled = errorsEnabled;
			}
			
            inline
            const std::string&
            driverInfo()
            {
                return _driverInfo;
            }

			void
			configureViewport(const unsigned int x,
							  const unsigned int y,
							  const unsigned int with,
							  const unsigned int height);

			void
			clear(float red 			= 0.f,
				  float green			= 0.f,
				  float blue			= 0.f,
				  float alpha			= 0.f,
				  float depth			= 1.f,
				  unsigned int stencil	= 0,
				  unsigned int mask		= 0xffffffff);

			void
			present();

			void
			drawTriangles(const unsigned int indexBuffer, const int numTriangles);

			const unsigned int
			createVertexBuffer(const unsigned int size);

			void
			setVertexBufferAt(const unsigned int	position,
							  const unsigned int	vertexBuffer,
							  const unsigned int	size,
							  const unsigned int	stride,
							  const unsigned int	offset);
			void
			uploadVertexBufferData(const unsigned int 	vertexBuffer,
								   const unsigned int 	offset,
								   const unsigned int 	size,
								   void* 				data);

			void
			deleteVertexBuffer(const unsigned int vertexBuffer);

			const unsigned int
			createIndexBuffer(const unsigned int size);

			void
			uploaderIndexBufferData(const unsigned int 	indexBuffer,
									const unsigned int 	offset,
									const unsigned int 	size,
									void*				data);

			void
			deleteIndexBuffer(const unsigned int indexBuffer);

			const unsigned int
			createTexture(unsigned int  width,
						  unsigned int  height,
						  bool		    mipMapping,
                          bool          optimizeForRenderToTexture = false);

			void
			uploadTextureData(const unsigned int texture,
							  unsigned int 		 width,
							  unsigned int 		 height,
							  unsigned int 		 mipLevel,
							  void*				 data);

			void
			deleteTexture(const unsigned int texture);

			void
			setTextureAt(const unsigned int	position,
						 const int			texture		= 0,
						 const int			location	= -1);

            void
            setSamplerStateAt(const unsigned int    position,
                              WrapMode              wrapping,
                              TextureFilter         filtering,
                              MipFilter             mipFiltering);

			const unsigned int
			createProgram();

			void
			attachShader(const unsigned int program, const unsigned int shader);

			void
			linkProgram(const unsigned int program);

			void
			deleteProgram(const unsigned int program);

			void
			compileShader(const unsigned int shader);

			void
			setProgram(const unsigned int program);

			void
			setShaderSource(const unsigned int shader, const std::string& source);

			const unsigned int
			createVertexShader();

			void
			deleteVertexShader(const unsigned int vertexShader);

			const unsigned int
			createFragmentShader();

			void
			deleteFragmentShader(const unsigned int fragmentShader);

			std::shared_ptr<ProgramInputs>
			getProgramInputs(const unsigned int program);

			std::string
			getShaderCompilationLogs(const unsigned int shader);

			std::string
			getProgramInfoLogs(const unsigned int program);

			void
			setUniform(unsigned int location, float value);

			void
			setUniform(unsigned int location, float value1, float value2);

			void
			setUniform(unsigned int location, float value1, float value2, float value3);

			void
			setUniform(unsigned int location, float value1, float value2, float value3, float value4);

			void
			setUniform(unsigned int location, unsigned int size, bool transpose, const float* values);

            void
            setBlendMode(Blending::Source source, Blending::Destination destination);

            void
            setBlendMode(Blending::Mode blendMode);

			void
			setDepthTest(bool depthMask, CompareMode depthFunc);

			void
			readPixels(unsigned char* pixels);

            void
            setTriangleCulling(TriangleCulling triangleCulling);

            void
            setRenderToBackBuffer();

            void
            setRenderToTexture(unsigned int texture, bool enableDepthAndStencil = false);

            void
            generateMipmaps(unsigned int texture);

		protected:
			OpenGLES2Context();

			virtual
			void
			fillUniformInputs(const unsigned int				program,
							  std::vector<std::string>&			names,
							  std::vector<ProgramInputs::Type>&	types,
							  std::vector<unsigned int>&		locations);

			virtual
			void
			fillAttributeInputs(const unsigned int					program,
								std::vector<std::string>&			names,
								std::vector<ProgramInputs::Type>&	types,
								std::vector<unsigned int>&			locations);

            static
            BlendFactorsMap
            initializeBlendFactorsMap();

			static
            DepthFuncsMap
            initializeDepthFuncsMap();

            void
            createRTTBuffers(unsigned int texture, unsigned int width, unsigned int height);

            inline
            void
            checkForErrors()
            {
#ifdef DEBUG
				if (_errorsEnabled && getError() != 0)
					throw;
#endif
            }

            unsigned int
            getError();
		};
	}
}
