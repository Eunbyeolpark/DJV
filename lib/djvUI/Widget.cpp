//------------------------------------------------------------------------------
// Copyright (c) 2018 Darby Johnston
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

#include <djvUI/Widget.h>

#include <djvUI/Action.h>
#include <djvUI/Shortcut.h>
#include <djvUI/Window.h>

#include <djvAV/FontSystem.h>
#include <djvAV/IconSystem.h>
#include <djvAV/Render2DSystem.h>

#include <djvCore/Math.h>

#include <algorithm>

//#pragma optimize("", off)

using namespace djv::Core;

namespace
{
    static size_t currentWidgetCount = 0;

} // namespace

namespace djv
{
    namespace UI
    {
        void Widget::_init(Core::Context * context)
        {
            IObject::_init(context);
            
            setClassName("djv::UI::Widget");

            /*context->log("djv::UI::Widget", "Widget::Widget");
            context->log("djv::UI::Widget", String::Format("widget count = %%1").arg(currentWidgetCount));*/
            ++currentWidgetCount;

            _iconSystem = context->getSystemT<AV::Image::IconSystem>();
            _fontSystem = context->getSystemT<AV::Font::System>();
            _renderSystem = context->getSystemT<AV::Render::Render2DSystem>();
            _style = context->getSystemT<Style::Style>();
        }

        Widget::~Widget()
        {
            --currentWidgetCount;
            /*if (auto context = getContext().lock())
            {
              context->log("djv::UI::Widget", "Widget::~Widget");
              context->log("djv::UI::Widget", String::Format("widget count = %%1").arg(currentWidgetCount));
            }*/
        }
        
        size_t Widget::getCurrentWidgetCount()
        {
            return currentWidgetCount;
        }

        std::shared_ptr<Widget> Widget::create(Context * context)
        {
            //! \bug It would be prefereable to use std::make_shared() here, but how can we do that
            //! with protected contructors?
            auto out = std::shared_ptr<Widget>(new Widget);
            out->_init(context);
            return out;
        }

        std::weak_ptr<Window> Widget::getWindow()
        {
            auto widget = std::dynamic_pointer_cast<Widget>(shared_from_this());
            while (widget)
            {
                if (auto parent = std::dynamic_pointer_cast<Widget>(widget->getParent().lock()))
                {
                    widget = parent;
                }
                else
                {
                    break;
                }
            }
            return std::dynamic_pointer_cast<Window>(std::const_pointer_cast<Widget>(widget));
        }

        void Widget::setVisible(bool value)
        {
            if (value == _visible)
                return;
            _visible = value;
            _redraw();
        }

        void Widget::setOpacity(float value)
        {
            if (fuzzyCompare(value, _opacity))
                return;
            _opacity = value;
            _redraw();
        }

        void Widget::show()
        {
            setVisible(true);
        }

        void Widget::hide()
        {
            setVisible(false);
        }

        void Widget::setGeometry(const BBox2f& value)
        {
            if (fuzzyCompare(value, _geometry))
                return;
            _geometry = value;
            _resize();
        }

        void Widget::move(const glm::vec2& value)
        {
            const glm::vec2 size = _geometry.getSize();
            BBox2f geometry;
            geometry.min = value;
            geometry.max = value + size;
            setGeometry(geometry);
        }

        void Widget::resize(const glm::vec2& value)
        {
            setGeometry(BBox2f(_geometry.min, _geometry.min + value));
        }

        void Widget::setMargin(const Layout::Margin& value)
        {
            if (value == _margin)
                return;
            _margin = value;
            _resize();
        }

        void Widget::setHAlign(HAlign value)
        {
            if (value == _hAlign)
                return;
            _hAlign = value;
            _resize();
        }

        void Widget::setVAlign(VAlign value)
        {
            if (value == _vAlign)
                return;
            _vAlign = value;
            _resize();
        }

