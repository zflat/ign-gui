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
#ifndef IGNITION_GUI_PLUGINS_TRANSPORTSCENE3D_HH_
#define IGNITION_GUI_PLUGINS_TRANSPORTSCENE3D_HH_

#include <memory>

#include <ignition/gui/Plugin.hh>
#include <ignition/utilities/SuppressWarning.hh>

namespace ignition
{
namespace gui
{
namespace plugins
{
class TransportScene3DPrivate;

/// \brief 3D scene that can be updated through a transport interface.
class TransportScene3D : public ignition::gui::Plugin
{
  Q_OBJECT

  /// \brief Constructor
  public: TransportScene3D();

  /// \brief Destructor
  public: ~TransportScene3D();

  // Documentation inherited
  public: void LoadConfig(const tinyxml2::XMLElement *) override;

  IGN_UTILS_WARN_IGNORE__DLL_INTERFACE_MISSING
  private: std::unique_ptr<TransportScene3DPrivate> dataPtr;
  IGN_UTILS_WARN_RESUME__DLL_INTERFACE_MISSING
};

}
}
}
#endif
