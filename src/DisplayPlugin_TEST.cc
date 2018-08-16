/*
 * Copyright (C) 2018 Open Source Robotics Foundation
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

#include <gtest/gtest.h>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/gui/DisplayPlugin.hh"
#include "ignition/gui/Iface.hh"
#include "ignition/gui/MainWindow.hh"

using namespace ignition;
using namespace gui;

/////////////////////////////////////////////////
TEST(DisplayPluginTest, LoadingSavingDisplayPlugin)
{
  setenv("IGN_GUI_DISPLAY_PLUGIN_PATH",
    (std::string(PROJECT_BINARY_PATH) + "/lib").c_str(), 1);

  setVerbosity(4);
  EXPECT_TRUE(initApp());

  // Load empty Displays plugin
  EXPECT_TRUE(loadPlugin("Displays"));

  // Load Scene3D before DisplayPlugins
  EXPECT_TRUE(loadPlugin("Scene3D"));

  // Load Displays plugin with a TestDisplayPlugin
  const char *pluginStr =
    "<plugin filename=\"Displays\">"
      "<displays>"
        "<display type=\"TestDisplayPlugin\" />"
      "</displays>"
    "</plugin>";

  tinyxml2::XMLDocument pluginDoc;
  pluginDoc.Parse(pluginStr);
  EXPECT_TRUE(ignition::gui::loadPlugin("Displays",
      pluginDoc.FirstChildElement("plugin")));

  // Create main window
  EXPECT_TRUE(createMainWindow());
  auto win = mainWindow();
  ASSERT_NE(nullptr, win);

  // Check plugin count
  EXPECT_EQ(3, win->findChildren<Plugin *>().size());

  // Check scene
  auto engine = rendering::engine("ogre");
  ASSERT_NE(nullptr, engine);

  auto scene = engine->SceneByName("scene");
  ASSERT_NE(nullptr, scene);

  // Visual created by the base DisplayPlugin class' constructor.
  ASSERT_EQ(1u, scene->VisualCount());

  // Geometry added to the visual in the TestDisplayPlugin's initialization.
  ASSERT_EQ(1u, scene->VisualByIndex(0)->GeometryCount());

  // Get the config of the DisplayPlugin in its default state.
  auto currentConfigStr = win->CurrentWindowConfig().XMLString();
  EXPECT_FALSE(currentConfigStr.empty());

  // Scene should be written.
  // TODO(dhood): narrow the scope of this check so can't give false positives.
  EXPECT_NE(currentConfigStr.find("<scene>scene</scene>"),
    std::string::npos) << currentConfigStr;

  // Title should be written.
  EXPECT_NE(currentConfigStr.find("<title>Test display plugin</title>"),
    std::string::npos) << currentConfigStr;

  // By default the main visual is visible.
  // This can't be checked directly (Ogre Nodes support setting visibility, but
  // not retrieving it), but we can still check the property and saved config.
  auto visibleCheck = win->findChild<QCheckBox *>("displayPluginVisibleCheck");
  EXPECT_NE(nullptr, visibleCheck);
  EXPECT_TRUE(visibleCheck->isChecked());
  EXPECT_NE(currentConfigStr.find("<visible>true</visible>"),
    std::string::npos) << currentConfigStr;

  // Disable the main visual.
  visibleCheck->setChecked(false);
  EXPECT_FALSE(visibleCheck->isChecked());
  currentConfigStr = win->CurrentWindowConfig().XMLString();
  EXPECT_FALSE(currentConfigStr.empty());
  EXPECT_NE(currentConfigStr.find("<visible>false</visible>"),
    std::string::npos) << currentConfigStr;

  EXPECT_TRUE(stop());
}

/////////////////////////////////////////////////
TEST(DisplayPluginTest, LoadingCustomizedTestDisplayPlugin)
{
  setenv("IGN_GUI_DISPLAY_PLUGIN_PATH",
    (std::string(PROJECT_BINARY_PATH) + "/lib").c_str(), 1);

  setVerbosity(4);
  EXPECT_TRUE(initApp());

  // Load Scene3D with a custom name
  const char *pluginStr =
    "<plugin filename=\"Scene3D\">"
      "<scene>my_scene</scene>"
    "</plugin>";

  tinyxml2::XMLDocument pluginDoc;
  pluginDoc.Parse(pluginStr);
  EXPECT_TRUE(ignition::gui::loadPlugin("Scene3D",
      pluginDoc.FirstChildElement("plugin")));

  // Load Displays plugin with a customized TestDisplayPlugin
  pluginStr =
    "<plugin filename=\"Displays\">"
      "<displays>"
        "<display type=\"TestDisplayPlugin\">"
          "<title>My display plugin title</title>"
          "<scene>my_scene</scene>"
          "<visible>false</visible>"
        "</display>"
      "</displays>"
    "</plugin>";

  pluginDoc.Parse(pluginStr);
  EXPECT_TRUE(ignition::gui::loadPlugin("Displays",
      pluginDoc.FirstChildElement("plugin")));

  // Create main window
  EXPECT_TRUE(createMainWindow());
  auto win = mainWindow();
  ASSERT_NE(nullptr, win);

  // Check scene
  auto engine = rendering::engine("ogre");
  ASSERT_NE(nullptr, engine);

  auto scene = engine->SceneByName("my_scene");
  ASSERT_NE(nullptr, scene);

  // Get the config of the DisplayPlugin in its default state.
  auto currentConfigStr = win->CurrentWindowConfig().XMLString();
  EXPECT_FALSE(currentConfigStr.empty());

  // Scene should be written with the custom name.
  // TODO(dhood): narrow the scope of this check so can't give false positives.
  EXPECT_NE(currentConfigStr.find("<scene>my_scene</scene>"),
    std::string::npos) << currentConfigStr;

  // Title should be written with the custom value.
  EXPECT_NE(currentConfigStr.find("<title>My display plugin title</title>"),
    std::string::npos) << currentConfigStr;

  // Visible should be written as false.
  auto visibleCheck = win->findChild<QCheckBox *>("displayPluginVisibleCheck");
  EXPECT_NE(nullptr, visibleCheck);
  EXPECT_FALSE(visibleCheck->isChecked());
  EXPECT_NE(currentConfigStr.find("<visible>false</visible>"),
    std::string::npos) << currentConfigStr;

  EXPECT_TRUE(stop());
}
