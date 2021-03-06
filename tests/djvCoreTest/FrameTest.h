// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2004-2020 Darby Johnston
// All rights reserved.

#pragma once

#include <djvTestLib/Test.h>

namespace djv
{
    namespace CoreTest
    {
        class FrameTest : public Test::ITest
        {
        public:
            FrameTest(const std::shared_ptr<Core::Context>&);
            
            void run() override;
            
        private:
            void _sequence();
            void _conversion();
            void _serialize();
        };
        
    } // namespace CoreTest
} // namespace djv

