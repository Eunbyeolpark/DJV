// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#include <djvUI/NumericEdit.h>

#include <djvUI/DrawUtil.h>
#include <djvUI/LineEditBase.h>
#include <djvUI/RowLayout.h>
#include <djvUI/ToolButton.h>

#include <djvAV/FontSystem.h>
#include <djvAV/Render2D.h>

#include <djvCore/NumericValueModels.h>
#include <djvCore/ValueObserver.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace djv::Core;

namespace djv
{
    namespace UI
    {
        struct NumericEditButtons::Private
        {
            std::shared_ptr<ToolButton> incButtons[2];
            std::function<void(void)> incrementCallback;
            std::function<void(void)> decrementCallback;
        };

        void NumericEditButtons::_init(const std::shared_ptr<Context>& context)
        {
            Widget::_init(context);
            DJV_PRIVATE_PTR();

            setClassName("djv::UI::NumericEditButtons");
            setBackgroundRole(ColorRole::Button);

            const std::vector<std::string> icons =
            {
                "djvIconIncrement",
                "djvIconDecrement"
            };
            for (size_t i = 0; i < 2; ++i)
            {
                p.incButtons[i] = ToolButton::create(context);
                p.incButtons[i]->setIcon(icons[i]);
                p.incButtons[i]->setIconSizeRole(MetricsRole::IconMini);
                p.incButtons[i]->setInsideMargin(MetricsRole::Border);
                p.incButtons[i]->setAutoRepeat(true);
                addChild(p.incButtons[i]);
            }

            auto weak = std::weak_ptr<NumericEditButtons>(std::dynamic_pointer_cast<NumericEditButtons>(shared_from_this()));
            p.incButtons[0]->setClickedCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        if (widget->_p->incrementCallback)
                        {
                            widget->_p->incrementCallback();
                        }
                    }
                });
            p.incButtons[1]->setClickedCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        if (widget->_p->decrementCallback)
                        {
                            widget->_p->decrementCallback();
                        }
                    }
                });
        }

        NumericEditButtons::NumericEditButtons() :
            _p(new Private)
        {}

        NumericEditButtons::~NumericEditButtons()
        {}

        std::shared_ptr<NumericEditButtons> NumericEditButtons::create(const std::shared_ptr<Context>& value)
        {
            auto out = std::shared_ptr<NumericEditButtons>(new NumericEditButtons);
            out->_init(value);
            return out;
        }

        void NumericEditButtons::setIncrementEnabled(bool value)
        {
            _p->incButtons[0]->setEnabled(value);
        }

        void NumericEditButtons::setDecrementEnabled(bool value)
        {
            _p->incButtons[1]->setEnabled(value);
        }

        void NumericEditButtons::setIncrementCallback(const std::function<void(void)>& callback)
        {
            _p->incrementCallback = callback;
        }

        void NumericEditButtons::setDecrementCallback(const std::function<void(void)>& callback)
        {
            _p->decrementCallback = callback;
        }

        void NumericEditButtons::_preLayoutEvent(Event::PreLayout&)
        {
            DJV_PRIVATE_PTR();
            glm::vec2 size = glm::vec2(0.F, 0.F);
            for (size_t i = 0; i < 2; ++i)
            {
                const auto& tmp = p.incButtons[i]->getMinimumSize();
                size.x = std::max(size.x, tmp.x);
                size.y += tmp.y;
            }
            _setMinimumSize(size);
        }

        void NumericEditButtons::_layoutEvent(Event::Layout&)
        {
            DJV_PRIVATE_PTR();
            const BBox2f& g = getGeometry();
            float x = g.min.x;
            float y = g.min.y;
            float w = g.w();
            float h = ceilf(g.h() / 2.F);
            p.incButtons[0]->setGeometry(BBox2f(x, y, w, h));
            y = g.max.y - h;
            p.incButtons[1]->setGeometry(BBox2f(x, y, w, h));
        }

        struct NumericEdit::Private
        {
            std::shared_ptr<LineEditBase> lineEditBase;
            std::shared_ptr<NumericEditButtons> buttons;
        };

        void NumericEdit::_init(const std::shared_ptr<Context>& context)
        {
            Widget::_init(context);
            DJV_PRIVATE_PTR();

            setClassName("djv::UI::NumericEdit");
            setVAlign(VAlign::Center);

            p.lineEditBase = LineEditBase::create(context);
            p.lineEditBase->setFont(AV::Font::familyMono);
            p.lineEditBase->installEventFilter(shared_from_this());
            addChild(p.lineEditBase);

            p.buttons = NumericEditButtons::create(context);
            addChild(p.buttons);

            auto weak = std::weak_ptr<NumericEdit>(std::dynamic_pointer_cast<NumericEdit>(shared_from_this()));
            p.lineEditBase->setTextEditCallback(
                [weak](const std::string & value, TextEditReason reason)
            {
                if (auto widget = weak.lock())
                {
                    widget->_textEdit(value, reason);
                }
            });
            p.lineEditBase->setFocusCallback(
                [weak](bool)
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_redraw();
                    }
                });

            p.buttons->setIncrementCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_incrementValue();
                    }
                });
            p.buttons->setDecrementCallback(
                [weak]
                {
                    if (auto widget = weak.lock())
                    {
                        widget->_decrementValue();
                    }
                });
        }

        NumericEdit::NumericEdit() :
            _p(new Private)
        {}

        NumericEdit::~NumericEdit()
        {}

        void NumericEdit::_preLayoutEvent(Event::PreLayout & event)
        {
            DJV_PRIVATE_PTR();
            glm::vec2 size = glm::vec2(0.F, 0.F);
            glm::vec2 tmp = p.lineEditBase->getMinimumSize();
            size.x += tmp.x;
            size.y = std::max(size.y, tmp.y);
            tmp = p.buttons->getMinimumSize();
            size.x += tmp.x;
            size.y = std::max(size.y, tmp.y);
            const auto& style = _getStyle();
            const float b = style->getMetric(MetricsRole::Border);
            _setMinimumSize(size + b * 6.F + getMargin().getSize(style));
        }

        void NumericEdit::_layoutEvent(Event::Layout & event)
        {
            DJV_PRIVATE_PTR();
            const auto& style = _getStyle();
            const BBox2f& g = getMargin().bbox(getGeometry(), style);
            const float b = style->getMetric(MetricsRole::Border);
            glm::vec2 tmp = p.buttons->getMinimumSize();
            BBox2f g2 = g.margin(-b * 3.F);
            float x = g2.max.x - tmp.x;
            float y = g2.min.y;
            float w = tmp.x;
            float h = g2.h();
            p.buttons->setGeometry(BBox2f(x, y, w, h));
            x = g2.min.x;
            w = g2.w() - tmp.x;
            p.lineEditBase->setGeometry(BBox2f(x, y, w, h));
        }

        void NumericEdit::_textUpdate(const std::string& text, const std::string& sizeString)
        {
            DJV_PRIVATE_PTR();
            p.lineEditBase->setText(text);
            p.lineEditBase->setSizeString(sizeString);
        }

        void NumericEdit::_setIsMin(bool value)
        {
            DJV_PRIVATE_PTR();
            p.buttons->setDecrementEnabled(!value);
        }

        void NumericEdit::_setIsMax(bool value)
        {
            DJV_PRIVATE_PTR();
            p.buttons->setIncrementEnabled(!value);
        }

        void NumericEdit::_paintEvent(Event::Paint& event)
        {
            Widget::_paintEvent(event);
            DJV_PRIVATE_PTR();
            const auto& style = _getStyle();
            const BBox2f& g = getGeometry();
            const float b = style->getMetric(UI::MetricsRole::Border);
            const auto& render = _getRender();
            if (p.lineEditBase->hasTextFocus())
            {
                render->setFillColor(style->getColor(UI::ColorRole::TextFocus));
                drawBorder(render, g, b * 2.F);
            }
            render->setFillColor(style->getColor(UI::ColorRole::Border));
            const BBox2f g2 = g.margin(-b * 2.F);
            drawBorder(render, g2, b);
        }

        bool NumericEdit::_eventFilter(const std::shared_ptr<IObject>& object, Event::Event& event)
        {
            DJV_PRIVATE_PTR();
            switch (event.getEventType())
            {
            case Event::Type::KeyPress:
            {
                auto& keyPressEvent = static_cast<Event::KeyPress&>(event);
                if (_keyPress(keyPressEvent.getKey()))
                {
                    keyPressEvent.accept();
                    return true;
                }
                break;
            }
            case Event::Type::Scroll:
            {
                auto& scrollEvent = static_cast<Event::Scroll&>(event);
                scrollEvent.accept();
                p.lineEditBase->takeTextFocus();
                _scroll(scrollEvent.getScrollDelta().y);
                return true;
            }
            default: break;
            }
            return false;
         }

    } // namespace UI
} // namespace djv
