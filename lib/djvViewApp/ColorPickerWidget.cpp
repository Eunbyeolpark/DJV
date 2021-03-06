// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvViewApp/ColorPickerWidget.h>

#include <djvViewApp/Media.h>
#include <djvViewApp/MediaWidget.h>
#include <djvViewApp/ViewWidget.h>
#include <djvViewApp/WindowSystem.h>

#include <djvUIComponents/ColorPicker.h>

#include <djvUI/Action.h>
#include <djvUI/ActionGroup.h>
#include <djvUI/ColorSwatch.h>
#include <djvUI/EventSystem.h>
#include <djvUI/FormLayout.h>
#include <djvUI/ImageWidget.h>
#include <djvUI/IntSlider.h>
#include <djvUI/Label.h>
#include <djvUI/Menu.h>
#include <djvUI/PopupMenu.h>
#include <djvUI/RowLayout.h>
#include <djvUI/ToolButton.h>

#include <djvAV/OCIOSystem.h>
#include <djvAV/ImageUtil.h>
#include <djvAV/OpenGLOffscreenBuffer.h>
#include <djvAV/Render2D.h>
#if defined(DJV_OPENGL_ES2)
#include <djvAV/OpenGLMesh.h>
#include <djvAV/OpenGLShader.h>
#include <djvAV/Shader.h>
#endif // DJV_OPENGL_ES2

#include <djvCore/Context.h>
#if defined(DJV_OPENGL_ES2)
#include <djvCore/ResourceSystem.h>
#endif // DJV_OPENGL_ES2

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_transform_2d.hpp>

#include <iomanip>

using namespace djv::Core;

namespace djv
{
    namespace ViewApp
    {
        namespace
        {
            //! \todo Should this be configurable?
            const size_t sampleSizeMax = 100;

            //! \todo What is this really?
            const size_t bufferSizeMin = 100;
        
        } // namespace

        struct ColorPickerWidget::Private
        {
            bool currentTool = false;
            size_t sampleSize = 1;
            AV::Image::Type lockType = AV::Image::Type::None;
            bool applyColorOperations = true;
            bool applyColorSpace = true;
            AV::Image::Color color = AV::Image::Color(0.F, 0.F, 0.F);
            glm::vec2 pickerPos = glm::vec2(0.F, 0.F);
            std::shared_ptr<AV::Image::Image> image;
            AV::Render2D::ImageOptions imageOptions;
            glm::vec2 imagePos = glm::vec2(0.F, 0.F);
            float imageZoom = 1.F;
            UI::ImageRotate imageRotate = UI::ImageRotate::First;
            UI::ImageAspectRatio imageAspectRatio = UI::ImageAspectRatio::First;
            glm::vec2 pixelPos = glm::vec2(0.F, 0.F);
            AV::OCIO::Config ocioConfig;
            std::string outputColorSpace;
            std::shared_ptr<MediaWidget> activeWidget;

            std::map<std::string, std::shared_ptr<UI::Action> > actions;
            std::shared_ptr<UI::ColorSwatch> colorSwatch;
            std::shared_ptr<UI::Label> colorLabel;
            std::shared_ptr<UI::Label> pixelLabel;
            std::shared_ptr<UI::IntSlider> sampleSizeSlider;
            std::shared_ptr<UI::ColorTypeWidget> typeWidget;
            std::shared_ptr<UI::ToolButton> copyButton;
            std::shared_ptr<UI::Menu> settingsMenu;
            std::shared_ptr<UI::PopupMenu> settingsPopupMenu;
            std::shared_ptr<UI::FormLayout> formLayout;
            std::shared_ptr<UI::VerticalLayout> layout;

            std::shared_ptr<AV::OpenGL::OffscreenBuffer> offscreenBuffer;
#if defined(DJV_OPENGL_ES2)
            std::shared_ptr<AV::OpenGL::Shader> shader;
#endif // DJV_OPENGL_ES2

