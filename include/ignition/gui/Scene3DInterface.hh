/*
 * Copyright (C) 2021 Open Source Robotics Foundation
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
#ifndef IGNITION_GUI_SCENE3DINTERFACE_HH_
#define IGNITION_GUI_SCENE3DINTERFACE_HH_

#include "ignition/gui/Export.hh"

#include <memory>
#include <string>

#include <ignition/math/Pose3.hh>
#include <ignition/math/Color.hh>

#include <QQuickItem>

namespace ignition
{
namespace gui
{
  class Scene3DInterfacePrivate;

  /// \brief Creates a 3D rendering scene and adds a camera to it. Provides
  /// mouse orbit controls and emits events so that plugins can react to
  /// events like render (on the rendering thread) and various mouse events.
  ///
  /// The interface is not meant to be standalone, so it doesn't provide any
  /// methods of updating the scene, such as adding, removing or updating
  /// visuals. Instead, plugins should instantiate this interface and add
  /// update mechanisms on top of it.
  ///
  /// For example, the `TransportScene3D` plugin provides an Ignition Transport
  /// interface to updating the scene.
  class IGNITION_GUI_VISIBLE Scene3DInterface
  {
    /// \brief Constructor
    public: explicit Scene3DInterface();

    /// \brief Destructor
    public: ~Scene3DInterface();

    public: void SetPluginItem(QQuickItem *_pluginItem);
    public: void SetFullScreen(bool _fullscreen);
    public: void SetEngineName(const std::string &_name);
    public: void SetSceneName(const std::string &_name);

    public: bool IsSceneAvailable();

    /// \brief Set the user camera visibility mask
    /// \param[in] _mask Visibility mask to set to
    public: void SetVisibilityMask(uint32_t _mask);

    /// \internal
    /// \brief Pointer to private data.
    private: std::unique_ptr<Scene3DInterfacePrivate> dataPtr;
  };
}
}

#endif
