// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvViewApp/Application.h>

#include <djvViewApp/ApplicationSettings.h>
#include <djvViewApp/AnnotateSystem.h>
#include <djvViewApp/AudioSystem.h>
#include <djvViewApp/ColorPickerSystem.h>
#include <djvViewApp/FileSettings.h>
#include <djvViewApp/FileSystem.h>
#include <djvViewApp/HelpSystem.h>
#include <djvViewApp/ImageSystem.h>
#include <djvViewApp/InputSystem.h>
#include <djvViewApp/MagnifySystem.h>
#include <djvViewApp/MainWindow.h>
#include <djvViewApp/Media.h>
#include <djvViewApp/NUXSystem.h>
#include <djvViewApp/PlaybackSystem.h>
#include <djvViewApp/SettingsSystem.h>
#include <djvViewApp/ToolSystem.h>
#include <djvViewApp/ViewSystem.h>
#include <djvViewApp/WindowSettings.h>
#include <djvViewApp/WindowSystem.h>

#include <djvUIComponents/UIComponentsSystem.h>

#include <djvUI/SettingsSystem.h>

#include <djvAV/AVSystem.h>
#include <djvAV/GLFWSystem.h>
#include <djvAV/IO.h>
#include <djvAV/OCIOSystem.h>
#include <djvAV/Render2D.h>

