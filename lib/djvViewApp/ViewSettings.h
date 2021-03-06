// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvViewApp/Enum.h>

#include <djvUI/ISettings.h>

#include <djvCore/BBox.h>
#include <djvCore/ValueObserver.h>

#include <map>

namespace djv
{
    namespace ViewApp
    {
        struct GridOptions;
        struct HUDOptions;
        struct ViewBackgroundOptions;

        //! This class provides the view settings.
        class ViewSettings : public UI::Settings::ISettings
        {
            DJV_NON_COPYABLE(ViewSettings);

        protected:
            void _init(const std::shared_ptr<Core::Context>& context);

            ViewSettings();

        public:
            static std::shared_ptr<ViewSettings> create(const std::shared_ptr<Core::Context>&);

            const std::map<std::string, bool> getBellowsState() const;
            void setBellowsState(const std::map<std::string, bool>&);

            std::shared_ptr<Core::IValueSubject<ViewLock> > observeLock() const;
            void setLock(ViewLock);

            std::shared_ptr<Core::IValueSubject<GridOptions> > observeGridOptions() const;
            void setGridOptions(const GridOptions&);

            std::shared_ptr<Core::IValueSubject<HUDOptions> > observeHUDOptions() const;
            void setHUDOptions(const HUDOptions&);

            std::shared_ptr<Core::IValueSubject<ViewBackgroundOptions> > observeBackgroundOptions() const;
            void setBackgroundOptions(const ViewBackgroundOptions&);

            const std::map<std::string, Core::BBox2f>& getWidgetGeom() const;
            void setWidgetGeom(const std::map<std::string, Core::BBox2f>&);

            void load(const rapidjson::Value &) override;
            rapidjson::Value save(rapidjson::Document::AllocatorType&) override;

        private:
            DJV_PRIVATE();
        };

    } // namespace ViewApp
} // namespace djv

