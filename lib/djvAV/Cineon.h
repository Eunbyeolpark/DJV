// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvAV/SequenceIO.h>

namespace djv
{
    namespace AV
    {
        namespace IO
        {
            //! This namespace provides Cineon image I/O.
            //!
            //! References:
            //! - Kodak, "4.5 DRAFT - Image File Format Proposal for Digital Pictures"
            namespace Cineon
            {
                static const std::string pluginName = "Cineon";
                static const std::set<std::string> fileExtensions = { ".cin" };

                //! This enumeration provides the Cineon file color profiles.
                enum class ColorProfile
                {
                    Raw,
                    FilmPrint,

                    Count,
                    First = Raw
                };

                //! This constant provides the Cineon file header magic numbers.
                const uint32_t magic[] =
                {
                    0x802a5fd7,
                    0xd75f2a80
                };

                //! This enumeration provides the Cineon file image orientations.
                enum class Orient
                {
                    LeftRightTopBottom,
                    LeftRightBottomTop,
                    RightLeftTopBottom,
                    RightLeftBottomTop,
                    TopBottomLeftRight,
                    TopBottomRightLeft,
                    BottomTopLeftRight,
                    BottomTopRightLeft
                };

                //! This enumeration provides the Cineon file descriptors.
                enum class Descriptor
                {
                    Luminance,
                    RedFilmPrint,
                    GreenFilmPrint,
                    BlueFilmPrint,
                    RedCCIRXA11,
                    GreenCCIRXA11,
                    BlueCCIRXA11
                };

                //! This stuct provides the Cineon file header.
                struct Header
                {
                    struct File
                    {
                        uint32_t magic;
                        uint32_t imageOffset;
                        uint32_t headerSize;
                        uint32_t industryHeaderSize;
                        uint32_t userHeaderSize;
                        uint32_t size;
                        char     version[8];
                        char     name[100];
                        char     time[24];
                        uint8_t  pad[36];
                    };
                    File file;

                    struct Image
                    {
                        uint8_t orient;
                        uint8_t channels;
                        uint8_t pad[2];

                        struct Channel
                        {
                            uint8_t  descriptor[2];
                            uint8_t  bitDepth;
                            uint8_t  pad;
                            uint32_t size[2];
                            float    lowData;
                            float    lowQuantity;
                            float    highData;
                            float    highQuantity;
                        };
                        Channel channel[8];

                        float    white[2];
                        float    red[2];
                        float    green[2];
                        float    blue[2];
                        char     label[200];
                        uint8_t  pad2[28];
                        uint8_t  interleave;
                        uint8_t  packing;
                        uint8_t  dataSign;
                        uint8_t  dataSense;
                        uint32_t linePadding;
                        uint32_t channelPadding;
                        uint8_t  pad3[20];
                    };
                    Image image;

                    struct Source
                    {
                        int32_t offset[2];
                        char    file[100];
                        char    time[24];
                        char    inputDevice[64];
                        char    inputModel[32];
                        char    inputSerial[32];
                        float   inputPitch[2];
                        float   gamma;
                        char    pad[40];
                    };
                    Source source;

                    struct Film
                    {
                        uint8_t  id;
                        uint8_t  type;
                        uint8_t  offset;
                        uint8_t  pad;
                        uint8_t  prefix;
                        uint32_t count;
                        char     format[32];
                        uint32_t frame;
                        float    frameRate;
                        char     frameId[32];
                        char     slate[200];
                        char     pad2[740];
                    };
                    Film film;
                };

                //! Check whether the value is valid.
                bool isValid(const char*, size_t size);
                
                //! Convert to std::string.
                std::string toString(const char* in, size_t size);
                
                //! Convert from std::string.
                size_t fromString(
                    const std::string& string,
                    char*              out,
                    size_t             maxLen,
                    bool               terminate);
                    
                //! Zero out the data in a Cineon file header.
                void zero(Header&);

                //! Read a Cineon file header.
                //!
                //! Throws:
                //! - Core::FileSystem::Error
                Header read(
                    const std::shared_ptr<Core::FileSystem::FileIO>&,
                    Info&,
                    ColorProfile&,
                    const std::shared_ptr<Core::TextSystem>&);
                
                //! Write a Cineon file header.
                //!
                //! Throws:
                //! - Core::FileSystem::Error
                void write(
                    const std::shared_ptr<Core::FileSystem::FileIO>&,
                    const Info& info,
                    ColorProfile);

                //! Finish writing the Cineon file header after image data is written.
                void writeFinish(const std::shared_ptr<Core::FileSystem::FileIO>&);

                //! This class provides the Cineon file reader.
                class Read : public ISequenceRead
                {
                    DJV_NON_COPYABLE(Read);

                protected:
                    Read();

                public:
                    ~Read() override;

                    static std::shared_ptr<Read> create(
                        const Core::FileSystem::FileInfo&,
                        const ReadOptions&,
                        const std::shared_ptr<Core::TextSystem>&,
                        const std::shared_ptr<Core::ResourceSystem>&,
                        const std::shared_ptr<Core::LogSystem>&);

                    static std::shared_ptr<Image::Image> readImage(
                        const Info&,
                        const std::shared_ptr<Core::FileSystem::FileIO>&);

                protected:
                    Info _readInfo(const std::string&) override;
                    std::shared_ptr<Image::Image> _readImage(const std::string&) override;

                private:
                    Info _open(const std::string&, const std::shared_ptr<Core::FileSystem::FileIO>&);

                    DJV_PRIVATE();
                };

                //! This class provides the Cineon file writer.
                class Write : public ISequenceWrite
                {
                    DJV_NON_COPYABLE(Write);

                protected:
                    Write();

                public:
                    ~Write() override;

                    static std::shared_ptr<Write> create(
                        const Core::FileSystem::FileInfo&,
                        const Info&,
                        const WriteOptions&,
                        const std::shared_ptr<Core::TextSystem>&,
                        const std::shared_ptr<Core::ResourceSystem>&,
                        const std::shared_ptr<Core::LogSystem>&);

                protected:
                    Image::Type _getImageType(Image::Type) const override;
                    Image::Layout _getImageLayout() const override;
                    void _write(const std::string& fileName, const std::shared_ptr<Image::Image>&) override;

                private:
                    DJV_PRIVATE();
                };

                //! This class provides the Cineon file I/O plugin.
                class Plugin : public ISequencePlugin
                {
                    DJV_NON_COPYABLE(Plugin);

                protected:
                    Plugin();

                public:
                    static std::shared_ptr<Plugin> create(const std::shared_ptr<Core::Context>&);

                    std::shared_ptr<IRead> read(const Core::FileSystem::FileInfo&, const ReadOptions&) const override;
                    std::shared_ptr<IWrite> write(const Core::FileSystem::FileInfo&, const Info&, const WriteOptions&) const override;

                private:
                    DJV_PRIVATE();
                };

            } // namespace Cineon
        } // namespace IO
    } // namespace AV
} // namespace djv
