// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvViewApp/FileSettings.h>

#include <djvAV/ImageData.h>

#include <djvCore/Context.h>
#include <djvCore/FileInfo.h>

#if defined(GetObject)
#undef GetObject
#endif // GetObject

// These need to be included last on OSX.
#include <djvCore/RapidJSONTemplates.h>
#include <djvUI/ISettingsTemplates.h>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        struct FileSettings::Private
        {
            std::shared_ptr<ValueSubject<size_t> > openMax;
            std::shared_ptr<ListSubject<Core::FileSystem::FileInfo> > recentFiles;
            std::shared_ptr<ValueSubject<size_t> > recentFilesMax;
            std::shared_ptr<ValueSubject<bool> > autoDetectSequences;
            std::shared_ptr<ValueSubject<bool> > sequencesFirstFrame;
            std::shared_ptr<ValueSubject<bool> > cacheEnabled;
            std::shared_ptr<ValueSubject<int> > cacheMaxGB;
            std::map<std::string, BBox2f> widgetGeom;
        };

        void FileSettings::_init(const std::shared_ptr<Context>& context)
        {
            ISettings::_init("djv::ViewApp::FileSettings", context);
            DJV_PRIVATE_PTR();
            p.openMax = ValueSubject<size_t>::create(16);
            p.recentFiles = ListSubject<Core::FileSystem::FileInfo>::create();
            p.recentFilesMax = ValueSubject<size_t>::create(10);
            p.autoDetectSequences = ValueSubject<bool>::create(true);
            p.sequencesFirstFrame = ValueSubject<bool>::create(true);
            p.cacheEnabled = ValueSubject<bool>::create(true);
            p.cacheMaxGB = ValueSubject<int>::create(4);
            _load();
        }

        FileSettings::FileSettings() :
            _p(new Private)
        {}

        FileSettings::~FileSettings()
        {}

        std::shared_ptr<FileSettings> FileSettings::create(const std::shared_ptr<Context>& context)
        {
            auto out = std::shared_ptr<FileSettings>(new FileSettings);
            out->_init(context);
            return out;
        }

        std::shared_ptr<IValueSubject<size_t> > FileSettings::observeOpenMax() const
        {
            return _p->openMax;
        }

        void FileSettings::setOpenMax(size_t value)
        {
            _p->openMax->setIfChanged(value);
        }
        
        std::shared_ptr<IListSubject<Core::FileSystem::FileInfo> > FileSettings::observeRecentFiles() const
        {
            return _p->recentFiles;
        }

        std::shared_ptr<IValueSubject<size_t> > FileSettings::observeRecentFilesMax() const
        {
            return _p->recentFilesMax;
        }

        void FileSettings::setRecentFiles(const std::vector<Core::FileSystem::FileInfo>& value)
        {
            DJV_PRIVATE_PTR();
            std::vector<Core::FileSystem::FileInfo> files;
            for (size_t i = 0; i < value.size() && i < p.recentFilesMax->get(); ++i)
            {
                files.push_back(value[i]);
            }
            p.recentFiles->setIfChanged(files);
        }

        void FileSettings::setRecentFilesMax(size_t value)
        {
            DJV_PRIVATE_PTR();
            p.recentFilesMax->setIfChanged(value);
            const auto& files = _p->recentFiles->get();
            std::vector<Core::FileSystem::FileInfo> filesMax;
            for (size_t i = 0; i < files.size() && i < value; ++i)
            {
                filesMax.push_back(files[i]);
            }
            setRecentFiles(filesMax);
        }

        std::shared_ptr<IValueSubject<bool> > FileSettings::observeAutoDetectSequences() const
        {
            return _p->autoDetectSequences;
        }

        std::shared_ptr<IValueSubject<bool> > FileSettings::observeSequencesFirstFrame() const
        {
            return _p->sequencesFirstFrame;
        }

        void FileSettings::setAutoDetectSequences(bool value)
        {
            _p->autoDetectSequences->setIfChanged(value);
        }

        void FileSettings::setSequencesFirstFrame(bool value)
        {
            _p->sequencesFirstFrame->setIfChanged(value);
        }

        std::shared_ptr<IValueSubject<bool> > FileSettings::observeCacheEnabled() const
        {
            return _p->cacheEnabled;
        }

        std::shared_ptr<IValueSubject<int> > FileSettings::observeCacheMaxGB() const
        {
            return _p->cacheMaxGB;
        }

        void FileSettings::setCacheEnabled(bool value)
        {
            _p->cacheEnabled->setIfChanged(value);
        }

        void FileSettings::setCacheMaxGB(int value)
        {
            _p->cacheMaxGB->setIfChanged(value);
        }

        const std::map<std::string, BBox2f>& FileSettings::getWidgetGeom() const
        {
            return _p->widgetGeom;
        }

        void FileSettings::setWidgetGeom(const std::map<std::string, BBox2f>& value)
        {
            _p->widgetGeom = value;
        }

        void FileSettings::load(const rapidjson::Value & value)
        {
            if (value.IsObject())
            {
                DJV_PRIVATE_PTR();
                UI::Settings::read("OpenMax", value, p.openMax);
                std::vector< Core::FileSystem::FileInfo> recentFiles;
                UI::Settings::read("RecentFiles", value, recentFiles);
                auto i = recentFiles.begin();
                while (i != recentFiles.end())
                {
                    if (!i->doesExist())
                    {
                        i = recentFiles.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
                p.recentFiles->setIfChanged(recentFiles);
                UI::Settings::read("RecentFilesMax", value, p.recentFilesMax);
                UI::Settings::read("AutoDetectSequences", value, p.autoDetectSequences);
                UI::Settings::read("SequencesFirstFrame", value, p.sequencesFirstFrame);
                UI::Settings::read("CacheEnabled", value, p.cacheEnabled);
                UI::Settings::read("CacheMax", value, p.cacheMaxGB);
                UI::Settings::read("WidgetGeom", value, p.widgetGeom);
            }
        }

        rapidjson::Value FileSettings::save(rapidjson::Document::AllocatorType& allocator)
        {
            DJV_PRIVATE_PTR();
            rapidjson::Value out(rapidjson::kObjectType);
            UI::Settings::write("OpenMax", p.openMax->get(), out, allocator);
            UI::Settings::write("RecentFiles", p.recentFiles->get(), out, allocator);
            UI::Settings::write("RecentFilesMax", p.recentFilesMax->get(), out, allocator);
            UI::Settings::write("AutoDetectSequences", p.autoDetectSequences->get(), out, allocator);
            UI::Settings::write("SequencesFirstFrame", p.sequencesFirstFrame->get(), out, allocator);
            UI::Settings::write("CacheEnabled", p.cacheEnabled->get(), out, allocator);
            UI::Settings::write("CacheMax", p.cacheMaxGB->get(), out, allocator);
            UI::Settings::write("WidgetGeom", p.widgetGeom, out, allocator);
            return out;
        }

    } // namespace ViewApp
} // namespace djv

