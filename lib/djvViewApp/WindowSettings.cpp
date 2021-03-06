// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvViewApp/WindowSettings.h>

#include <djvCore/Context.h>
#include <djvCore/FileInfo.h>
#include <djvCore/ResourceSystem.h>
#include <djvCore/Vector.h>

// These need to be included last on OSX.
#include <djvCore/RapidJSONTemplates.h>
#include <djvUI/ISettingsTemplates.h>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        namespace
        {
            //! \todo Should this be configurable?
            const glm::ivec2 windowSizeDefault = glm::ivec2(1280, 720);
        
        } // namespace

        struct WindowSettings::Private
        {
            std::shared_ptr<ValueSubject<bool> > restorePos;
            std::shared_ptr<ValueSubject<bool> > restoreSize;
            glm::ivec2 windowPos = glm::ivec2(0, 0);
            glm::ivec2 windowSize = windowSizeDefault;
            std::shared_ptr<ValueSubject<bool> > fullScreen;
            std::shared_ptr<ValueSubject<int> > fullScreenMonitor;
            std::shared_ptr<ValueSubject<bool> > floatOnTop;
            std::shared_ptr<ValueSubject<bool> > maximize;
            std::shared_ptr<ValueSubject<bool> > autoHide;
            std::shared_ptr<ValueSubject<std::string> > backgroundImage;
            std::shared_ptr<ValueSubject<bool> > backgroundImageScale;
            std::shared_ptr<ValueSubject<bool> > backgroundImageColorize;
        };

        void WindowSettings::_init(const std::shared_ptr<Core::Context>& context)
        {
            ISettings::_init("djv::ViewApp::WindowSettings", context);

            DJV_PRIVATE_PTR();
            p.restorePos = ValueSubject<bool>::create(false);
            p.restoreSize = ValueSubject<bool>::create(true);
            p.fullScreen = ValueSubject<bool>::create(false);
            p.fullScreenMonitor = ValueSubject<int>::create(0);
            p.floatOnTop = ValueSubject<bool>::create(false);
            p.maximize = ValueSubject<bool>::create(true);
            p.autoHide = ValueSubject<bool>::create(true);
            auto resourceSystem = context->getSystemT<Core::ResourceSystem>();
            const auto& iconsPath = resourceSystem->getPath(FileSystem::ResourcePath::Icons);
            p.backgroundImage = ValueSubject<std::string>::create(std::string(
                FileSystem::Path(iconsPath, "djv-tshirt-v02.png")));
            p.backgroundImageScale = ValueSubject<bool>::create(false);
            p.backgroundImageColorize = ValueSubject<bool>::create(true);
            _load();
        }

        WindowSettings::WindowSettings() :
            _p(new Private)
        {}

        WindowSettings::~WindowSettings()
        {}

        std::shared_ptr<WindowSettings> WindowSettings::create(const std::shared_ptr<Core::Context>& context)
        {
            auto out = std::shared_ptr<WindowSettings>(new WindowSettings);
            out->_init(context);
            return out;
        }

        std::shared_ptr<Core::IValueSubject<bool> > WindowSettings::observeRestorePos() const
        {
            return _p->restorePos;
        }

        std::shared_ptr<Core::IValueSubject<bool> > WindowSettings::observeRestoreSize() const
        {
            return _p->restoreSize;
        }

        const glm::ivec2& WindowSettings::getWindowPos() const
        {
            return _p->windowPos;
        }

        const glm::ivec2& WindowSettings::getWindowSize() const
        {
            return _p->windowSize;
        }

        const glm::ivec2& WindowSettings::getWindowSizeDefault() const
        {
            return windowSizeDefault;
        }

        void WindowSettings::setRestorePos(bool value)
        {
            _p->restorePos->setIfChanged(value);
        }

        void WindowSettings::setRestoreSize(bool value)
        {
            _p->restoreSize->setIfChanged(value);
        }

        void WindowSettings::setWindowPos(const glm::ivec2& value)
        {
            _p->windowPos = value;
        }

        void WindowSettings::setWindowSize(const glm::ivec2& value)
        {
            _p->windowSize = value;
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeFullScreen() const
        {
            return _p->fullScreen;
        }

        std::shared_ptr<IValueSubject<int> > WindowSettings::observeFullScreenMonitor() const
        {
            return _p->fullScreenMonitor;
        }

        void WindowSettings::setFullScreen(bool value)
        {
            _p->fullScreen->setIfChanged(value);
        }

        void WindowSettings::setFullScreenMonitor(int value)
        {
            _p->fullScreenMonitor->setIfChanged(value);
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeFloatOnTop() const
        {
            return _p->floatOnTop;
        }

        void WindowSettings::setFloatOnTop(bool value)
        {
            _p->floatOnTop->setIfChanged(value);
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeMaximize() const
        {
            return _p->maximize;
        }

        void WindowSettings::setMaximize(bool value)
        {
            _p->maximize->setIfChanged(value);
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeAutoHide() const
        {
            return _p->autoHide;
        }

        void WindowSettings::setAutoHide(bool value)
        {
            _p->autoHide->setIfChanged(value);
        }

        std::shared_ptr<IValueSubject<std::string> > WindowSettings::observeBackgroundImage() const
        {
            return _p->backgroundImage;
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeBackgroundImageScale() const
        {
            return _p->backgroundImageScale;
        }

        std::shared_ptr<IValueSubject<bool> > WindowSettings::observeBackgroundImageColorize() const
        {
            return _p->backgroundImageColorize;
        }

        void WindowSettings::setBackgroundImageScale(bool value)
        {
            _p->backgroundImageScale->setIfChanged(value);
        }

        void WindowSettings::setBackgroundImageColorize(bool value)
        {
            _p->backgroundImageColorize->setIfChanged(value);
        }

        void WindowSettings::setBackgroundImage(const std::string& value)
        {
            _p->backgroundImage->setIfChanged(value);
        }

        void WindowSettings::load(const rapidjson::Value & value)
        {
            if (value.IsObject())
            {
                DJV_PRIVATE_PTR();
                UI::Settings::read("RestorePos", value, p.restorePos);
                UI::Settings::read("RestoreSize", value, p.restoreSize);
                UI::Settings::read("WindowPos", value, p.windowPos);
                UI::Settings::read("WindowSize", value, p.windowSize);
                UI::Settings::read("FullScreen", value, p.fullScreen);
                UI::Settings::read("FullScreenMonitor", value, p.fullScreenMonitor);
                UI::Settings::read("FloatOnTop", value, p.floatOnTop);
                UI::Settings::read("Maximize", value, p.maximize);
                UI::Settings::read("AutoHide", value, p.autoHide);
                UI::Settings::read("BackgroundImage", value, p.backgroundImage);
                UI::Settings::read("BackgroundImageScale", value, p.backgroundImageScale);
                UI::Settings::read("BackgroundImageColorize", value, p.backgroundImageColorize);
            }
        }

        rapidjson::Value WindowSettings::save(rapidjson::Document::AllocatorType& allocator)
        {
            DJV_PRIVATE_PTR();
            rapidjson::Value out(rapidjson::kObjectType);
            UI::Settings::write("RestorePos", p.restorePos->get(), out, allocator);
            UI::Settings::write("RestoreSize", p.restoreSize->get(), out, allocator);
            UI::Settings::write("WindowPos", p.windowPos, out, allocator);
            UI::Settings::write("WindowSize", p.windowSize, out, allocator);
            UI::Settings::write("FullScreen", p.fullScreen->get(), out, allocator);
            UI::Settings::write("FullScreenMonitor", p.fullScreenMonitor->get(), out, allocator);
            UI::Settings::write("FloatOnTop", p.floatOnTop->get(), out, allocator);
            UI::Settings::write("Maximize", p.maximize->get(), out, allocator);
            UI::Settings::write("AutoHide", p.autoHide->get(), out, allocator);
            UI::Settings::write("BackgroundImage", p.backgroundImage->get(), out, allocator);
            UI::Settings::write("BackgroundImageScale", p.backgroundImageScale->get(), out, allocator);
            UI::Settings::write("BackgroundImageColorize", p.backgroundImageColorize->get(), out, allocator);
            return out;
        }

    } // namespace ViewApp
} // namespace djv

