/*
 * Copyright (C) 2017 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#include <ignition/common/Console.hh>

#include "ignition/gui/MainWindow.hh"
#include "ignition/gui/Scene3DInterface.hh"
#include "RenderWindow.hh"

namespace ignition
{
namespace gui
{
  /// \brief Private data class for Scene3DInterface
  class Scene3DInterfacePrivate
  {
    /// \brief Pointer to the render window QML item.
    public: RenderWindowItem *renderWindow{nullptr};
  };
}
}

using namespace ignition;
using namespace gui;

/////////////////////////////////////////////////
Scene3DInterface::Scene3DInterface()
  : dataPtr(new Scene3DInterfacePrivate)
{
  qmlRegisterType<RenderWindowItem>("RenderWindow", 1, 0, "RenderWindow");
}

/////////////////////////////////////////////////
Scene3DInterface::~Scene3DInterface()
{
}

/////////////////////////////////////////////////
bool Scene3DInterface::IsSceneAvailable()
{
  return this->dataPtr->renderWindow->IsSceneAvailable();
}

/////////////////////////////////////////////////
void Scene3DInterface::SetPluginItem(QQuickItem *_pluginItem)
{
  this->dataPtr->renderWindow = _pluginItem->findChild<RenderWindowItem *>();
  if (!this->dataPtr->renderWindow)
  {
    ignerr << "Unable to find Render Window item. "
           << "Render window will not be created" << std::endl;
    return;
  }
  this->dataPtr->renderWindow->forceActiveFocus();
}

/////////////////////////////////////////////////
void Scene3DInterface::SetFullScreen(bool _fullscreen)
{
  if(_fullscreen)
  {
    ignition::gui::App()->findChild
      <ignition::gui::MainWindow *>()->QuickWindow()->showFullScreen();
  }
}

/////////////////////////////////////////////////
void Scene3DInterface::SetVisibilityMask(uint32_t _mask)
{
  this->dataPtr->renderWindow->SetVisibilityMask(_mask);
}

/////////////////////////////////////////////////
void Scene3DInterface::SetEngineName(const std::string &_name)
{
  this->dataPtr->renderWindow->SetEngineName(_name);
}

/////////////////////////////////////////////////
void Scene3DInterface::SetSceneName(const std::string &_name)
{
  this->dataPtr->renderWindow->SetSceneName(_name);
}
