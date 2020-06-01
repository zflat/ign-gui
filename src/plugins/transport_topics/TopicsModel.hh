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
#include <QStandardItemModel>
#include <QHash>
#include <QByteArray>
#include <QString>
#include <QModelIndex>
#include <string>
#include <vector>

#include <ignition/msgs.hh>

#define NAME_KEY "name"
#define TYPE_KEY "key"

#define NAME_ROLE 51
#define TYPE_ROLE 52

namespace ignition
{
namespace gui
{
  /// \brief Model for the TreeView in the Qml
  class TopicsModel : public QStandardItemModel
  {
    /// \brief Constructor
    public: TopicsModel();
    /// \brief add topic to the model with no children
    public: void AddTopic(std::string _topic, std::string _msg);

    /// \brief add _topic to the model with child _msg
    public: void AddField(QStandardItem* _parentItem,
                          std::string _msgName, std::string _msgType);

    /// \brief factory method for creating an item
    public: QStandardItem *FactoryItem(std::string _item, std::string _type);

    /// \brief get the topic name from its _index in the Model
    public: QString MsgName(QModelIndex _index);

    /// \brief roles and names of the model
    public: QHash<int, QByteArray> roleNames() const override;

    /// \brief get the topic name of selected msg
    public: std::string GetParentTopicName(QModelIndex _index);

    /// \brief full path starting from topic name till the msg name
    /// \param[in] _index index of the QStanadardItem
    /// \return string with all elements separated by '/'
    public: std::vector<std::string> GetFullPathItemName(QModelIndex _index);

    /// \brief check if the type is supported in the plotting types
    /// \param _type the msg type to check if it is supported
    public: bool IsPlotable(
              const google::protobuf::FieldDescriptor::Type &_type);

    /// \brief check if the index is an absolute child
    public: bool IsPlotable(QModelIndex _index);

    /// \brief supported types for plotting
    private: std::vector<google::protobuf::FieldDescriptor::Type> plotableTypes;

    /// \brief mode to show or hide non plotting types
    private: bool plottingMode;

    public: void SetPlottingMode(bool _mode);

    public: bool GetPlottingMode();
  };

}
}

