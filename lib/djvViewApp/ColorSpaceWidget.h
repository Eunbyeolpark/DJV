// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvViewApp/MDIWidget.h>

namespace djv
{
    namespace ViewApp
    {
        //! This class provides the color space widget.
        class ColorSpaceWidget : public MDIWidget
        {
            DJV_NON_COPYABLE(ColorSpaceWidget);

        protected:
            void _init(const std::shared_ptr<Core::Context>&);
            ColorSpaceWidget();

        public:
            ~ColorSpaceWidget() override;

            static std::shared_ptr<ColorSpaceWidget> create(const std::shared_ptr<Core::Context>&);

            std::map<std::string, bool> getBellowsState() const;
            void setBellowsState(const std::map<std::string, bool>&);

        protected:
            void _initLayoutEvent(djv::Core::Event::InitLayout&) override;

            void _initEvent(Core::Event::Init &) override;

        private:
            void _widgetUpdate();
            void _imageListUpdate();

            DJV_PRIVATE();
        };

    } // namespace ViewApp
} // namespace djv