            std::map<std::string, std::shared_ptr<ValueObserver<bool> > > actionObservers;
            std::shared_ptr<ValueObserver<std::shared_ptr<MediaWidget> > > activeWidgetObserver;
            std::shared_ptr<ValueObserver<std::shared_ptr<AV::Image::Image> > > imageObserver;
            std::shared_ptr<ValueObserver<AV::Render2D::ImageOptions> > imageOptionsObserver;
            std::shared_ptr<ValueObserver<glm::vec2> > imagePosObserver;
            std::shared_ptr<ValueObserver<float> > imageZoomObserver;
            std::shared_ptr<ValueObserver<UI::ImageRotate> > imageRotateObserver;
            std::shared_ptr<ValueObserver<UI::ImageAspectRatio> > imageAspectRatioObserver;
            std::shared_ptr<ValueObserver<AV::OCIO::Config> > ocioConfigObserver;
            std::shared_ptr<ValueObserver<PointerData> > dragObserver;
        };

        void ColorPickerWidget::_init(const std::shared_ptr<Core::Context>& context)
        {
            MDIWidget::_init(context);

            DJV_PRIVATE_PTR();
            setClassName("djv::ViewApp::ColorPickerWidget");
            
            p.actions["LockType"] = UI::Action::create();
            p.actions["LockType"]->setButtonType(UI::ButtonType::Toggle);
            p.actions["ApplyColorOperations"] = UI::Action::create();
            p.actions["ApplyColorOperations"]->setButtonType(UI::ButtonType::Toggle);
            p.actions["ApplyColorSpace"] = UI::Action::create();
            p.actions["ApplyColorSpace"]->setButtonType(UI::ButtonType::Toggle);

            p.colorSwatch = UI::ColorSwatch::create(context);
            p.colorSwatch->setBorder(false);
            p.colorSwatch->setHAlign(UI::HAlign::Fill);

            p.colorLabel = UI::Label::create(context);
            p.colorLabel->setFontFamily(AV::Font::familyMono);
            p.colorLabel->setTextHAlign(UI::TextHAlign::Left);

            p.pixelLabel = UI::Label::create(context);
            p.pixelLabel->setFontFamily(AV::Font::familyMono);
            p.pixelLabel->setTextHAlign(UI::TextHAlign::Left);

            p.sampleSizeSlider = UI::IntSlider::create(context);
            p.sampleSizeSlider->setRange(IntRange(1, sampleSizeMax));

            p.typeWidget = UI::ColorTypeWidget::create(context);

            p.copyButton = UI::ToolButton::create(context);
            p.copyButton->setIcon("djvIconShare");

            p.settingsMenu = UI::Menu::create(context);
            p.settingsMenu->setIcon("djvIconSettings");
            p.settingsMenu->addAction(p.actions["LockType"]);
            p.settingsMenu->addAction(p.actions["ApplyColorOperations"]);
            p.settingsMenu->addAction(p.actions["ApplyColorSpace"]);
            p.settingsPopupMenu = UI::PopupMenu::create(context);
            p.settingsPopupMenu->setMenu(p.settingsMenu);

            p.layout = UI::VerticalLayout::create(context);
            p.layout->setSpacing(UI::MetricsRole::None);
            p.layout->setBackgroundRole(UI::ColorRole::Background);
            p.layout->setShadowOverlay({ UI::Side::Top });
            p.layout->addChild(p.colorSwatch);
            p.layout->setStretch(p.colorSwatch, UI::RowStretch::Expand);
            p.formLayout = UI::FormLayout::create(context);
            p.formLayout->setMargin(UI::MetricsRole::MarginSmall);
            p.formLayout->setSpacing(UI::MetricsRole::SpacingSmall);
            p.formLayout->addChild(p.colorLabel);
            p.formLayout->addChild(p.pixelLabel);
            p.formLayout->addChild(p.sampleSizeSlider);
            p.layout->addChild(p.formLayout);
            auto hLayout = UI::HorizontalLayout::create(context);
            hLayout->setSpacing(UI::MetricsRole::None);
            hLayout->addChild(p.typeWidget);
            hLayout->addChild(p.copyButton);
            hLayout->addExpander();
            hLayout->addChild(p.settingsPopupMenu);
            p.layout->addChild(hLayout);
            addChild(p.layout);

#if defined(DJV_OPENGL_ES2)
            auto resourceSystem = context->getSystemT<ResourceSystem>();
            const Core::FileSystem::Path shaderPath = resourceSystem->getPath(Core::FileSystem::ResourcePath::Shaders);
            p.shader = AV::OpenGL::Shader::create(AV::Render::Shader::create(
                Core::FileSystem::Path(shaderPath, "djvAVRender2DVertex.glsl"),
                Core::FileSystem::Path(shaderPath, "djvAVRender2DFragment.glsl")));
#endif // DJV_OPENGL_ES2

            _sampleUpdate();
            _widgetUpdate();

            auto weak = std::weak_ptr<ColorPickerWidget>(std::dynamic_pointer_cast<ColorPickerWidget>(shared_from_this()));
            p.copyButton->setClickedCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        if (auto eventSystem = widget->_getEventSystem().lock())
                        {
                            std::stringstream ss;
                            ss << AV::Image::Color::getLabel(widget->_p->color) << ", ";
                            ss << floorf(widget->_p->pixelPos.x) << " ";
                            ss << floorf(widget->_p->pixelPos.y);
                            eventSystem->setClipboard(ss.str());
                        }
                    }
                });

            p.sampleSizeSlider->setValueCallback(
                [weak](int value)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_p->sampleSize = value;
                        widget->_sampleUpdate();
                        widget->_widgetUpdate();
                    }
                });

            p.typeWidget->setTypeCallback(
                [weak](AV::Image::Type value)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_p->color = widget->_p->color.convert(value);
                        if (widget->_p->lockType != AV::Image::Type::None)
                        {
                            widget->_p->lockType = value;
                        }
                        widget->_widgetUpdate();
                    }
                });

            p.actionObservers["LockType"] = ValueObserver<bool>::create(
                p.actions["LockType"]->observeChecked(),
                [weak](bool value)
            {
                if (auto widget = weak.lock())
                {
                    if (value)
                    {
                        widget->_p->lockType = widget->_p->typeWidget->getType();
                    }
                    else
                    {
                        widget->_p->lockType = AV::Image::Type::None;
                    }
                }
            });

            p.actionObservers["ApplyColorOperations"] = ValueObserver<bool>::create(
                p.actions["ApplyColorOperations"]->observeChecked(),
                [weak](bool value)
            {
                if (auto widget = weak.lock())
                {
                    widget->_p->applyColorOperations = value;
                    widget->_sampleUpdate();
                    widget->_widgetUpdate();
                }
            });

            p.actionObservers["ApplyColorSpace"] = ValueObserver<bool>::create(
                p.actions["ApplyColorSpace"]->observeChecked(),
                [weak](bool value)
            {
                if (auto widget = weak.lock())
                {
                    widget->_p->applyColorSpace = value;
                    widget->_sampleUpdate();
                    widget->_widgetUpdate();
                }
            });
        
            if (auto windowSystem = context->getSystemT<WindowSystem>())
            {
                p.activeWidgetObserver = ValueObserver<std::shared_ptr<MediaWidget> >::create(
                    windowSystem->observeActiveWidget(),
                    [weak](const std::shared_ptr<MediaWidget>& value)
                    {
                        if (auto widget = weak.lock())
                        {
                            widget->_p->activeWidget = value;
                            if (widget->_p->activeWidget)
                            {
                                widget->_p->imageObserver = ValueObserver<std::shared_ptr<AV::Image::Image> >::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImage(),
                                    [weak](const std::shared_ptr<AV::Image::Image>& value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->image = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->imageOptionsObserver = ValueObserver<AV::Render2D::ImageOptions>::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImageOptions(),
                                    [weak](const AV::Render2D::ImageOptions& value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->imageOptions = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->imagePosObserver = ValueObserver<glm::vec2>::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImagePos(),
                                    [weak](const glm::vec2& value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->imagePos = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->imageZoomObserver = ValueObserver<float>::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImageZoom(),
                                    [weak](float value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->imageZoom = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->imageRotateObserver = ValueObserver<UI::ImageRotate>::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImageRotate(),
                                    [weak](UI::ImageRotate value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->imageRotate = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->imageAspectRatioObserver = ValueObserver<UI::ImageAspectRatio>::create(
                                    widget->_p->activeWidget->getViewWidget()->observeImageAspectRatio(),
                                    [weak](UI::ImageAspectRatio value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            widget->_p->imageAspectRatio = value;
                                            widget->_sampleUpdate();
                                            widget->_widgetUpdate();
                                        }
                                    });

                                widget->_p->dragObserver = ValueObserver<PointerData>::create(
                                    widget->_p->activeWidget->observeDrag(),
                                    [weak](const PointerData& value)
                                    {
                                        if (auto widget = weak.lock())
                                        {
                                            if (widget->_p->currentTool)
                                            {
                                                widget->_p->pickerPos = value.pos;
                                                widget->_sampleUpdate();
                                                widget->_widgetUpdate();
                                            }
                                        }
                                    });
                            }
                            else
                            {
                                widget->_p->imageObserver.reset();
                                widget->_p->imageOptionsObserver.reset();
                                widget->_p->imagePosObserver.reset();
                                widget->_p->imageZoomObserver.reset();
                                widget->_p->imageRotateObserver.reset();
                                widget->_p->imageAspectRatioObserver.reset();
                                widget->_p->dragObserver.reset();
                            }
                        }
                    });
            }

            auto ocioSystem = context->getSystemT<AV::OCIO::System>();
            auto contextWeak = std::weak_ptr<Context>(context);
            p.ocioConfigObserver = ValueObserver<AV::OCIO::Config>::create(
                ocioSystem->observeCurrentConfig(),
                [weak, contextWeak](const AV::OCIO::Config& value)
                {
                    if (auto context = contextWeak.lock())
                    {
                        if (auto widget = weak.lock())
                        {
                            auto ocioSystem = context->getSystemT<AV::OCIO::System>();
                            widget->_p->ocioConfig = value;
                            widget->_p->outputColorSpace = ocioSystem->getColorSpace(value.display, value.view);
                            widget->_sampleUpdate();
                            widget->_widgetUpdate();
                        }
                    }
                });
        }

        ColorPickerWidget::ColorPickerWidget() :
            _p(new Private)
        {}

        ColorPickerWidget::~ColorPickerWidget()
        {}

        std::shared_ptr<ColorPickerWidget> ColorPickerWidget::create(const std::shared_ptr<Core::Context>& context)
        {
            auto out = std::shared_ptr<ColorPickerWidget>(new ColorPickerWidget);
            out->_init(context);
            return out;
        }

        void ColorPickerWidget::setCurrentTool(bool value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.currentTool)
                return;
            p.currentTool = value;
            _sampleUpdate();
            _widgetUpdate();
        }

        size_t ColorPickerWidget::getSampleSize() const
        {
            return _p->sampleSize;
        }

        void ColorPickerWidget::setSampleSize(size_t value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.sampleSize)
                return;
            p.sampleSize = value;
            _widgetUpdate();
        }

        AV::Image::Type ColorPickerWidget::getLockType() const
        {
            return _p->lockType;
        }

        void ColorPickerWidget::setLockType(AV::Image::Type value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.lockType)
                return;
            p.lockType = value;
            if (p.lockType != AV::Image::Type::None)
            {
                p.color = p.color.convert(p.lockType);
            }
            _widgetUpdate();
        }

        bool ColorPickerWidget::getApplyColorOperations() const
        {
            return _p->applyColorOperations;
        }

        void ColorPickerWidget::setApplyColorOperations(bool value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.applyColorOperations)
                return;
            p.applyColorOperations = value;
            _sampleUpdate();
            _widgetUpdate();
        }

        bool ColorPickerWidget::getApplyColorSpace() const
        {
            return _p->applyColorSpace;
        }

        void ColorPickerWidget::setApplyColorSpace(bool value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.applyColorSpace)
                return;
            p.applyColorSpace = value;
            _sampleUpdate();
            _widgetUpdate();
        }

        const glm::vec2& ColorPickerWidget::getPickerPos() const
        {
            return _p->pickerPos;
        }

        void ColorPickerWidget::setPickerPos(const glm::vec2& value)
        {
            DJV_PRIVATE_PTR();
            if (value == p.pickerPos)
                return;
            p.pickerPos = value;
            _sampleUpdate();
            _widgetUpdate();
        }

        void ColorPickerWidget::_initEvent(Event::Init & event)
        {
            MDIWidget::_initEvent(event);
            DJV_PRIVATE_PTR();
            if (event.getData().text)
            {
                setTitle(_getText(DJV_TEXT("widget_color_picker")));

                p.actions["LockType"]->setText(_getText(DJV_TEXT("widget_color_picker_lock_color_type")));
                p.actions["LockType"]->setTooltip(_getText(DJV_TEXT("widget_color_picker_lock_color_type_tooltip")));
                p.actions["ApplyColorOperations"]->setText(_getText(DJV_TEXT("widget_color_picker_apply_color_operations")));
                p.actions["ApplyColorOperations"]->setTooltip(_getText(DJV_TEXT("widget_color_picker_apply_color_operations_tooltip")));
                p.actions["ApplyColorSpace"]->setText(_getText(DJV_TEXT("widget_color_picker_apply_color_space")));
                p.actions["ApplyColorSpace"]->setTooltip(_getText(DJV_TEXT("widget_color_picker_apply_color_space_tooltip")));

                p.sampleSizeSlider->setTooltip(_getText(DJV_TEXT("widget_color_picker_sample_size_tooltip")));

                p.copyButton->setTooltip(_getText(DJV_TEXT("widget_color_picker_copy_tooltip")));

                p.settingsPopupMenu->setTooltip(_getText(DJV_TEXT("widget_color_picker_settings_tooltip")));

                p.formLayout->setText(p.colorLabel, _getText(DJV_TEXT("widget_color_picker_color")) + ":");
                p.formLayout->setText(p.pixelLabel, _getText(DJV_TEXT("widget_color_picker_pixel")) + ":");
                p.formLayout->setText(p.sampleSizeSlider, _getText(DJV_TEXT("widget_color_picker_sample_size")) + ":");
            }
        }
        
        void ColorPickerWidget::_sampleUpdate()
        {
            DJV_PRIVATE_PTR();
            glm::vec3 pixelPos(0.F, 0.F, 1.F);
            if (p.image && p.image->isValid())
            {
                try
                {
                    glm::mat3x3 m(1.F);
                    m = glm::translate(m, -(p.pickerPos / p.imageZoom));
                    const float z = p.sampleSize / 2.F;
                    m = glm::translate(m, glm::vec2(z, z));
                    m = glm::translate(m, p.imagePos / p.imageZoom);
                    m *= UI::ImageWidget::getXForm(
                        p.image,
                        p.imageRotate,
                        glm::vec2(1.F, 1.F),
                        p.imageAspectRatio);
                    pixelPos = glm::inverse(glm::translate(m, glm::vec2(-.5F, -.5F))) * pixelPos;

                    const size_t sampleSize = std::max(p.sampleSize, bufferSizeMin);
                    const AV::Image::Size size(sampleSize, sampleSize);
                    const AV::Image::Type type = p.lockType != AV::Image::Type::None ? p.lockType : p.image->getType();
                    
                    bool create = !p.offscreenBuffer;
                    create |= p.offscreenBuffer && size != p.offscreenBuffer->getSize();
                    create |= p.offscreenBuffer && type != p.offscreenBuffer->getColorType();
                    if (create)
                    {
                        p.offscreenBuffer = AV::OpenGL::OffscreenBuffer::create(size, type);
                    }

                    p.offscreenBuffer->bind();
                    const auto& render = _getRender();
                    const auto imageFilterOptions = render->getImageFilterOptions();
                    render->setImageFilterOptions(AV::Render2D::ImageFilterOptions(AV::Render2D::ImageFilter::Nearest));
                    render->beginFrame(size);
                    render->setFillColor(AV::Image::Color(0.F, 0.F, 0.F));
                    render->drawRect(BBox2f(0.F, 0.F, sampleSize, sampleSize));
                    render->setFillColor(AV::Image::Color(1.F, 1.F, 1.F));
                    render->pushTransform(m);
                    auto options = p.imageOptions;
                    if (!p.applyColorOperations)
                    {
                        options.colorEnabled    = false;
                        options.levelsEnabled   = false;
                        options.exposureEnabled = false;
                        options.softClipEnabled = false;
                    }
                    if (p.applyColorSpace)
                    {
                        auto i = p.ocioConfig.fileColorSpaces.find(p.image->getPluginName());
                        if (i != p.ocioConfig.fileColorSpaces.end())
                        {
                            options.colorSpace.input = i->second;
                        }
                        else
                        {
                            i = p.ocioConfig.fileColorSpaces.find(std::string());
                            if (i != p.ocioConfig.fileColorSpaces.end())
                            {
                                options.colorSpace.input = i->second;
                            }
                        }
                        options.colorSpace.output = p.outputColorSpace;
                    }
                    options.cache = AV::Render2D::ImageCache::Dynamic;
                    render->drawImage(p.image, glm::vec2(0.F, 0.F), options);
                    render->popTransform();
                    render->endFrame();
                    render->setImageFilterOptions(imageFilterOptions);
                    auto data = AV::Image::Data::create(AV::Image::Info(p.sampleSize, p.sampleSize, type));
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
#if !defined(DJV_OPENGL_ES2)  // \todo GL_READ_FRAMEBUFFER, glClampColor not in OpenGL ES 2
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, p.offscreenBuffer->getID());
                    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
#endif
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    glReadPixels(
                        0,
                        static_cast<int>(sampleSize) - static_cast<int>(data->getHeight()),
                        data->getWidth(),
                        data->getHeight(),
                        AV::Image::getGLFormat(type),
                        AV::Image::getGLType(type),
                        data->getData());
                    p.color = AV::Image::getAverageColor(data);
                }
                catch (const std::exception& e)
                {
                    std::vector<std::string> messages;
                    messages.push_back(_getText(DJV_TEXT("error_cannot_sample_color")));
                    messages.push_back(e.what());
                    _log(String::join(messages, ' '), LogLevel::Error);
                }
            }
            else if (p.offscreenBuffer)
            {
                p.offscreenBuffer.reset();
            }
            switch (p.imageRotate)
            {
            /*case UI::ImageRotate::_90:
            {
                const float tmp = p.pixelPos.x;
                p.pixelPos.x = pixelPos.y;
                p.pixelPos.y = tmp;
                break;
            }*/
            default:
                p.pixelPos.x = pixelPos.x;
                p.pixelPos.y = pixelPos.y;
                break;
            }
        }

        void ColorPickerWidget::_widgetUpdate()
        {
            DJV_PRIVATE_PTR();

            const AV::Image::Type type = p.color.getType();
            p.typeWidget->setType(type);

            const bool lockType = p.lockType != AV::Image::Type::None;
            p.actions["LockType"]->setChecked(lockType);
            p.actions["ApplyColorOperations"]->setChecked(p.applyColorOperations);
            p.actions["ApplyColorSpace"]->setChecked(p.applyColorSpace);

            p.colorSwatch->setColor(p.color);
            p.colorLabel->setText(AV::Image::Color::getLabel(p.color, 2, false));
            p.colorLabel->setTooltip(_getText(DJV_TEXT("color_label_tooltip")));
            {
                std::stringstream ss;
                ss << static_cast<int>(floorf(p.pixelPos.x)) << " " << static_cast<int>(floorf(p.pixelPos.y));
                p.pixelLabel->setText(ss.str());
            }
            p.pixelLabel->setTooltip(_getText(DJV_TEXT("pixel_label_tooltip")));
            p.sampleSizeSlider->setValue(p.sampleSize);
        }

    } // namespace ViewApp
} // namespace djv

