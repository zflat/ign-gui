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
#include "TopicViewer.hh"
#include <ignition/plugin/Register.hh>

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

TopicViewer::TopicViewer() :Plugin(), dataPtr(new TopicViewerPrivate)
{
  using namespace google::protobuf;
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_DOUBLE);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_FLOAT);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_INT32);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_INT64);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_UINT32);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_UINT64);
  this->plotableTypes.push_back(FieldDescriptor::Type::TYPE_BOOL);

  this->plottingMode = true;

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
void TopicViewer::SetPlottingMode(bool _mode)
{
  this->plottingMode = _mode;
}

//////////////////////////////////////////////////
bool TopicViewer::PlottingMode()
{
  return this->plottingMode;
}

//////////////////////////////////////////////////
void TopicViewer::AddTopic(const std::string &_topic,
                           const std::string &_msg)
{
  // remove 'ignition.msgs.' from msg name
  std::string msg = _msg;
  try
  {
    if (msg.substr(0, 14) == "ignition.msgs.")
      msg.erase(0, 14);
  }
  catch (std::out_of_range &exception)
  {
    igndbg << "faild to parse msg: " << _msg << std::endl;
  }

  QStandardItem *topicItem = this->FactoryItem(_topic, msg);
  topicItem->setWhatsThis("Topic");
  QStandardItem *parent = this->dataPtr->model->invisibleRootItem();
  parent->appendRow(topicItem);

  if (msg == "Scene")
    return;

  this->AddField(topicItem , msg, msg);
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
    return;

  for (int i =0 ; i < msgDescriptor->field_count(); ++i)
  {
    auto msgField = msgDescriptor->field(i);

    auto messageType = msgField->message_type();
    if (messageType)
    {
      AddField(msgItem, msgField->name(), messageType->name());
      igndbg << "name: " << msgField->name() <<
                ", type: " << messageType->name() << std::endl;
    }
    else
    {
      // skip if it is not plottable type and plottingMode
      if (this->plottingMode && !this->IsPlotable(msgField->type()))
        continue;

      auto msgFieldItem = this->FactoryItem(msgField->name(),
                                            msgField->type_name());
      msgItem->appendRow(msgFieldItem);

      this->SetItemPath(msgFieldItem);
      this->SetItemTopic(msgFieldItem);
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
std::string TopicViewer::TopicName(QStandardItem *_item)
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
std::string TopicViewer::ItemPath(QStandardItem *_item)
{
  std::deque<std::string> Path;
  while (_item)
  {
    Path.push_front(_item->data(NAME_ROLE).toString().toStdString());
    _item = _item->parent();
  }

  if (Path.size())
    Path.erase(Path.begin());

  // convert to string
  std::string path;

  for (unsigned int i = 0; i < Path.size()-1; ++i)
    path += Path[i] + "-";

  if (Path.size())
    path += Path[Path.size()-1];

  return path;
}

/////////////////////////////////////////////////
bool TopicViewer::IsPlotable(
        const google::protobuf::FieldDescriptor::Type &_type)
{
  for (unsigned int j =0 ; j < this->plotableTypes.size() ; ++j)
  {
    if (_type == this->plotableTypes[j])
      return true;
  }
  return false;
}

//////////////////////////////////////////////////
bool TopicViewer::IsPlotable(QModelIndex _index)
{
  QStandardItem *item = this->dataPtr->model->itemFromIndex(_index);
  if (item->hasChildren())
    return false;

  std::string msgType = item->data(TYPE_ROLE).toString().toStdString();
  auto msg = ignition::msgs::Factory::New(msgType);
  if (msg)
    return false;

  return true;
}

////////////////////////////////////////////
void TopicViewer::print(QModelIndex _index)
{
  auto item = this->dataPtr->model->itemFromIndex(_index);
  auto name = item->data(NAME_ROLE).toString().toStdString();
  auto type = item->data(TYPE_ROLE).toString().toStdString();
  auto path = item->data(PATH_ROLE).toString().toStdString();
  auto topic = item->data(TOPIC_ROLE).toString().toStdString();

  igndbg << "name: " << name << ", type: " << type <<
               ", path: " << path << ", topic: " << topic << std::endl;
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::TopicViewer,
                    ignition::gui::Plugin)
