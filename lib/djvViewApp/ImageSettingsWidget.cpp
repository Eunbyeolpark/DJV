//------------------------------------------------------------------------------
// Copyright (c) 2004-2019 Darby Johnston
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

#include <djvViewApp/ImageSettingsWidget.h>

#include <djvViewApp/ColorSpaceModel.h>
#include <djvViewApp/ImageSettings.h>

#include <djvUI/ComboBox.h>
#include <djvUI/FormLayout.h>
#include <djvUI/SettingsSystem.h>
#include <djvUI/ToggleButton.h>

#include <djvCore/Context.h>
#include <djvCore/TextSystem.h>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        struct ImageAspectRatioSettingsWidget::Private
        {
            ImageAspectRatio aspectRatio = ImageAspectRatio::First;
            std::shared_ptr<UI::ComboBox> comboBox;
            std::shared_ptr<ValueObserver<ImageAspectRatio> > aspectRatioObserver;
        };

        void ImageAspectRatioSettingsWidget::_init(Context* context)
        {
            ISettingsWidget::_init(context);
            DJV_PRIVATE_PTR();

            setClassName("djv::ViewApp::ImageAspectRatioSettingsWidget");

            p.comboBox = UI::ComboBox::create(context);
            addChild(p.comboBox);

            _widgetUpdate();

            auto weak = std::weak_ptr<ImageAspectRatioSettingsWidget>(std::dynamic_pointer_cast<ImageAspectRatioSettingsWidget>(shared_from_this()));
            p.comboBox->setCallback(
                [weak, context](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
                        {
                            imageSettings->setImageAspectRatio(static_cast<ImageAspectRatio>(value));
                        }
                    }
                });

            auto settingsSystem = context->getSystemT<UI::Settings::System>();
            if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
            {
                p.aspectRatioObserver = ValueObserver<ImageAspectRatio>::create(
                    imageSettings->observeImageAspectRatio(),
                    [weak](ImageAspectRatio value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->aspectRatio = value;
                            widget->_p->comboBox->setCurrentItem(static_cast<int>(value));
                        }
                    });
            }
        }

        ImageAspectRatioSettingsWidget::ImageAspectRatioSettingsWidget() :
            _p(new Private)
        {}

        std::shared_ptr<ImageAspectRatioSettingsWidget> ImageAspectRatioSettingsWidget::create(Context* context)
        {
            auto out = std::shared_ptr<ImageAspectRatioSettingsWidget>(new ImageAspectRatioSettingsWidget);
            out->_init(context);
            return out;
        }

        std::string ImageAspectRatioSettingsWidget::getSettingsName() const
        {
            return DJV_TEXT("Aspect Ratio");
        }

        std::string ImageAspectRatioSettingsWidget::getSettingsGroup() const
        {
            return DJV_TEXT("Image");
        }

        std::string ImageAspectRatioSettingsWidget::getSettingsSortKey() const
        {
            return "F";
        }

        void ImageAspectRatioSettingsWidget::_localeEvent(Event::Locale& event)
        {
            ISettingsWidget::_localeEvent(event);
            _widgetUpdate();
        }

        void ImageAspectRatioSettingsWidget::_widgetUpdate()
        {
            DJV_PRIVATE_PTR();
            p.comboBox->clearItems();
            for (auto i : getImageAspectRatioEnums())
            {
                std::stringstream ss;
                ss << i;
                p.comboBox->addItem(_getText(ss.str()));
            }
            p.comboBox->setCurrentItem(static_cast<int>(p.aspectRatio));
        }

        struct ImageColorSpaceSettingsWidget::Private
        {
            std::shared_ptr<ColorSpaceModel> model;

            std::shared_ptr<UI::ComboBox> inputComboBox;
            std::shared_ptr<UI::ComboBox> displayComboBox;
            std::shared_ptr<UI::ComboBox> viewComboBox;
            std::shared_ptr<UI::FormLayout> formLayout;

            std::shared_ptr<ListObserver<std::string> > colorSpacesObserver;
            std::shared_ptr<ListObserver<std::string> > displaysObserver;
            std::shared_ptr<ListObserver<std::string> > viewsObserver;
            std::shared_ptr<ValueObserver<std::string> > colorSpaceObserver;
            std::shared_ptr<ValueObserver<std::string> > colorSpaceObserver2;
            std::shared_ptr<ValueObserver<std::string> > displayObserver;
            std::shared_ptr<ValueObserver<std::string> > displayObserver2;
            std::shared_ptr<ValueObserver<std::string> > viewObserver;
            std::shared_ptr<ValueObserver<std::string> > viewObserver2;
        };

        void ImageColorSpaceSettingsWidget::_init(Context* context)
        {
            ISettingsWidget::_init(context);
            DJV_PRIVATE_PTR();

            setClassName("djv::ViewApp::ImageColorSpaceSettingsWidget");

            p.model = ColorSpaceModel::create(context);

            p.inputComboBox = UI::ComboBox::create(context);
            p.displayComboBox = UI::ComboBox::create(context);
            p.viewComboBox = UI::ComboBox::create(context);

            p.formLayout = UI::FormLayout::create(context);
            p.formLayout->addChild(p.inputComboBox);
            p.formLayout->addChild(p.displayComboBox);
            p.formLayout->addChild(p.viewComboBox);
            addChild(p.formLayout);

            _widgetUpdate();

            auto weak = std::weak_ptr<ImageColorSpaceSettingsWidget>(std::dynamic_pointer_cast<ImageColorSpaceSettingsWidget>(shared_from_this()));
            p.inputComboBox->setCallback(
                [weak, context](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
                        {
                            imageSettings->setColorSpace(widget->_p->model->indexToColorSpace(value));
                        }
                    }
                });

            p.displayComboBox->setCallback(
                [weak, context](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
                        {
                            imageSettings->setColorDisplay(widget->_p->model->indexToDisplay(value));
                        }
                    }
                });

            p.viewComboBox->setCallback(
                [weak, context](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
                        {
                            imageSettings->setColorView(widget->_p->model->indexToView(value));
                        }
                    }
                });

            p.colorSpacesObserver = ListObserver<std::string>::create(
                p.model->observeColorSpaces(),
                [weak](const std::vector<std::string>&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            p.displaysObserver = ListObserver<std::string>::create(
                p.model->observeDisplays(),
                [weak](const std::vector<std::string>&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            p.viewsObserver = ListObserver<std::string>::create(
                p.model->observeViews(),
                [weak](const std::vector<std::string>&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            p.colorSpaceObserver = ValueObserver<std::string>::create(
                p.model->observeColorSpace(),
                [weak](const std::string&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            p.displayObserver = ValueObserver<std::string>::create(
                p.model->observeDisplay(),
                [weak](const std::string&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            p.viewObserver = ValueObserver<std::string>::create(
                p.model->observeView(),
                [weak](const std::string&)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_widgetUpdate();
                    }
                });

            auto settingsSystem = context->getSystemT<UI::Settings::System>();
            if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
            {
                p.colorSpaceObserver2 = ValueObserver<std::string>::create(
                    imageSettings->observeColorSpace(),
                    [weak](const std::string& value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->model->setColorSpace(value);
                        }
                    });

                p.displayObserver2 = ValueObserver<std::string>::create(
                    imageSettings->observeColorDisplay(),
                    [weak](const std::string& value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->model->setDisplay(value);
                        }
                    });

                p.viewObserver2 = ValueObserver<std::string>::create(
                    imageSettings->observeColorView(),
                    [weak](const std::string& value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->model->setView(value);
                        }
                    });
            }
        }

        ImageColorSpaceSettingsWidget::ImageColorSpaceSettingsWidget() :
            _p(new Private)
        {}

        std::shared_ptr<ImageColorSpaceSettingsWidget> ImageColorSpaceSettingsWidget::create(Context* context)
        {
            auto out = std::shared_ptr<ImageColorSpaceSettingsWidget>(new ImageColorSpaceSettingsWidget);
            out->_init(context);
            return out;
        }

        std::string ImageColorSpaceSettingsWidget::getSettingsName() const
        {
            return DJV_TEXT("Color Space");
        }

        std::string ImageColorSpaceSettingsWidget::getSettingsGroup() const
        {
            return DJV_TEXT("Image");
        }

        std::string ImageColorSpaceSettingsWidget::getSettingsSortKey() const
        {
            return "F";
        }

        void ImageColorSpaceSettingsWidget::_localeEvent(Event::Locale& event)
        {
            ISettingsWidget::_localeEvent(event);
            DJV_PRIVATE_PTR();
            p.formLayout->setText(p.inputComboBox, _getText(DJV_TEXT("Input")) + ":");
            p.formLayout->setText(p.displayComboBox, _getText(DJV_TEXT("Display")) + ":");
            p.formLayout->setText(p.viewComboBox, _getText(DJV_TEXT("View")) + ":");
            _widgetUpdate();
        }

        void ImageColorSpaceSettingsWidget::_widgetUpdate()
        {
            DJV_PRIVATE_PTR();
            auto context = getContext();
            p.inputComboBox->clearItems();
            for (const auto& i : p.model->observeColorSpaces()->get())
            {
                p.inputComboBox->addItem(!i.empty() ? i : "-");
            }
            const std::string& colorSpace = p.model->observeColorSpace()->get();
            p.inputComboBox->setCurrentItem(p.model->colorSpaceToIndex(colorSpace));

            p.displayComboBox->clearItems();
            for (const auto& i : p.model->observeDisplays()->get())
            {
                p.displayComboBox->addItem(!i.empty() ? i : "-");
            }
            p.displayComboBox->setCurrentItem(p.model->displayToIndex(p.model->observeDisplay()->get()));

            p.viewComboBox->clearItems();
            for (const auto& i : p.model->observeViews()->get())
            {
                p.viewComboBox->addItem(!i.empty() ? i : "-");
            }
            p.viewComboBox->setCurrentItem(p.model->viewToIndex(p.model->observeView()->get()));
        }

        struct ImageRotateSettingsWidget::Private
        {
            ImageRotate rotate = ImageRotate::First;
            std::shared_ptr<UI::ComboBox> comboBox;
            std::shared_ptr<ValueObserver<ImageRotate> > rotateObserver;
        };

        void ImageRotateSettingsWidget::_init(Context* context)
        {
            ISettingsWidget::_init(context);
            DJV_PRIVATE_PTR();

            setClassName("djv::ViewApp::ImageRotateSettingsWidget");

            p.comboBox = UI::ComboBox::create(context);
            addChild(p.comboBox);

            _widgetUpdate();

            auto weak = std::weak_ptr<ImageRotateSettingsWidget>(std::dynamic_pointer_cast<ImageRotateSettingsWidget>(shared_from_this()));
            p.comboBox->setCallback(
                [weak, context](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        auto settingsSystem = context->getSystemT<UI::Settings::System>();
                        if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
                        {
                            imageSettings->setImageRotate(static_cast<ImageRotate>(value));
                        }
                    }
                });

            auto settingsSystem = context->getSystemT<UI::Settings::System>();
            if (auto imageSettings = settingsSystem->getSettingsT<ImageSettings>())
            {
                p.rotateObserver = ValueObserver<ImageRotate>::create(
                    imageSettings->observeImageRotate(),
                    [weak](ImageRotate value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->rotate = value;
                            widget->_p->comboBox->setCurrentItem(static_cast<int>(value));
                        }
                    });
            }
        }

        ImageRotateSettingsWidget::ImageRotateSettingsWidget() :
            _p(new Private)
        {}

        std::shared_ptr<ImageRotateSettingsWidget> ImageRotateSettingsWidget::create(Context* context)
        {
            auto out = std::shared_ptr<ImageRotateSettingsWidget>(new ImageRotateSettingsWidget);
            out->_init(context);
            return out;
        }

        std::string ImageRotateSettingsWidget::getSettingsName() const
        {
            return DJV_TEXT("Rotate");
        }

        std::string ImageRotateSettingsWidget::getSettingsGroup() const
        {
            return DJV_TEXT("Image");
        }

        std::string ImageRotateSettingsWidget::getSettingsSortKey() const
        {
            return "F";
        }

        void ImageRotateSettingsWidget::_localeEvent(Event::Locale& event)
        {
            ISettingsWidget::_localeEvent(event);
            _widgetUpdate();
        }

        void ImageRotateSettingsWidget::_widgetUpdate()
        {
            DJV_PRIVATE_PTR();
            p.comboBox->clearItems();
            for (auto i : getImageRotateEnums())
            {
                std::stringstream ss;
                ss << i;
                p.comboBox->addItem(_getText(ss.str()));
            }
            p.comboBox->setCurrentItem(static_cast<int>(p.rotate));
        }

    } // namespace ViewApp
} // namespace djv

