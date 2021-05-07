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
#ifndef IGNITION_GUI_IGNRENDERER_HH_
#define IGNITION_GUI_IGNRENDERER_HH_

#include <string>
#include <memory>

#include <ignition/common/MouseEvent.hh>
#include <ignition/math/Vector2.hh>
#include <ignition/math/Vector3.hh>

#include "ignition/gui/Application.hh"
#include "ignition/gui/qt.h"

namespace ignition
{
namespace gui
{
  class IgnRendererPrivate;
/// \brief Ign-rendering renderer.
/// All ign-rendering calls should be performed inside this class as it makes
/// sure that OpenGL calls in the underlying render engine do not interfere
/// with QtQuick's OpenGL render operations. The main Render function will
/// render to an offscreen texture and notify via signal and slots when it's
/// ready to be displayed.
class IgnRenderer : public QObject
{
  Q_OBJECT
  ///  \brief Constructor
  public: IgnRenderer();

  ///  \brief Destructor
  public: ~IgnRenderer();

  ///  \brief Main render function
  public: void Render();

  /// \brief Initialize the render engine
  public: void Initialize();

  /// \brief Destroy camera associated with this renderer
  public: void Destroy();

  /// \brief New mouse event triggered
  /// \param[in] _e New mouse event
  /// \param[in] _drag Mouse move distance
  public: void NewMouseEvent(const common::MouseEvent &_e,
      const math::Vector2d &_drag = math::Vector2d::Zero);

  /// \brief Handle mouse event for view control
  private: void HandleMouseEvent();

  private: void BroadcastHoverPos();

  /// \brief Handle mouse event for view control
  private: void HandleMouseViewControl();

  /// \brief Retrieve the first point on a surface in the 3D scene hit by a
  /// ray cast from the given 2D screen coordinates.
  /// \param[in] _screenPos 2D coordinates on the screen, in pixels.
  /// \return 3D coordinates of a point in the 3D scene.
  private: math::Vector3d ScreenToScene(const math::Vector2i &_screenPos)
      const;

  /// \brief Render texture id
  public: GLuint textureId = 0u;

  /// \brief Render engine to use
  public: std::string engineName = "ogre";

  /// \brief Unique scene name
  public: std::string sceneName = "scene";

  /// \brief Camera visibility mask
  public: uint32_t visibilityMask = 0xFFFFFFFFu;

  /// \brief True if engine has been initialized;
  public: bool initialized = false;

  /// \brief Render texture size
  public: QSize textureSize = QSize(1024, 1024);

  /// \brief Flag to indicate texture size has changed.
  public: bool textureDirty = false;

  /// \internal
  /// \brief Pointer to private data.
  private: std::unique_ptr<IgnRendererPrivate> dataPtr;
};
}
}
#endif