        BBox2f Widget::getAlign(const BBox2f& value, const glm::vec2& minimumSize, HAlign hAlign, VAlign vAlign)
        {
            float x = 0.f;
            float y = 0.f;
            float w = 0.f;
            float h = 0.f;
            switch (hAlign)
            {
            case HAlign::Center:
                w = minimumSize.x;
                x = value.min.x + value.w() / 2.f - minimumSize.x / 2.f;
                break;
            case HAlign::Left:
                x = value.min.x;
                w = minimumSize.x;
                break;
            case HAlign::Right:
                w = minimumSize.x;
                x = value.min.x + value.w() - minimumSize.x;
                break;
            case HAlign::Fill:
                x = value.min.x;
                w = value.w();
                break;
            default: break;
            }
            switch (vAlign)
            {
            case VAlign::Center:
                h = minimumSize.y;
                y = value.min.y + value.h() / 2.f - minimumSize.y / 2.f;
                break;
            case VAlign::Top:
                y = value.min.y;
                h = minimumSize.y;
                break;
            case VAlign::Bottom:
                h = minimumSize.y;
                y = value.min.y + value.h() - minimumSize.y;
                break;
            case VAlign::Fill:
                y = value.min.y;
                h = value.h();
                break;
            default: break;
            }
            return BBox2f(x, y, w, h);
        }

        void Widget::setBackgroundRole(Style::ColorRole value)
        {
            if (value == _backgroundRole)
                return;
            _backgroundRole = value;
            _redraw();
        }

        void Widget::setPointerEnabled(bool value)
        {
            _pointerEnabled = value;
        }

        void Widget::addAction(const std::shared_ptr<Action>& action)
        {
            _actions.push_back(action);
        }

        void Widget::removeAction(const std::shared_ptr<Action>& action)
        {
            const auto i = std::find(_actions.begin(), _actions.end(), action);
            if (i != _actions.end())
            {
                _actions.erase(i);
            }
        }

        void Widget::clearActions()
        {
            _actions.clear();
        }

        bool Widget::event(Event::IEvent& event)
        {
            bool out = IObject::event(event);
            if (!out)
            {
                switch (event.getEventType())
                {
                case Event::Type::PreLayout:
                    _preLayoutEvent(static_cast<Event::PreLayout&>(event));
                    break;
                case Event::Type::Layout:
                    _layoutEvent(static_cast<Event::Layout&>(event));
                    break;
                case Event::Type::Clip:
                {
                    auto& clipEvent = static_cast<Event::Clip&>(event);
                    if (auto parent = std::dynamic_pointer_cast<Widget>(getParent().lock()))
                    {
                        _parentsVisible = parent->_visible && parent->_parentsVisible;
                        _clipped = !clipEvent.getClipRect().isValid() || !_visible || !parent->_visible || !parent->_parentsVisible;
                    }
                    else
                    {
                        _parentsVisible = true;
                        _clipped = false;
                    }
                    _clipEvent(clipEvent);
                    break;
                }
                case Event::Type::Paint:
                {
                    if (auto parent = std::dynamic_pointer_cast<Widget>(getParent().lock()))
                    {
                        _parentsOpacity = parent->_opacity * parent->_parentsOpacity;
                    }
                    else
                    {
                        _parentsOpacity = 1.f;
                    }
                    _paintEvent(static_cast<Event::Paint&>(event));
                    break;
                }
                case Event::Type::PointerEnter:
                    _pointerEnterEvent(static_cast<Event::PointerEnter&>(event));
                    break;
                case Event::Type::PointerLeave:
                    _pointerLeaveEvent(static_cast<Event::PointerLeave&>(event));
                    break;
                case Event::Type::PointerMove:
                    _pointerMoveEvent(static_cast<Event::PointerMove&>(event));
                    break;
                case Event::Type::ButtonPress:
                    _buttonPressEvent(static_cast<Event::ButtonPress&>(event));
                    break;
                case Event::Type::ButtonRelease:
                    _buttonReleaseEvent(static_cast<Event::ButtonRelease&>(event));
                    break;
                case Event::Type::Scroll:
                    _scrollEvent(static_cast<Event::Scroll&>(event));
                    break;
                case Event::Type::Drop:
                    _dropEvent(static_cast<Event::Drop&>(event));
                    break;
                case Event::Type::KeyboardFocus:
                    _keyboardFocusEvent(static_cast<Event::KeyboardFocus&>(event));
                    break;
                case Event::Type::KeyboardFocusLost:
                    _keyboardFocusLostEvent(static_cast<Event::KeyboardFocusLost&>(event));
                    break;
                case Event::Type::KeyPress:
                    _keyPressEvent(static_cast<Event::KeyPress&>(event));
                    break;
                case Event::Type::KeyRelease:
                    _keyReleaseEvent(static_cast<Event::KeyRelease&>(event));
                    break;
                case Event::Type::Text:
                    _textEvent(static_cast<Event::Text&>(event));
                    break;
                default: break;
                }
                out = event.isAccepted();
            }
            return out;
        }

