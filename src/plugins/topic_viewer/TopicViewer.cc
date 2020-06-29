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
#include <ignition/plugin/Register.hh>

#include "TopicViewer.hh"

#define NAME_KEY "name"
#define TYPE_KEY "type"
#define TOPIC_KEY "topic"
#define PATH_KEY "path"
#define PLOT_KEY "plottable"

#define NAME_ROLE 51
#define TYPE_ROLE 52
#define TOPIC_ROLE 53
#define PATH_ROLE 54
#define PLOT_ROLE 55

namespace ignition
{
namespace gui
{
namespace plugins
{
  class TopicsModel : public QStandardItemModel
  {
    /// \brief roles and names of the model
    public: QHash<int, QByteArray> roleNames() const override
    {
      QHash<int, QByteArray> roles;
      roles[NAME_ROLE] = NAME_KEY;
      roles[TYPE_ROLE] = TYPE_KEY;
      roles[TOPIC_ROLE] = TOPIC_KEY;
      roles[PATH_ROLE] = PATH_KEY;
      roles[PLOT_ROLE] = PLOT_KEY;
      return roles;
    }
  };

  class TopicViewerPrivate
  {
    /// \brief Node for Commincation
    public: ignition::transport::Node node;

    /// \brief Model to create it from the available topics and messages
    public: TopicsModel *model;
  };
}
}
}

using namespace ignition;
using namespace gui;
using namespace plugins;

TopicViewer::TopicViewer() : Plugin(), dataPtr(new TopicViewerPrivate)
{
  using namespace google::protobuf;
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_DOUBLE);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_FLOAT);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_INT32);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_INT64);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_UINT32);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_UINT64);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_BOOL);

  this->CreateModel();

  ignition::gui::App()->Engine()->rootContext()->setContextProperty(
                "TopicsModel", this->dataPtr->model);
}

//////////////////////////////////////////////////
TopicViewer::~TopicViewer()
{
}

//////////////////////////////////////////////////
void TopicViewer::LoadConfig(const tinyxml2::XMLElement *)
{
  if (this->title.empty())
    this->title = "Topic Viewer";
}

//////////////////////////////////////////////////
void TopicViewer::CreateModel()
{
  this->dataPtr->model = new TopicsModel();

  std::vector<std::string> topics;
  this->dataPtr->node.TopicList(topics);

  for (unsigned int i = 0; i < topics.size(); ++i)
  {
    std::vector<ignition::transport::MessagePublisher> infoMsgs;
    this->dataPtr->node.TopicInfo(topics[i], infoMsgs);
    std::string msgType = infoMsgs[0].MsgTypeName();
    this->AddTopic(topics[i], msgType);
  }
}

//////////////////////////////////////////////////
QStandardItemModel *TopicViewer::Model()
{
  return reinterpret_cast<QStandardItemModel *>(this->dataPtr->model);
}

//////////////////////////////////////////////////
void TopicViewer::AddTopic(const std::string &_topic,
                           const std::string &_msg)
{
  QStandardItem *topicItem = this->FactoryItem(_topic, _msg);
  topicItem->setWhatsThis("Topic");
  QStandardItem *parent = this->dataPtr->model->invisibleRootItem();
  parent->appendRow(topicItem);

  this->AddField(topicItem , _msg, _msg);
}

//////////////////////////////////////////////////
void TopicViewer::AddField(QStandardItem *_parentItem,
                           const std::string &_msgName,
                           const std::string &_msgType)
{
  QStandardItem *msgItem;

  // check if it is a topic, to skip the extra level of the topic Msg
  if (_parentItem->whatsThis() == "Topic")
  {
    msgItem = _parentItem;
    // make it different, so next iteration will make a new msg item
    msgItem->setWhatsThis("Msg");
  }
  else
  {
    msgItem = this->FactoryItem(_msgName, _msgType);
    _parentItem->appendRow(msgItem);
  }

  auto msg = ignition::msgs::Factory::New(_msgType);
  if (!msg)
      return;

  auto msgDescriptor = msg->GetDescriptor();
  if (!msgDescriptor)
  {
    ignwarn << "Null Descriptor of Msg: " << _msgType << std::endl;
    return;
  }

  for (int i = 0 ; i < msgDescriptor->field_count(); ++i)
  {
    auto msgField = msgDescriptor->field(i);

    if (msgField->is_repeated())
      continue;

    auto messageType = msgField->message_type();

    if (messageType)
      AddField(msgItem, msgField->name(), messageType->name());

    else
    {
      auto msgFieldItem = this->FactoryItem(msgField->name(),
                                            msgField->type_name());
      msgItem->appendRow(msgFieldItem);

      this->SetItemPath(msgFieldItem);
      this->SetItemTopic(msgFieldItem);

      // to make the plottable items draggable
      if (this->IsPlotable(msgField->type()))
        msgFieldItem->setData(QVariant(true), PLOT_ROLE);
    }
  }
}

//////////////////////////////////////////////////
QStandardItem *TopicViewer::FactoryItem(const std::string &_name,
                                        const std::string &_type,
                                        const std::string &_path,
                                        const std::string &_topic)
{
  QString name = QString::fromStdString(_name);
  QString type = QString::fromStdString(_type);
  QString path = QString::fromStdString(_path);
  QString topic = QString::fromStdString(_topic);

  QStandardItem *item = new QStandardItem(name);

  item->setData(QVariant(name), NAME_ROLE);
  item->setData(QVariant(type), TYPE_ROLE);
  item->setData(QVariant(path), PATH_ROLE);
  item->setData(QVariant(topic), TOPIC_ROLE);
  item->setData(QVariant(false), PLOT_ROLE);

  return item;
}

//////////////////////////////////////////////////
void TopicViewer::SetItemTopic(QStandardItem *_item)
{
  std::string topic = this->TopicName(_item);
  QVariant Topic(QString::fromStdString(topic));
  _item->setData(Topic, TOPIC_ROLE);
}

//////////////////////////////////////////////////
void TopicViewer::SetItemPath(QStandardItem *_item)
{
  std::string path = this->ItemPath(_item);
  QVariant Path(QString::fromStdString(path));
  _item->setData(Path, PATH_ROLE);
}

//////////////////////////////////////////////////
std::string TopicViewer::TopicName(const QStandardItem *_item) const
{
  QStandardItem *parent = _item->parent();

  // get the next parent until you reach the first level parent
  while (parent)
  {
    _item = parent;
    parent = parent->parent();
  }

  return _item->data(NAME_ROLE).toString().toStdString();
}

//////////////////////////////////////////////////
std::string TopicViewer::ItemPath(const QStandardItem *_item) const
{
  std::deque<std::string> path;
  while (_item)
  {
    path.push_front(_item->data(NAME_ROLE).toString().toStdString());
    _item = _item->parent();
  }

  if (path.size())
    path.erase(path.begin());

  // convert to string
  std::string pathString;

  for (unsigned int i = 0; i < path.size()-1; ++i)
    pathString += path[i] + "-";

  if (path.size())
    pathString += path[path.size()-1];

  return pathString;
}

/////////////////////////////////////////////////
bool TopicViewer::IsPlotable(
    const google::protobuf::FieldDescriptor::Type &_type)
{
  return std::find(this->plotableTypes.begin(), this->plotableTypes.end(),
                     _type) != this->plotableTypes.end();
}


// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::TopicViewer,
                    ignition::gui::Plugin)
