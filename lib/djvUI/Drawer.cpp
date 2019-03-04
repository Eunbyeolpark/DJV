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

#include <djvUI/Drawer.h>

#include <djvUI/StackLayout.h>

#include <djvAV/Render2D.h>

using namespace djv::Core;

namespace djv
{
    namespace UI
    {
        namespace Layout
        {
            struct Drawer::Private
            {
                bool open = false;
                Side side = Side::First;
                std::shared_ptr<Stack> layout;
            };

            void Drawer::_init(Context * context)
            {
                IContainer::_init(context);

                setClassName("djv::UI::Layout::Drawer");

                _p->layout = Stack::create(context);
                IContainer::addWidget(_p->layout);
            }

            Drawer::Drawer() :
                _p(new Private)
            {}

            Drawer::~Drawer()
            {}

            std::shared_ptr<Drawer> Drawer::create(Context * context)
            {
                auto out = std::shared_ptr<Drawer>(new Drawer);
                out->_init(context);
                return out;
            }

            bool Drawer::isOpen() const
            {
                return _p->open;
            }

            void Drawer::setOpen(bool value)
            {
                DJV_PRIVATE_PTR();
                if (value == p.open)
                    return;
                p.open = value;
                _resize();
            }

            void Drawer::open()
            {
                setOpen(true);
            }

            void Drawer::close()
            {
                setOpen(false);
            }

            Side Drawer::getSide() const
            {
                return _p->side;
            }

            void Drawer::setSide(Side value)
            {
                DJV_PRIVATE_PTR();
                if (value == p.side)
                    return;
                p.side = value;
                _resize();
            }

            void Drawer::addWidget(const std::shared_ptr<Widget>& value)
            {
                DJV_PRIVATE_PTR();
                p.layout->addWidget(value);
            }

            void Drawer::removeWidget(const std::shared_ptr<Widget>& value)
            {
                _p->layout->removeWidget(value);
            }

            void Drawer::clearWidgets()
            {
                _p->layout->clearWidgets();
            }

            void Drawer::_preLayoutEvent(Event::PreLayout& event)
            {
                _setMinimumSize(_p->layout->getMinimumSize());
            }

            void Drawer::_layoutEvent(Event::Layout& event)
            {
                DJV_PRIVATE_PTR();
                const BBox2f & g = getGeometry();
                const glm::vec2 & minimumSize = p.layout->getMinimumSize();
                BBox2f childGeometry = g;
                switch (p.side)
                {
                case Side::Left:
                    childGeometry.max.x = p.open ? (g.min.x + minimumSize.x) : g.min.x;
                    break;
                case Side::Top:
                    childGeometry.max.y = p.open ? (g.min.y + minimumSize.y) : g.min.y;
                    break;
                case Side::Right:
                    childGeometry.min.x = p.open ? (g.max.x - minimumSize.x) : g.max.x;
                    break;
                case Side::Bottom:
                    childGeometry.min.y = p.open ? (g.max.y - minimumSize.y) : g.max.y;
                    break;
                default: break;
                }
                _p->layout->setGeometry(childGeometry);
            }

            void Drawer::_paintEvent(Event::Paint& event)
            {
                Widget::_paintEvent(event);
                DJV_PRIVATE_PTR();
                if (auto render = _getRender().lock())
                {
                    if (auto style = _getStyle().lock())
                    {
                        if (p.open)
                        {
                            const BBox2f & g = _p->layout->getGeometry();
                            const float b = style->getMetric(MetricsRole::Border);
                            render->setFillColor(_getColorWithOpacity(style->getColor(ColorRole::Border)));
                            switch (p.side)
                            {
                            case Side::Left:
                                render->drawRect(BBox2f(g.max.x + b, g.min.y, b, g.h()));
                                break;
                            case Side::Top:
                                render->drawRect(BBox2f(g.min.x, g.max.y, g.w(), b));
                                break;
                            case Side::Right:
                                render->drawRect(BBox2f(g.min.x - b, g.min.y, b, g.h()));
                                break;
                            case Side::Bottom:
                                render->drawRect(BBox2f(g.min.x, g.min.y - b, g.w(), b));
                                break;
                            default: break;
                            }
                        }
                    }
                }
            }

        } // namespace Layout
    } // namespace UI
} // namespace djv