        void Widget::_paintEvent(Event::Paint& event)
        {
            if (_backgroundRole != Style::ColorRole::None)
            {
                if (auto render = _renderSystem.lock())
                {
                    if (auto style = _style.lock())
                    {
                        render->setFillColor(_getColorWithOpacity(style->getColor(_backgroundRole)));
                        render->drawRectangle(getGeometry());
                    }
                }
            }
        }

        void Widget::_pointerEnterEvent(Event::PointerEnter& event)
        {
            if (_pointerEnabled && !event.isRejected())
            {
                event.accept();
            }
        }

        void Widget::_pointerLeaveEvent(Event::PointerLeave& event)
        {
            if (_pointerEnabled)
            {
                event.accept();
            }
        }

        void Widget::_pointerMoveEvent(Event::PointerMove& event)
        {
            if (_pointerEnabled)
            {
                event.accept();
            }
        }

        void Widget::_keyPressEvent(Event::KeyPress& event)
        {
            if (isEnabled())
            {
                // Find the actions that are enabled and have shortcuts.
                std::vector<std::shared_ptr<Action> > actions;
                for (const auto& i : _actions)
                {
                    if (i->isEnabled()->get())
                    {
                        if (i->getShortcut())
                        {
                            actions.push_back(i);
                        }
                    }
                }

                // Sort the actions so that we test those with keyboard modifiers first.
                std::sort(actions.begin(), actions.end(),
                    [](const std::shared_ptr<Action> & a, const std::shared_ptr<Action> & b) -> bool
                {
                    return a->getShortcut()->getShortcutModifiers()->get() > b->getShortcut()->getShortcutModifiers()->get();
                });

                for (const auto& i : actions)
                {
                    const int key = i->getShortcut()->getShortcutKey()->get();
                    const int modifiers = i->getShortcut()->getShortcutModifiers()->get();
                    if ((key == event.getKey() && event.getKeyModifiers() & modifiers) ||
                        (key == event.getKey() && modifiers == 0 && event.getKeyModifiers() == 0))
                    {
                        event.accept();
                        i->getShortcut()->doCallback();
                        break;
                    }
                }
            }
        }

        AV::Image::Color Widget::_getColorWithOpacity(const AV::Image::Color & value) const
        {
            auto out = value.convert(AV::Image::Type::RGBA_F32);
            out.setF32(out.getF32(3) * getOpacity(true), 3);
            return out;
        }

        void Widget::_redraw()
        {
            _redrawRequest = true;
        }

        void Widget::_resize()
        {
            _resizeRequest = true;
        }

        void Widget::_setMinimumSize(const glm::vec2& value)
        {
            if (value == _minimumSize)
                return;
            _minimumSize = value;
            _resize();
        }

        void Widget::_parentChangedEvent(Event::ParentChanged & event)
        {
            _clipped = event.getNewParent() ? true : false;
            _redraw();
        }

        void Widget::_childAddedEvent(Event::ChildAdded &)
        {
            _redraw();
        }

        void Widget::_childRemovedEvent(Event::ChildRemoved &)
        {
            _redraw();
        }

    } // namespace UI
} // namespace djv
