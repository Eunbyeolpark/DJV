// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvScene/IPrimitive.h>

#include <djvAV/TriangleMesh.h>

namespace djv
{
    namespace Scene
    {
        //! This class provides a mesh primitive.
        class MeshPrimitive : public IPrimitive
        {
            DJV_NON_COPYABLE(MeshPrimitive);

        protected:
            MeshPrimitive();

        public:
            static std::shared_ptr<MeshPrimitive> create();

            void addMesh(const std::shared_ptr<AV::Geom::TriangleMesh>&);

            std::string getClassName() const override;
            const std::vector<std::shared_ptr<AV::Geom::TriangleMesh> >& getMeshes() const override;
            size_t getPointCount() const override;

        private:
            std::vector<std::shared_ptr<AV::Geom::TriangleMesh> > _meshes;
            size_t _pointCount = 0;
        };

    } // namespace Scene
} // namespace djv

#include <djvScene/MeshPrimitiveInline.h>
