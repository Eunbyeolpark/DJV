// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2019-2020 Darby Johnston
// All rights reserved.

#pragma once

#include "ISettingsWidget.h"

#include <djvUIComponents/SceneWidget.h>

#include <djvUI/FloatEdit.h>
#include <djvUI/FormLayout.h>
#include <djvUI/GroupBox.h>

class CameraWidget : public ISettingsWidget
{
    DJV_NON_COPYABLE(CameraWidget);

protected:
    void _init(const std::shared_ptr<djv::Core::Context>&);
    CameraWidget();

public:
    virtual ~CameraWidget();

    static std::shared_ptr<CameraWidget> create(const std::shared_ptr<djv::Core::Context>&);

    void setCameraData(const djv::Scene::PolarCameraData&);
    void setCameraDataCallback(const std::function<void(const djv::Scene::PolarCameraData&)>&);

    void setLabelSizeGroup(const std::weak_ptr<djv::UI::LabelSizeGroup>&) override;

protected:
    void _initEvent(djv::Core::Event::Init&) override;

private:
    void _widgetUpdate();

    djv::Scene::PolarCameraData _cameraData;
    std::map<std::string, std::shared_ptr<djv::UI::FloatEdit> > _floatEdits;
    std::map<std::string, std::shared_ptr<djv::UI::FormLayout> > _layouts;
    std::map<std::string, std::shared_ptr<djv::UI::GroupBox> > _groupBoxes;
    std::function<void(const djv::Scene::PolarCameraData&)> _cameraDataCallback;
};
