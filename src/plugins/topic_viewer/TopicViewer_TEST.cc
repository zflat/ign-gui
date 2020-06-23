#include <gtest/gtest.h>

#include <ignition/common/Console.hh>

#include "test_config.h"  // NOLINT(build/include)
#include "ignition/gui/Application.hh"
#include "ignition/gui/Plugin.hh"
#include "ignition/gui/MainWindow.hh"
#include <QTreeView>
#include <QTreeWidget>
#include "TopicViewer.hh"

#define NAME_ROLE 51
#define TYPE_ROLE 52
#define TOPIC_ROLE 53
#define PATH_ROLE 54
#define PLOT_ROLE 55


int g_argc = 1;
char **g_argv = new char *[g_argc];

using namespace ignition;
using namespace gui;
using namespace plugins;

/////////////////////////////////////////////////
TEST(TopicViewerTest, Load)
{
    common::Console::SetVerbosity(4);

    Application app(g_argc, g_argv);
    app.AddPluginPath(std::string(PROJECT_BINARY_PATH) + "/lib");

    EXPECT_TRUE(app.LoadPlugin("TopicViewer"));

    // Get main window
    auto win = app.findChild<MainWindow *>();
    ASSERT_NE(nullptr, win);

    // Get plugin
    auto plugins = win->findChildren<Plugin *>();
    EXPECT_EQ(plugins.size(), 1);

    auto plugin = plugins[0];
    EXPECT_EQ(plugin->Title(), "Topic Viewer");

    // Cleanup
    plugins.clear();
}

TEST(TopicViewerTest, Model)
{
    // =========== Publish =================
    transport::Node node;

    // int
    auto pubInt = node.Advertise<msgs::Int32>("/IntTopic");
    msgs::Int32 msgInt;

    // collision
    auto pub = node.Advertise<msgs::Collision> ("/CollisionTopic");
    msgs::Collision msg;

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    pub.Publish(msg);
    pubInt.Publish(msgInt);

    // ========== Load the Plugin ============
    common::Console::SetVerbosity(4);

    Application app(g_argc, g_argv);
    app.AddPluginPath(std::string(PROJECT_BINARY_PATH) + "/lib");

    EXPECT_TRUE(app.LoadPlugin("TopicViewer"));

    // Get main window
    auto win = app.findChild<MainWindow *>();
    EXPECT_NE(nullptr, win);

    // Get plugin
    auto plugins = win->findChildren<plugins::TopicViewer *>();
    ASSERT_EQ(plugins.size(), 1);

    auto plugin = plugins[0];
    ASSERT_NE(nullptr, plugin);

    // ============= Model =====================
    auto model = plugin->Model();
    ASSERT_NE(model, nullptr);

    auto root = model->invisibleRootItem();
    ASSERT_NE(model, nullptr);

    ASSERT_EQ(root->hasChildren(), true);

    bool foundCollision = false;
    bool foundInt = false;

    for (int i = 0; i < root->rowCount(); ++i)
    {
        auto child = root->child(i);

        if (child->data(NAME_ROLE) == "/CollisionTopic")
        {
            foundCollision = true;

            EXPECT_EQ(child->data(TYPE_ROLE), "ignition.msgs.Collision");
            EXPECT_EQ(child->rowCount(), 8);

            auto pose = child->child(5);
            auto position = pose->child(3);
            auto x = position->child(1);

            EXPECT_EQ(x->data(NAME_ROLE), "x");
            EXPECT_EQ(x->data(TYPE_ROLE), "double");
            EXPECT_EQ(x->data(PATH_ROLE), "pose-position-x");
            EXPECT_EQ(x->data(TOPIC_ROLE), "/CollisionTopic");
            EXPECT_EQ(x->data(PLOT_ROLE), true);
        }
        else if (child->data(NAME_ROLE) == "/IntTopic")
        {
            foundInt = true;

            EXPECT_EQ(child->data(TYPE_ROLE), "ignition.msgs.Int32");
            EXPECT_EQ(child->rowCount(), 2);

            auto data = child->child(1);

            EXPECT_EQ(data->data(NAME_ROLE), "data");
            EXPECT_EQ(data->data(TYPE_ROLE), "int32");
            EXPECT_EQ(data->data(PATH_ROLE), "data");
            EXPECT_EQ(data->data(TOPIC_ROLE), "/IntTopic");
            EXPECT_EQ(data->data(PLOT_ROLE), true);
        }
    }

    EXPECT_EQ(foundCollision, true);
    EXPECT_EQ(foundInt, true);

    auto treeWidget = plugin->findChild<QTreeWidget *>("treeView");
    auto treeView = plugin->findChild<QTreeView *>("treeView");
    bool found = treeView || treeWidget;
//    ASSERT_EQ(true, found);
}
