//------------------------------------------------------------------------------
// Copyright (c) 2019 Darby Johnston
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions, and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions, and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the names of the copyright holders nor the names of any
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//------------------------------------------------------------------------------

#pragma once

#include <djvAV/ImageData.h>

#include <djvCore/ISystem.h>

#include <glm/mat4x4.hpp>

namespace djv
{
    namespace AV
    {
        namespace Geom
        {
            struct TriangleMesh;
        
        } // namespace Geom

        namespace OpenGL
        {
            class Shader;

        } // namespace OpenGL

        namespace Render3D
        {
            //! This struct provides camera information
            struct Camera
            {
                glm::mat4x4 v;
                glm::mat4x4 p;
                float fov = 45.F;
            };

            //! This class provides the base functionality for materials.
            class IMaterial : public std::enable_shared_from_this<IMaterial>
            {
                DJV_NON_COPYABLE(IMaterial);
                
            protected:
                void _init(
                    const std::string& vertex,
                    const std::string& fragment,
                    const std::shared_ptr<Core::Context>&);

                IMaterial();

            public:
                virtual ~IMaterial() = 0;

                virtual void bind(const glm::mat4x4&);

            protected:
                std::shared_ptr<OpenGL::Shader> _shader;
                GLint _mvpLoc = 0;
            };

            //! This class provides a default material.
            class DefaultMaterial : public IMaterial
            {
                DJV_NON_COPYABLE(DefaultMaterial);

            protected:
                void _init(const std::shared_ptr<Core::Context>&);

                DefaultMaterial();

            public:
                ~DefaultMaterial() override;

                static std::shared_ptr<DefaultMaterial> create(const std::shared_ptr<Core::Context>&);

                void bind(const glm::mat4x4&) override;

            private:
            };

            //! This struct provides render options.
            struct RenderOptions
            {
                Camera camera;
                Image::Size size;
            };

            //! This class provides a 3D render system.
            class Render : public Core::ISystem
            {
                DJV_NON_COPYABLE(Render);

            protected:
                void _init(const std::shared_ptr<Core::Context>&);
                Render();

            public:
                ~Render();

                static std::shared_ptr<Render> create(const std::shared_ptr<Core::Context>&);

                //! \name Begin and End
                ///@{

                void beginFrame(const RenderOptions&);
                void endFrame();

                ///@}

                //! \name Transform
                ///@{

                void pushTransform(const glm::mat4x4&);
                void popTransform();

                ///@}

                //! \name Material
                ///@{

                void setMaterial(const std::shared_ptr<IMaterial>&);

                ///@}

                //! \name Primitives
                ///@{

                void drawTriangleMesh(const Geom::TriangleMesh&);

                ///@}

            private:
                void _updateCurrentTransform();
                void _updateCurrentClipRect();

                std::list<glm::mat4x4>  _transforms;
                glm::mat4x4             _currentTransform = glm::mat4x4(1.F);

                DJV_PRIVATE();
            };

        } // namespace Render3D
    } // namespace AV
} // namespace djv

#include <djvAV/Render3DInline.h>