#include <djvCore/LogSystem.h>
#include <djvCore/ResourceSystem.h>
#include <djvCore/StringFormat.h>
#include <djvCore/TextSystem.h>
#include <djvCore/Timer.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        struct Application::Private
        {
            std::vector<std::shared_ptr<ISystem> > systems;

            CmdLineMode cmdLineMode = CmdLineMode::DJV;
            std::shared_ptr<bool> fullScreenCmdLine;
            std::shared_ptr<int> fullScreenMonitorCmdLine;
            std::shared_ptr<std::string> ocioConfigCmdLine;
            std::shared_ptr<std::string> ocioDisplayCmdLine;
            std::shared_ptr<std::string> ocioViewCmdLine;
            std::shared_ptr<std::string> ocioImageCmdLine;
            std::shared_ptr<Time::Speed> speedCmdLine;
            std::shared_ptr<Core::Frame::Range> startEndMayaCmdLine;
            std::shared_ptr<std::string> inPointCmdLine;
            std::shared_ptr<std::string> outPointCmdLine;
            std::shared_ptr<std::string> frameCmdLine;

            std::shared_ptr<ApplicationSettings> settings;

            std::vector<std::string> cmdlinePaths;

            std::vector<std::shared_ptr<AV::IO::IRead> > read;
            std::vector<std::shared_ptr<AV::Image::Image> > icons;
            std::shared_ptr<Time::Timer> timer;

            std::shared_ptr<MainWindow> mainWindow;
            std::shared_ptr<NUXWidget> nuxWidget;
        };

        namespace
        {
            class CmdLineLocale
            {
            public:
                CmdLineLocale(const std::shared_ptr<TextSystem>& textSystem) :
                    _textSystem(textSystem)
                {
                    _currentLocale = _textSystem->observeCurrentLocale()->get();
                    _textSystem->setCurrentLocale("en");
                }

                ~CmdLineLocale()
                {
                    _textSystem->setCurrentLocale(_currentLocale);
                }

            private:
                std::shared_ptr<TextSystem> _textSystem;
                std::string _currentLocale;
            };

        } // namespace
        
        void Application::_init(std::list<std::string>& args)
        {
            Desktop::Application::_init(args);
            DJV_PRIVATE_PTR();

            // Create the systems.
            UI::UIComponentsSystem::create(shared_from_this());
            p.systems.push_back(InputSystem::create(shared_from_this()));
            p.systems.push_back(FileSystem::create(shared_from_this()));
            p.systems.push_back(WindowSystem::create(shared_from_this()));
            p.systems.push_back(ViewSystem::create(shared_from_this()));
            p.systems.push_back(ImageSystem::create(shared_from_this()));
            p.systems.push_back(PlaybackSystem::create(shared_from_this()));
            p.systems.push_back(AudioSystem::create(shared_from_this()));
            p.systems.push_back(ColorPickerSystem::create(shared_from_this()));
            p.systems.push_back(MagnifySystem::create(shared_from_this()));
            //p.systems.push_back(AnnotateSystem::create(shared_from_this()));
            p.systems.push_back(ToolSystem::create(shared_from_this()));
            p.systems.push_back(HelpSystem::create(shared_from_this()));
            p.systems.push_back(NUXSystem::create(shared_from_this()));
            p.systems.push_back(SettingsSystem::create(shared_from_this()));

            // Create settings.
            p.settings = ApplicationSettings::create(shared_from_this());

            // Parse the command-line.
            auto arg = args.begin();
            while (arg != args.end())
            {
                if ("-maya" == *arg)
                {
                    arg = args.erase(arg);
                    p.cmdLineMode = CmdLineMode::Maya;
                    break;
                }
                ++arg;
            }
            auto textSystem = getSystemT<Core::TextSystem>();
            CmdLineLocale cmdLineLocale(textSystem);
            _parseCmdLine(args);
        }

        Application::Application() :
            _p(new Private)
        {}

        Application::~Application()
        {}

        std::shared_ptr<Application> Application::create(std::list<std::string>& args)
        {
            auto out = std::shared_ptr<Application>(new Application);
            out->_init(args);
            return out;
        }

        void Application::run()
        {
            DJV_PRIVATE_PTR();

            // Create the main window.
            p.mainWindow = MainWindow::create(shared_from_this());
            auto windowSystem = getSystemT<WindowSystem>();
            windowSystem->setMediaCanvas(p.mainWindow->getMediaCanvas());

            // NUX.
            auto nuxSystem = getSystemT<NUXSystem>();
            p.nuxWidget = nuxSystem->createNUXWidget();
            if (p.nuxWidget)
            {
                p.nuxWidget->show();
                auto weak = std::weak_ptr<Application>(std::dynamic_pointer_cast<Application>(shared_from_this()));
                p.nuxWidget->setFinishCallback(
                    [weak]
                    {
                        if (auto app = weak.lock())
                        {
                            app->_p->nuxWidget->close();
                            app->_p->nuxWidget.reset();
                        }
                    });
            }

            // Read the application icons.
            _readIcon("djv-reel-16.png");
            _readIcon("djv-reel-32.png");
            _readIcon("djv-reel-64.png");
            _readIcon("djv-reel-128.png");
            _readIcon("djv-reel-512.png");
            _readIcon("djv-reel-1024.png");
            p.timer = Time::Timer::create(shared_from_this());
            p.timer->setRepeating(true);
            p.timer->start(
                Time::getTime(Time::TimerValue::Fast),
                [this](const std::chrono::steady_clock::time_point&, const Time::Duration&)
                {
                    DJV_PRIVATE_PTR();
                    auto i = p.read.begin();
                    while (i != p.read.end())
                    {
                        bool erase = false;
                        {
                            std::unique_lock<std::mutex> lock((*i)->getMutex());
                            auto& queue = (*i)->getVideoQueue();
                            if (!queue.isEmpty())
                            {
                                erase = true;
                                p.icons.push_back(queue.popFrame().image);
                            }
                            else if (queue.isFinished())
                            {
                                erase = true;
                            }
                        }
                        if (erase)
                        {
                            i = p.read.erase(i);
                        }
                        else
                        {
                            ++i;
                        }
                    }
                    if (!p.read.size())
                    {
                        p.timer->stop();
                        auto glfwSystem = getSystemT<AV::GLFW::System>();
                        auto glfwWindow = glfwSystem->getGLFWWindow();
                        std::vector<GLFWimage> glfwImages;
                        for (const auto& i : p.icons)
                        {
                            glfwImages.push_back(GLFWimage{ i->getWidth(), i->getHeight(), i->getData() });
                        }
                        glfwSetWindowIcon(glfwWindow, glfwImages.size(), glfwImages.data());
                    }
                });

            // Open command-line files.
            auto fileSystem = getSystemT<FileSystem>();
            OpenOptions openOptions;
            openOptions.speed = p.speedCmdLine;
            openOptions.inPoint = p.inPointCmdLine;
            openOptions.outPoint = p.outPointCmdLine;
            openOptions.frame = p.frameCmdLine;
            fileSystem->open(p.cmdlinePaths, openOptions);

            // Apply command-line arguments.
            if (p.fullScreenMonitorCmdLine && windowSystem)
            {
                auto settingsSystem = getSystemT<UI::Settings::System>();
                auto windowSettings = settingsSystem->getSettingsT<WindowSettings>();
                windowSettings->setFullScreenMonitor(*(p.fullScreenMonitorCmdLine));
            }
            if (p.fullScreenCmdLine && windowSystem)
            {
                windowSystem->setFullScreen(*(p.fullScreenCmdLine));
            }
            if (p.ocioConfigCmdLine)
            {
                auto ocioSystem = getSystemT<AV::OCIO::System>();
                ocioSystem->addConfig(*p.ocioConfigCmdLine);
            }
            if (p.ocioDisplayCmdLine || p.ocioViewCmdLine || p.ocioImageCmdLine)
            {
                auto ocioSystem = getSystemT<AV::OCIO::System>();
                auto config = ocioSystem->observeCurrentConfig()->get();
                if (p.ocioDisplayCmdLine)
                {
                    config.display = *p.ocioDisplayCmdLine;
                }
                if (p.ocioViewCmdLine)
                {
                    config.view = *p.ocioViewCmdLine;
                }
                if (p.ocioImageCmdLine)
                {
                    config.fileColorSpaces[std::string()] = *p.ocioImageCmdLine;
                }
                ocioSystem->setCurrentConfig(config);
            }

            // Show the main window.
            p.mainWindow->show();

            Desktop::Application::run();
        }

        void Application::_parseCmdLine(std::list<std::string>& args)
        {
            Desktop::Application::_parseCmdLine(args);
            DJV_PRIVATE_PTR();
            if (0 == getExitCode())
            {
                switch (p.cmdLineMode)
                {
                case CmdLineMode::DJV: _parseCmdLineDJV(args); break;
                case CmdLineMode::Maya: _parseCmdLineMaya(args); break;
                default: break;
                }
            }
        }

        void Application::_parseCmdLineDJV(std::list<std::string>& args)
        {
            DJV_PRIVATE_PTR();
            auto textSystem = getSystemT<Core::TextSystem>();
            auto arg = args.begin();
            while (arg != args.end())
            {
                if ("-full_screen" == *arg)
                {
                    arg = args.erase(arg);
                    p.fullScreenCmdLine.reset(new bool(true));
                }
                else if ("-full_screen_monitor" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-full_screen_monitor").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    int value = 0;
                    std::stringstream ss(*arg);
                    ss >> value;
                    arg = args.erase(arg);
                    p.fullScreenMonitorCmdLine.reset(new int(std::max(value, 0)));
                }
                else if ("-ocio_config" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-ocio_config").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    p.ocioConfigCmdLine.reset(new std::string(*arg));
                    arg = args.erase(arg);
                }
                else if ("-ocio_display" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-ocio_display").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    if (textSystem->getText(DJV_TEXT("av_ocio_display_none")) == *arg)
                    {
                        p.ocioDisplayCmdLine.reset(new std::string());
                    }
                    else
                    {
                        p.ocioDisplayCmdLine.reset(new std::string(*arg));
                    }
                    arg = args.erase(arg);
                }
                else if ("-ocio_view" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-ocio_view").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    if (textSystem->getText(DJV_TEXT("av_ocio_view_none")) == *arg)
                    {
                        p.ocioViewCmdLine.reset(new std::string());
                    }
                    else
                    {
                        p.ocioViewCmdLine.reset(new std::string(*arg));
                    }
                    arg = args.erase(arg);
                }
                else if ("-ocio_image" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-ocio_image").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    if (textSystem->getText(DJV_TEXT("av_ocio_image_none")) == *arg)
                    {
                        p.ocioImageCmdLine.reset(new std::string());
                    }
                    else
                    {
                        p.ocioImageCmdLine.reset(new std::string(*arg));
                    }
                    arg = args.erase(arg);
                }
                else if ("-speed" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-speed").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    float value = 0.F;
                    std::stringstream ss(*arg);
                    ss >> value;
                    arg = args.erase(arg);
                    Time::Speed speed;
                    if (value >= 1.F)
                    {
                        speed = Time::Speed(value);
                    }
                    else if (value > 0.F && value < 1.F)
                    {
                        speed = Time::Speed(static_cast<int>(std::floor(value * 1000.F)), 1000);
                    }
                    p.speedCmdLine.reset(new Time::Speed(speed));
                }
                else if ("-in_out" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-in_out").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    p.inPointCmdLine.reset(new std::string(*arg));
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-in_out").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    p.outPointCmdLine.reset(new std::string(*arg));
                    arg = args.erase(arg);
                }
                else if ("-frame" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-frame").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    p.frameCmdLine.reset(new std::string(*arg));
                    arg = args.erase(arg);
                }
                else
                {
                    p.cmdlinePaths.push_back(*arg);
                    arg = args.erase(arg);
                }
            }
        }

        void Application::_parseCmdLineMaya(std::list<std::string>& args)
        {
            DJV_PRIVATE_PTR();
            auto textSystem = getSystemT<Core::TextSystem>();
            auto arg = args.begin();
            while (arg != args.end())
            {
                if ("-full_screen" == *arg)
                {
                    arg = args.erase(arg);
                    p.fullScreenCmdLine.reset(new bool(true));
                }
                else if ("-full_screen_monitor" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-full_screen_monitor").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    int value = 0;
                    std::stringstream ss(*arg);
                    ss >> value;
                    arg = args.erase(arg);
                    p.fullScreenMonitorCmdLine.reset(new int(std::max(value, 0)));
                }
                else if ("-r" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-r").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    float value = 0.F;
                    std::stringstream ss(*arg);
                    ss >> value;
                    arg = args.erase(arg);
                    Time::Speed speed;
                    if (value >= 1.F)
                    {
                        speed = Time::Speed(value);
                    }
                    else if (value > 0.F && value < 1.F)
                    {
                        speed = Time::Speed(static_cast<int>(std::floor(value * 1000.F)), 1000);
                    }
                    p.speedCmdLine.reset(new Time::Speed(speed));
                }
                else if ("-start_end" == *arg)
                {
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-start_end").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    Frame::Number min = 0;
                    Frame::Number max = 0;
                    {
                        std::stringstream ss(*arg);
                        ss >> min;
                    }
                    arg = args.erase(arg);
                    if (args.end() == arg)
                    {
                        throw std::runtime_error(String::Format("{0}: {1}").
                            arg("-start_end").
                            arg(textSystem->getText(DJV_TEXT("error_cannot_parse_argument"))));
                    }
                    {
                        std::stringstream ss(*arg);
                        ss >> max;
                    }
                    p.startEndMayaCmdLine.reset(new Core::Frame::Range(min, max));
                    arg = args.erase(arg);
                }
                else
                {
                    p.cmdlinePaths.push_back(*arg);
                    arg = args.erase(arg);
                }
            }
        }

        void Application::_printUsage()
        {
            DJV_PRIVATE_PTR();
            switch (p.cmdLineMode)
            {
            case CmdLineMode::DJV: _printUsageDJV(); break;
            case CmdLineMode::Maya: _printUsageMaya(); break;
            default: break;
            }
            Desktop::Application::_printUsage();
        }

        void Application::_printUsageDJV()
        {
            DJV_PRIVATE_PTR();
            auto textSystem = getSystemT<Core::TextSystem>();
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_usage")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_usage_format")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_usage_format_input_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_compatibility")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_maya")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_maya_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_window")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_monitor")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_monitor_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_ocio")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_config")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_config_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_display")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_display_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_view")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_view_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_image")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_ocio_image_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_playback")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_speed")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_speed_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_in_out_points")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_in_out_points_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_frame")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_frame_description")) << std::endl;
            std::cout << std::endl;
        }

        void Application::_printUsageMaya()
        {
            DJV_PRIVATE_PTR();
            auto textSystem = getSystemT<Core::TextSystem>();
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_usage")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_usage_format")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_usage_format_input_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_window")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_description")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_monitor")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_full_screen_monitor_description")) << std::endl;
            std::cout << std::endl;
            std::cout << " " << textSystem->getText(DJV_TEXT("djv_cli_options_playback")) << std::endl;
            std::cout << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_speed_maya")) << std::endl;
            std::cout << "   " << textSystem->getText(DJV_TEXT("djv_cli_option_speed_maya_description")) << std::endl;
            std::cout << std::endl;
        }

        void Application::_readIcon(const std::string& fileName)
        {
            DJV_PRIVATE_PTR();
            try
            {
                auto resourceSystem = getSystemT<Core::ResourceSystem>();
                const auto& iconsPath = resourceSystem->getPath(Core::FileSystem::ResourcePath::Icons);
                auto io = getSystemT<AV::IO::System>();
                p.read.push_back(io->read(Core::FileSystem::Path(iconsPath, fileName)));
            }
            catch (const std::exception& e)
            {
                auto logSystem = getSystemT<LogSystem>();
                logSystem->log(
                    "djv::ViewApp::Application",
                    String::Format("{0}: {1}").arg(fileName).arg(e.what()),
                    LogLevel::Error);
            }
        }

    } // namespace ViewApp
} // namespace djv

