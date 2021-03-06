// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvUI/CheckBox.h>

#include <djvUI/DrawUtil.h>
#include <djvUI/Label.h>
#include <djvUI/Style.h>

#include <djvAV/Render2D.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

//#pragma optimize("", off)

using namespace djv::Core;

namespace djv
{
    namespace UI
    {
        namespace Button
        {
            struct CheckBox::Private
            {
                std::shared_ptr<Label> label;
            };

            void CheckBox::_init(const std::shared_ptr<Context>& context)
            {
                IButton::_init(context);

                DJV_PRIVATE_PTR();
                setClassName("djv::UI::Button::CheckBox");
                setButtonType(UI::ButtonType::Toggle);

                p.label = Label::create(context);
                p.label->setTextHAlign(TextHAlign::Left);
                p.label->setTextColorRole(getForegroundColorRole());
                p.label->setMargin(MetricsRole::MarginSmall);
                addChild(p.label);

                _widgetUpdate();
            }

            CheckBox::CheckBox() :
                _p(new Private)
            {}

            CheckBox::~CheckBox()
            {}

            std::shared_ptr<CheckBox> CheckBox::create(const std::shared_ptr<Context>& context)
            {
                auto out = std::shared_ptr<CheckBox>(new CheckBox);
                out->_init(context);
                return out;
            }

            std::string CheckBox::getText() const
            {
                DJV_PRIVATE_PTR();
                return p.label ? p.label->getText() : std::string();
            }

            void CheckBox::setText(const std::string & value)
            {
                DJV_PRIVATE_PTR();
                if (value == p.label->getText())
                    return;
                p.label->setText(value);
                _widgetUpdate();
            }

            const std::string & CheckBox::getFontFamily() const
            {
                return _p->label->getFontFamily();
            }

            const std::string & CheckBox::getFontFace() const
            {
                return _p->label->getFontFace();
            }

            MetricsRole CheckBox::getFontSizeRole() const
            {
                return _p->label->getFontSizeRole();
            }

            void CheckBox::setFontFamily(const std::string & value)
            {
                _p->label->setFontFamily(value);
            }

            void CheckBox::setFontFace(const std::string & value)
            {
                _p->label->setFontFace(value);
            }

            void CheckBox::setFontSizeRole(MetricsRole value)
            {
                _p->label->setFontSizeRole(value);
            }

            bool CheckBox::acceptFocus(TextFocusDirection)
            {
                bool out = false;
                if (isEnabled(true) && isVisible(true) && !isClipped())
                {
                    takeTextFocus();
                    out = true;
                }
                return out;
            }

            void CheckBox::_preLayoutEvent(Event::PreLayout & event)
            {
                const auto& style = _getStyle();
                const float m = style->getMetric(MetricsRole::MarginInside);
                const float b = style->getMetric(MetricsRole::Border);
                const float is = style->getMetric(MetricsRole::IconSmall);
                glm::vec2 size = _p->label->getMinimumSize();
                size.x += is + m * 2.F + b * 4.F;
                size.y = std::max(size.y, is + m * 2.F + b * 4.F);
                _setMinimumSize(size);
            }

            void CheckBox::_layoutEvent(Event::Layout &)
            {
                _p->label->setGeometry(_getLabelGeometry());
            }

            void CheckBox::_paintEvent(Event::Paint& event)
            {
                IButton::_paintEvent(event);
                const auto& render = _getRender();
                const auto& style = _getStyle();
                const BBox2f& g = getGeometry();
                const float m = style->getMetric(MetricsRole::MarginInside);
                const float b = style->getMetric(MetricsRole::Border);

                if (_isPressed())
                {
                    render->setFillColor(style->getColor(ColorRole::Pressed));
                    render->drawRect(g);
                }
                else if (_isHovered())
                {
                    render->setFillColor(style->getColor(ColorRole::Hovered));
                    render->drawRect(g);
                }

                if (hasTextFocus())
                {
                    render->setFillColor(style->getColor(ColorRole::TextFocus));
                    drawBorder(render, g, b * 2.F);
                }

                BBox2f checkGeometry = _getCheckGeometry().margin(-m);
                render->setFillColor(style->getColor(ColorRole::Border));
                drawBorder(render, checkGeometry, b);
                render->setFillColor(style->getColor(_isToggled() ? ColorRole::Checked : ColorRole::Trough));
                render->drawRect(checkGeometry.margin(-b));
            }

            void CheckBox::_buttonPressEvent(Event::ButtonPress& event)
            {
                IButton::_buttonPressEvent(event);
                if (event.isAccepted())
                {
                    takeTextFocus();
                }
            }

            void CheckBox::_keyPressEvent(Event::KeyPress& event)
            {
                IButton::_keyPressEvent(event);
                if (!event.isAccepted() && hasTextFocus())
                {
                    switch (event.getKey())
                    {
                    case GLFW_KEY_ENTER:
                    case GLFW_KEY_SPACE:
                        event.accept();
                        setChecked(!isChecked());
                        _doCheckedCallback(isChecked());
                        break;
                    case GLFW_KEY_ESCAPE:
                        event.accept();
                        releaseTextFocus();
                        break;
                    default: break;
                    }
                }
            }

            void CheckBox::_textFocusEvent(Event::TextFocus&)
            {
                _redraw();
            }

            void CheckBox::_textFocusLostEvent(Event::TextFocusLost&)
            {
                _redraw();
            }

            BBox2f CheckBox::_getCheckGeometry() const
            {
                const auto& style = _getStyle();
                const BBox2f& g = getGeometry();
                const float b = style->getMetric(MetricsRole::Border);
                const float m = style->getMetric(MetricsRole::MarginInside);
                const float is = style->getMetric(MetricsRole::IconSmall);
                const float size = is + m * 2.F;
                const BBox2f g2 = g.margin(-b);
                return BBox2f(g2.min.x, floorf(g2.min.y + g2.h() / 2.F - size / 2.F), size, size);
            }

            BBox2f CheckBox::_getLabelGeometry() const
            {
                const auto& style = _getStyle();
                const BBox2f& g = getGeometry();
                const float b = style->getMetric(MetricsRole::Border);
                const float m = style->getMetric(MetricsRole::MarginInside);
                const float is = style->getMetric(MetricsRole::IconSmall);
                const float size = is + m * 2.F;
                const BBox2f g2 = g.margin(-b);
                return BBox2f(g2.min.x + size, g2.min.y, g2.w() - size, g2.h());
            }

            void CheckBox::_widgetUpdate()
            {
                DJV_PRIVATE_PTR();
                p.label->setVisible(!p.label->getText().empty());
            }

        } // namespace Button
    } // namespace UI
} // namespace Gp
