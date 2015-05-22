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

#pragma once

#include "minko/Common.hpp"
#include "minko/material/PhongMaterial.hpp"

namespace minko
{
    namespace material
    {
        class WaterMaterial :
            public PhongMaterial
        {
        public:
            typedef std::shared_ptr<WaterMaterial> Ptr;

        private:
            uint                    _numWaves;
            std::vector<float>      _amplitudes;
            std::vector<float>      _waveLength;
            std::vector<math::vec2> _origins;
            std::vector<float>      _speeds;
            std::vector<float>      _sharpness;
            std::vector<int>        _waveType;

        public:
            inline static
            Ptr
            create(uint numWaves)
            {
                Ptr ptr = std::shared_ptr<WaterMaterial>(new WaterMaterial(numWaves));

                ptr->initialize();

                return ptr;
            }

            Ptr
            setDirection(int waveId, const math::vec2& direction);

            Ptr
            setCenter(int waveId, const math::vec2& origin);

            Ptr
            setAmplitude(int waveId, float amplitude);

            Ptr
            setWaveLength(int waveId, float waveLength);

            Ptr
            setSharpness(int waveId, float sharpness);

            Ptr
            setSpeed(int waveId, float speed);

            // Ptr
            // dudvMap(AbsTexturePtr);
            //
            // TexturePtr
            // dudvMap() const;
            //
            // Ptr
            // dudvFactor(float);
            //
            // float
            // dudvFactor() const;
            //
            // Ptr
            // dudvSpeed(float);
            //
            // float
            // dudvSpeed() const;

            // Ptr
            // depthMap(AbsTexturePtr);
            //
            // TexturePtr
            // depthMap() const;
            //
            // Ptr
            // reflectionMap(AbsTexturePtr);
            //
            // TexturePtr
            // reflectionMap() const;
            //
            // Ptr
            // flowMap(AbsTexturePtr);
            //
            // TexturePtr
            // flowMap() const;
            //
            // Ptr
            // flowMapScale(float);
            //
            // float
            // flowMapScale() const;
            //
            // Ptr
            // flowMapCycle(float);
            //
            // float
            // flowMapCycle() const;
            //
            // Ptr
            // flowMapOffset1(float);
            //
            // float
            // flowMapOffset1() const;
            //
            // Ptr
            // flowMapOffset2(float);
            //
            // float
            // flowMapOffset2() const;
            //
            // Ptr
            // noiseMap(AbsTexturePtr);
            //
            // TexturePtr
            // noiseMap() const;
            //
            // Ptr
            // reflectivity(float);
            //
            // float
            // reflectivity() const;

        private:
            WaterMaterial(uint numWaves);

            void
            initialize();

            template <typename T>
            void
            setWaveProperty(const std::string& propertyName, int waveId, T value)
            {
                std::vector<T>& values = *data()->getUnsafePointer<std::vector<T>>(propertyName);

                values[waveId] = value;
            }
        };
    }
}
