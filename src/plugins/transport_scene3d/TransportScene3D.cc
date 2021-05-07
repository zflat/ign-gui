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

#include <iostream>

#include <ignition/common/Console.hh>
#include <ignition/gui/Scene3DInterface.hh>
#include <ignition/plugin/Register.hh>
#include <ignition/transport/Node.hh>

#include "TransportScene3D.hh"

class ignition::gui::plugins::TransportScene3DPrivate
{
  /// \brief Interface to communicate with Qml
  public: std::unique_ptr<Scene3DInterface> iface{nullptr};

  /// \brief Transport node
  public: transport::Node node;
};

using namespace ignition;
using namespace gui;
using namespace plugins;

//////////////////////////////////////////
TransportScene3D::TransportScene3D() : Plugin(),
    dataPtr(new TransportScene3DPrivate())
{
  this->dataPtr->iface = std::make_unique<Scene3DInterface>();
}

//////////////////////////////////////////
TransportScene3D::~TransportScene3D()
{
}

//////////////////////////////////////////
void TransportScene3D::LoadConfig(const tinyxml2::XMLElement * _pluginElem)
{
  ignerr << "TransportScene3D LoadConfig" << std::endl;

  if (this->title.empty())
    this->title = "Scene3D new!";

  this->dataPtr->iface->SetPluginItem(this->PluginItem());

  if (_pluginElem)
  {
    auto elem = _pluginElem->FirstChildElement("engine");
    if (nullptr != elem && nullptr != elem->GetText())
    {
      this->dataPtr->iface->SetEngineName(elem->GetText());
      // renderWindow->SetEngineName();
      // there is a problem with displaying ogre2 render textures that are in
      // sRGB format. Workaround for now is to apply gamma correction manually.
      // There maybe a better way to solve the problem by making OpenGL calls..
      if (elem->GetText() == std::string("ogre2"))
        this->PluginItem()->setProperty("gammaCorrect", true);

      elem = _pluginElem->FirstChildElement("scene");
      if (nullptr != elem && nullptr != elem->GetText())
        this->dataPtr->iface->SetSceneName(elem->GetText());

    //  elem = _pluginElem->FirstChildElement("ambient_light");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    math::Color ambient;
    //    std::stringstream colorStr;
    //    colorStr << std::string(elem->GetText());
    //    colorStr >> ambient;
    //    this->dataPtr->iface->SetAmbientLight(ambient);
    //  }

    //  elem = _pluginElem->FirstChildElement("background_color");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    math::Color bgColor;
    //    std::stringstream colorStr;
    //    colorStr << std::string(elem->GetText());
    //    colorStr >> bgColor;
    //    this->dataPtr->iface->SetBackgroundColor(bgColor);
    //  }

    //  elem = _pluginElem->FirstChildElement("camera_pose");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    math::Pose3d pose;
    //    std::stringstream poseStr;
    //    poseStr << std::string(elem->GetText());
    //    poseStr >> pose;
    //    this->dataPtr->iface->SetCameraPose(pose);
    //  }

    //  elem = _pluginElem->FirstChildElement("service");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    std::string service = elem->GetText();
    //    this->dataPtr->iface->SetSceneService(service);
    //  }

    //  elem = _pluginElem->FirstChildElement("pose_topic");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    std::string topic = elem->GetText();
    //    this->dataPtr->iface->SetPoseTopic(topic);
    //  }

    //  elem = _pluginElem->FirstChildElement("deletion_topic");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    std::string topic = elem->GetText();
    //    this->dataPtr->iface->SetDeletionTopic(topic);
    //  }

    //  elem = _pluginElem->FirstChildElement("scene_topic");
    //  if (nullptr != elem && nullptr != elem->GetText())
    //  {
    //    std::string topic = elem->GetText();
    //    this->dataPtr->iface->SetSceneTopic(topic);
    //  }
    }
    //this->dataPtr->iface->SetSceneService("/world/shapes/scene/info");
    //this->dataPtr->iface->SetPoseTopic("/world/shapes/pose/info");
    //this->dataPtr->iface->SetSceneTopic("/world/shapes/scene/info");
  }
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::TransportScene3D,
                    ignition::gui::Plugin)
