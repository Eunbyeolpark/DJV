// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvCore/Context.h>

#include <djvCore/CoreSystem.h>
#include <djvCore/FileIO.h>
#include <djvCore/IObject.h>
#include <djvCore/LogSystem.h>
#include <djvCore/OS.h>
#include <djvCore/ResourceSystem.h>
#include <djvCore/TextSystem.h>
#include <djvCore/Time.h>
#include <djvCore/Timer.h>

#include <iostream>
#include <thread>

#if defined(DJV_PLATFORM_WINDOWS)
#include <fcntl.h>
#endif // DJV_PLATFORM_WINDOWS

namespace djv
{
    namespace Core
    {
        namespace
        {
            //! \todo Should this be configurable?
            const size_t fpsSamplesCount = 60;

            void addSample(std::list<float>& list, float sample)
            {
                list.push_front(sample);
                while (list.size() > fpsSamplesCount)
                {
                    list.pop_back();
                }
            }
            
            float averageSamples(const std::list<float>& list)
            {
                float out = 0.F;
                for (const auto& i : list)
                {
                    out += i;
                }
                return out / static_cast<float>(list.size());
            }

            struct TickTimes
            {
                Time::TimePoint time = std::chrono::steady_clock::now();
                Time::Duration total = Time::Duration::zero();
                std::vector<std::pair<std::string, Time::Duration> > times;

                void add(const std::string& name)
                {
                    auto end = std::chrono::steady_clock::now();
                    const auto diff = std::chrono::duration_cast<Time::Duration>(end - time);
                    time = end;
                    times.push_back(std::make_pair(name, diff));
                    total += diff;
                }

                void sort()
                {
                    std::sort(
                        times.begin(),
                        times.end(),
                        [](const std::pair<std::string, Time::Duration>& a,
                            const std::pair<std::string, Time::Duration>& b)
                        {
                            return a.second > b.second;
                        });
                }

                void print()
                {
                    if (times.size() > 0)
                    {
                        std::cout << "System tick time: " <<
                            times[0].first << ", " <<
                            times[0].second.count() << std::endl;
                    }
                    for (const auto& i : times)
                    {
                        std::cout << i.first << ": " << i.second.count() << std::endl;
                    }
                    std::cout << "total: " << total.count() << std::endl << std::endl;
                }
            };

        } // namespace

        void Context::_init(const std::string& argv0)
        {
            _name = FileSystem::Path(argv0).getBaseName();

#if defined(DJV_PLATFORM_WINDOWS)
            _set_fmode(_O_BINARY);
#endif // DJV_PLATFORM_WINDOWS

            _timerSystem = Time::TimerSystem::create(shared_from_this());
            _resourceSystem = ResourceSystem::create(argv0, shared_from_this());
            _logSystem = LogSystem::create(shared_from_this());
            _textSystem = TextSystem::create(shared_from_this());
            CoreSystem::create(argv0, shared_from_this());

            _logInfo(argv0);

            _fpsTimer = Time::Timer::create(shared_from_this());
            _fpsTimer->setRepeating(true);
            auto weak = std::weak_ptr<Context>(shared_from_this());
            _fpsTimer->start(
                Time::getTime(Time::TimerValue::VerySlow),
                [weak](const std::chrono::steady_clock::time_point&, const Time::Duration&)
            {
                if (auto context = weak.lock())
                {
                    std::stringstream ss;
                    ss << "FPS: " << context->_fpsAverage;
                    context->_logSystem->log("djv::Core::Context", ss.str());
                }
            });
        }

        Context::~Context()
        {}

        std::shared_ptr<Context> Context::create(const std::string& argv0)
        {
            auto out = std::shared_ptr<Context>(new Context);
            out->_init(argv0);
            return out;
        }

        void Context::removeSystem(const std::shared_ptr<ISystemBase>& value)
        {
            auto i = _systems.begin();
            while (i != _systems.end())
            {
                if (value == *i)
                {
                    i = _systems.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
                
        void Context::tick()
        {
            if (_logSystemOrderInit)
            {
                _logSystemOrderInit = false;
                _logSystemOrder();
                //_writeSystemDotGraph();
            }

            _calcFPS();

            TickTimes tickTimes;
            for (const auto& system : _systems)
            {
                system->tick();
                tickTimes.add(system->getSystemName());
            }
            tickTimes.sort();
            //tickTimes.print();
            _systemTickTimes = tickTimes.times;
        }

        void Context::_addSystem(const std::shared_ptr<ISystemBase>& system)
        {
            _systems.push_back(system);
        }

        void Context::_logInfo(const std::string& argv0)
        {
            std::stringstream ss;
            ss << "Application: " << _name << '\n';
            ss << "System: " << OS::getInformation() << '\n';
            ss << "Hardware concurrency: " << std::thread::hardware_concurrency() << '\n';
            {
                std::stringstream ss2;
                ss2 << Memory::Unit::GB;
                ss << "RAM: " << (OS::getRAMSize() / Memory::gigabyte) << _textSystem->getText(ss2.str()) << '\n';
            }
            ss << "argv0: " << argv0 << '\n';
            ss << "Resource paths:" << '\n';
            for (auto path : FileSystem::getResourcePathEnums())
            {
                std::stringstream ss2;
                ss2 << path;
                ss << "    " << _textSystem->getText(ss2.str()) << ": " << _resourceSystem->getPath(path) << '\n';
            }
            _logSystem->log("djv::Core::Context", ss.str());
        }

        void Context::_logSystemOrder()
        {
            size_t count = 0;
            std::vector<std::string> dot;
            dot.push_back("digraph {");
            for (const auto& system : _systems)
            {
                {
                    std::stringstream ss;
                    ss << "Tick system #" << count << ": " << system->getSystemName();
                    _logSystem->log("djv::Core::Context", ss.str());
                    ++count;
                }
                for (const auto& dependency : system->getDependencies())
                {
                    std::stringstream ss;
                    ss << "    " << "\"" << system->getSystemName() << "\"";
                    ss << " -> " << "\"" << dependency->getSystemName() << "\"";
                    dot.push_back(ss.str());
                }
            }
            dot.push_back("}");
            //FileSystem::FileIO::writeLines("systems.dot", dot);
        }
        
        void Context::_writeSystemDotGraph()
        {
            std::vector<std::string> dot;
            dot.push_back("digraph {");
            for (const auto& system : _systems)
            {
                for (const auto& dependency : system->getDependencies())
                {
                    std::stringstream ss;
                    ss << "    " << "\"" << system->getSystemName() << "\"";
                    ss << " -> " << "\"" << dependency->getSystemName() << "\"";
                    dot.push_back(ss.str());
                }
            }
            dot.push_back("}");
            FileSystem::FileIO::writeLines("systems.dot", dot);
        }

        void Context::_calcFPS()
        {
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<float> delta = now - _fpsTime;
            _fpsTime = now;
            addSample(_fpsSamples, delta.count());
            _fpsAverage = 1.F / averageSamples(_fpsSamples);
            //std::cout << "fps = " << _fpsAverage << std::endl;
        }

    } // namespace ViewExperiment
} // namespace djv

