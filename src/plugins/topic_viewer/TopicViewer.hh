/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
#include <QStandardItem>
#include <QString>
#include <QModelIndex>
#include <string>
#include <vector>
#include <deque>
#include <memory>

#include <ignition/gui/Application.hh>
#include <ignition/gui/Plugin.hh>

#include <ignition/transport/Node.hh>
#include <ignition/transport/MessageInfo.hh>
#include <ignition/transport/Publisher.hh>

#include <ignition/common.hh>
namespace ignition
{
namespace gui
{
namespace plugins
{

  class TopicsModel;
  class TopicViewerPrivate;

  /// \brief Model for the Topics and their Msgs and Fields
  /// a tree model that represents the topics tree with its Msgs
  /// Childeren and each msg node has its own fileds/msgs childeren
  class TopicViewer : public Plugin
  {
    Q_OBJECT

    /// \brief Constructor
    public: TopicViewer();

    /// \brief Destructor
    public: ~TopicViewer();

    /// \brief Documentaation inherited
    public: void LoadConfig(const tinyxml2::XMLElement *) override;

    /// \brief Create the fields model
    private: void CreateModel();

    /// \brief add a topic to the model
    /// \param[in] _topic topic name to be displayed
    /// \param[in] _msg topic's msg type
    private: void AddTopic(const std::string &_topic,
                           const std::string &_msg);

    /// \brief add a field/msg child to that parent item
    /// \param[in] _parentItem a parent for the added field/msg
    /// \param[in] _msgName the displayed name of the field/msg
    /// \param[in] _msgType field/msg type
    private: void AddField(QStandardItem *_parentItem,
                           const std::string &_msgName,
                           const std::string &_msgType);

    /// \brief factory method for creating an item
    /// \param[in] _name the display name
    /// \param[in] _type type of the field of the item
    /// \param[in] _path a set of concatinate strings of parent msgs
    /// names that lead to that field, starting from the most parent
    /// ex : if we have [Collision]msg contains [pose]msg contains [position]
    /// msg contains [x,y,z] fields, so the path of x = "pose-position-x"
    /// \param[in] _topic the name of the most parent item
    /// \return the created Item
    private: QStandardItem *FactoryItem(const std::string &_name,
                                        const std::string &_type,
                                        const std::string &_path = "",
                                        const std::string &_topic = "");

    /// \brief set the topic role name of the item with the most
    /// topic parent of that field item
    /// \param[in] _item item ref to set its topic
    private: void SetItemTopic(QStandardItem *_item);

    /// \brief set the path/ID of the givin item starting from
    /// the most topic parent to the field itself
    /// \param[in] _item item ref to set its path
    private: void SetItemPath(QStandardItem *_item);

    /// \brief get the topic name of selected item
    /// \param[in] _item ref to the item to get its parent topic
    private: std::string TopicName(const QStandardItem *_item) const;

    /// \brief full path starting from topic name till the msg name
    /// \param[in] _index index of the QStanadardItem
    /// \return string with all elements separated by '/'
    private: std::string ItemPath(const QStandardItem *_item) const;

    /// \brief check if the type is supported in the plotting types
    /// \param[in] _type the msg type to check if it is supported
    private: bool IsPlotable(
              const google::protobuf::FieldDescriptor::Type &_type);

    /// \brief supported types for plotting
    private: std::vector<google::protobuf::FieldDescriptor::Type> plotableTypes;

    /// \brief Print all the roles of the item
    /// \param[in] _index index of the item in the model
    public: Q_INVOKABLE void print(const QModelIndex _index);

    /// \brief Pointer to private data.
    private: std:: unique_ptr<TopicViewerPrivate> dataPtr;
  };

}
}
}
