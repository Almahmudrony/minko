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

#include "minko/file/ASSIMPParser.hpp"

namespace minko
{
	namespace geometry
	{
		class Bone;
		
		class Skin:
			public std::enable_shared_from_this<Skin>
		{
		public:
			typedef std::shared_ptr<Skin>					Ptr;

		private:
			typedef std::shared_ptr<Bone>					BonePtr;
			typedef std::shared_ptr<math::Matrix4x4>		Matrix4x4Ptr;
			typedef std::vector<Matrix4x4Ptr>				Matrices4x4Ptr;

		private:
			const unsigned int				_numBones;
			std::vector<BonePtr>			_bones;

			float							_duration;				// in seconds
			float							_timeFactor;
			std::vector<Matrices4x4Ptr>		_boneMatricesPerFrame;

			unsigned int					_maxNumVertexBones;
			std::vector<unsigned int>		_numVertexBones;		// size = #vertices
			std::vector<unsigned int>		_vertexBones;			// size = #vertices * #bones      
			std::vector<float>				_vertexBoneWeights;		// size = #vertices * #bones   
			
		public:
			inline
			static
			Ptr
			create(unsigned int numBones, unsigned int numFrames)
			{
				return std::shared_ptr<Skin>(new Skin(numBones, numFrames));
			}

			inline
			unsigned int
			numBones() const
			{
				return _numBones;
			}

			inline
			unsigned int
			maxNumVertexBones() const
			{
				return _maxNumVertexBones;
			}

			inline
			std::vector<BonePtr>&
			bones()
			{
				return _bones;
			}

			inline
			BonePtr
			bone(unsigned int boneId) const
			{
				return _bones[boneId];
			}

			inline
			void
			bone(unsigned int boneId, BonePtr value)
			{
				_bones[boneId] = value;
			}

			inline
			float
			duration() const
			{
				return _duration;
			}

			unsigned int
			getFrameId(float) const;

			void
			duration(float);

			inline
			unsigned int
			numFrames() const
			{
				return _boneMatricesPerFrame.size();
			}

			inline
			const std::vector<Matrix4x4Ptr>&
			matrices(unsigned int frameId) const
			{
				return _boneMatricesPerFrame[frameId];
			}

			void
			matrix(unsigned int frameId, unsigned int boneId, Matrix4x4Ptr value)
			{
				_boneMatricesPerFrame[frameId][boneId] = value;
			}

			Matrix4x4Ptr
			matrix(unsigned int frameId, unsigned int boneId) const
			{
				return _boneMatricesPerFrame[frameId][boneId];
			}

			inline
			unsigned int
			numVertices() const
			{
				return _numVertexBones.size();
			}

			inline
			unsigned int
			numVertexBones(unsigned int vertexId) const
			{
#ifdef DEBUG_SKINNING
				assert(vertexId < numVertices());
#endif // DEBUG_SKINNING

				return _numVertexBones[vertexId];
			}

			void
			vertexBoneData(unsigned int vertexId, unsigned int j, unsigned int& boneId, float& boneWeight) const;

			unsigned int
			vertexBoneId(unsigned int vertexId, unsigned int j) const;

			float 
			vertexBoneWeight(unsigned int vertexId, unsigned int j) const;

			Ptr
			reorganizeByVertices();

			Ptr
			disposeBones();

		private:
			Skin(unsigned int numBones, unsigned int numFrames);

			void
			clear();
			
			unsigned short
			lastVertexId() const;

			inline
			unsigned int
			vertexArraysIndex(unsigned int vertexId, unsigned int j) const
			{
#ifdef DEBUG_SKINNING
				assert(vertexId < numVertices() && j < numVertexBones(vertexId));
#endif // DEBUG_SKINNING

				return j + _numBones * vertexId;
			}
		};
	}
}
