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

#include <ignition/gui/PlottingInterface.hh>
#include <ignition/gui/Application.hh>
#include <ignition/common.hh>
#include <ignition/gui.hh>

namespace ignition
{
namespace gui
{
class PlotDataPrivate
{
  /// \brief value of that field
  public: double value;

  /// \brief Registered Charts to that field
  public: std::set<int> charts;
};


class TopicPrivate
{
  /// \brief topic name
  public: std::string name;

  /// \brief Plotting fields to update its values
  public: std::map<std::string, ignition::gui::PlotData*> fields;
};

class TransportPrivate
{
  /// \brief Node for Commincation
  public: ignition::transport::Node node;
  /// \brief subscribed topics
  public: std::map<std::string, ignition::gui::Topic*> topics;
};

class PlottingIfacePrivate
{
  /// \brief responsible for transport messages and topics
  public: Transport* transport;

  /// \brief current plotting time
  public: float time;

  /// \brief timer to update the plotting each time step
  public: QTimer* timer;
};

}
}

using namespace ignition;
using namespace gui;

// ==================== Field =======================
PlotData::PlotData()
{
    this->dataPtr->value = 0;
}

PlotData::~PlotData()
{

}

//////////////////////////////////////////////////////
void PlotData::SetValue(const double _value)
{
  this->dataPtr->value = _value;
}

//////////////////////////////////////////////////////
double PlotData::Value() const
{
  return this->dataPtr->value;
}

//////////////////////////////////////////////////////
void PlotData::AddChart(int _chart)
{
  this->dataPtr->charts.insert(_chart);
}

//////////////////////////////////////////////////////
void PlotData::RemoveChart(int _chart)
{
  auto chartIt = this->dataPtr->charts.find(_chart);
  if (chartIt != this->dataPtr->charts.end())
    this->dataPtr->charts.erase(chartIt);
}

//////////////////////////////////////////////////////
int PlotData::ChartCount()
{
  return this->dataPtr->charts.size();
}

//////////////////////////////////////////////////////
std::set<int>& PlotData::Charts()
{
  return this->dataPtr->charts;
}

//////////////////////////////////////////////////////
Topic::Topic(std::string _name)
{
    this->dataPtr->name = _name;
}

Topic::~Topic()
{

}

//////////////////////////////////////////////////////
std::string Topic::Name()
{
  return this->dataPtr->name;
}

//////////////////////////////////////////////////////
void Topic::Register(std::string _fieldPath, int _chart)
{
  // if a new field create a new field and register the chart
  if (this->dataPtr->fields.count(_fieldPath) == 0)
    this->dataPtr->fields[_fieldPath] = new PlotData();

  this->dataPtr->fields[_fieldPath]->AddChart(_chart);
}

//////////////////////////////////////////////////////
void Topic::UnRegister(std::string _fieldPath, int _chart)
{
  this->dataPtr->fields[_fieldPath]->RemoveChart(_chart);

  // if no one registers to the field, remove it
  if (!this->dataPtr->fields[_fieldPath]->ChartCount())
    this->dataPtr->fields.erase(_fieldPath);
}

//////////////////////////////////////////////////////
int Topic::FieldCount()
{
  return this->dataPtr->fields.size();
}

//////////////////////////////////////////////////////
std::map<std::string, PlotData*>& Topic::Fields()
{
  return this->dataPtr->fields;
}

//////////////////////////////////////////////////////
void Topic::Callback(const google::protobuf::Message &_msg)
{
  for (auto fieldIt : this->dataPtr->fields)
  {
      auto msgDescriptor = _msg.GetDescriptor();
      auto ref = _msg.GetReflection();

      google::protobuf::Message *valueMsg = nullptr;

      auto fieldFullPath = ignition::common::Split(fieldIt.first, '-');
      int pathSize = fieldFullPath.size();

      // loop until you reach the last field in the path
      for (int i = 0; i < pathSize-1 ; i++)
      {
        std::string fieldName = fieldFullPath[i];

        auto field = msgDescriptor->FindFieldByName(fieldName);

        msgDescriptor = field->message_type();

        if (valueMsg)
          valueMsg = ref->MutableMessage
                  (const_cast<google::protobuf::Message *>(valueMsg), field);
        else
        {
            valueMsg = ref->MutableMessage
                    (const_cast<google::protobuf::Message *>(&_msg), field);
        }

          ref = valueMsg->GetReflection();
      }

      std::string fieldName = fieldFullPath[pathSize-1];
      double data;

      if (valueMsg)
      {
        auto field = valueMsg->GetDescriptor()->FindFieldByName(fieldName);
        data = this->FieldData(*valueMsg, field);
      }
      else
      {
        auto field = msgDescriptor->FindFieldByName(fieldName);
        data = this->FieldData(_msg, field);
      }

      fieldIt.second->SetValue(data);
  }
}

//////////////////////////////////////////////////////
double Topic::FieldData(const google::protobuf::Message &_msg,
                              const google::protobuf::FieldDescriptor *_field)
{
  using namespace google::protobuf;
  auto ref = _msg.GetReflection();
  auto type = _field->type();

  if (type == FieldDescriptor::Type::TYPE_DOUBLE)
    return ref->GetDouble(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_FLOAT)
    return ref->GetFloat(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_INT32)
    return ref->GetInt32(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_INT64)
    return ref->GetInt64(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_BOOL)
    return ref->GetBool(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_UINT32)
    return ref->GetUInt32(_msg, _field);
  else if (type == FieldDescriptor::Type::TYPE_UINT64)
    return ref->GetUInt64(_msg, _field);
  else
  {
    ignwarn << "Non Plotting Type" << std::endl;
    return 0;
  }
}

//  ================= Transport ==================
Transport::Transport() : dataPtr(new TransportPrivate)
{
}

////////////////////////////////////////////
Transport::~Transport()
{
  // unsubscribe from all topics in the transport
  for (auto topicIt = this->dataPtr->topics.begin();
    topicIt != this->dataPtr->topics.end(); ++topicIt)
    this->dataPtr->node.Unsubscribe(topicIt->first);
}

////////////////////////////////////////////
void Transport::Unsubscribe(std::string _topic,
                              std::string _fieldPath,
                              int _chart)
{
  if (this->dataPtr->topics.count(_topic))
  {
      this->dataPtr->topics[_topic]->UnRegister(_fieldPath, _chart);

      // if there is no registered fields, unsubscribe from the topic
      if (this->dataPtr->topics[_topic]->FieldCount() == 0)
      {
        this->dataPtr->node.Unsubscribe(_topic);
        this->dataPtr->topics.erase(_topic);
      }
  }
}

////////////////////////////////////////////
void Transport::Subscribe(std::string _topic,
                          std::string _fieldPath,
                          int _chart)
{
  // new topic
  if (this->dataPtr->topics.count(_topic) == 0)
  {
    auto topicHandler = new Topic(_topic);
    this->dataPtr->topics[_topic] = topicHandler;

    topicHandler->Register(_fieldPath, _chart);
    this->dataPtr->node.Subscribe(_topic, &Topic::Callback, topicHandler);
  }

  // already exist topic
  else
  {
    this->dataPtr->topics[_topic]->Register(_fieldPath, _chart);
    this->dataPtr->node.Subscribe(_topic, &Topic::Callback,
                                  this->dataPtr->topics[_topic]);
  }
}

//////////////////////////////////////////////////////
const std::map<std::string, Topic*>& Transport::Topics()
{
  return this->dataPtr->topics;
}

//////////////////////////////////////////////////////
bool Transport::TopicFound(const std::string &_topic)
{
  // check if the topic exist
  std::vector<std::string> topics;
  this->dataPtr->node.TopicList(topics);
  auto foundTopic = std::find(topics.begin(), topics.end(), _topic);
  // topic is unsubscribed ... update the model
  if (foundTopic == topics.end())
  {
    // TODO(Amr) : unsubscribe from the topic and remove its UI fields
    return false;
  }
  return true;
}


// ================ Plotting Interface ==================
PlottingInterface::PlottingInterface() : QObject(),
    dataPtr(std::make_unique<PlottingIfacePrivate>())
{
  this->dataPtr->transport = new Transport();
  this->InitTimer();

  App()->Engine()->rootContext()->setContextProperty("PlottingIface", this);
}

//////////////////////////////////////////////////////
PlottingInterface::~PlottingInterface()
{
}

//////////////////////////////////////////////////////
void PlottingInterface::unsubscribe(QString _topic,
                                    QString _fieldPath,
                                    int _chart)
{
  this->dataPtr->transport->Unsubscribe(_topic.toStdString(),
                                        _fieldPath.toStdString(),
                                        _chart);
}

//////////////////////////////////////////////////////
int PlottingInterface::Timeout()
{
  return this->dataPtr->timer->interval();
}

//////////////////////////////////////////////////////
float PlottingInterface::Time()
{
  return this->dataPtr->time;
}

//////////////////////////////////////////////////////
void PlottingInterface::onComponentSubscribe(QString _entity, QString _typeId,
                                             QString _type, QString _attribute,
                                             int _chart)
{
  // convert the strings into
  uint64_t entity, typeId;
  std::istringstream issEntity(_entity.toStdString());
  issEntity >> entity;
  std::istringstream issTypeId(_typeId.toStdString());
  issTypeId >> typeId;

  emit this->ComponentSubscribe(entity, typeId, _type.toStdString(),
                                _attribute.toStdString(), _chart);
}

//////////////////////////////////////////////////////
void PlottingInterface::onComponentUnSubscribe(QString _entity, QString _typeId,
                                               QString _attribute, int _chart)
{
  // convert the strings into
  uint64_t entity, typeId;
  std::istringstream issEntity(_entity.toStdString());
  issEntity >> entity;
  std::istringstream issTypeId(_typeId.toStdString());
  issTypeId >> typeId;

  emit this->ComponentUnSubscribe(entity, typeId,
                                  _attribute.toStdString(), _chart);
}

//////////////////////////////////////////////////////
void PlottingInterface::subscribe(QString _topic,
                                  QString _fieldPath,
                                  int _chart)
{
  this->dataPtr->transport->Subscribe(_topic.toStdString(),
                                      _fieldPath.toStdString(),
                                      _chart);
}

////////////////////////////////////////////
void PlottingInterface::InitTimer()
{
  this->dataPtr->timer = new QTimer();
  this->dataPtr->timer->setInterval(350);
  connect(this->dataPtr->timer, SIGNAL(timeout()), this, SLOT(UpdateGui()));
  this->dataPtr->timer->start();

  auto moveTimer = new QTimer();
  moveTimer->setInterval(1000);
  connect(moveTimer, SIGNAL(timeout()), this, SLOT(moveCharts()));
}

////////////////////////////////////////////
void PlottingInterface::UpdateGui()
{
  auto topics = this->dataPtr->transport->Topics();

  // Complexity O(Num of Dragged Items) or O(Num of Chart Value Axes)
  for (auto topic : topics)
  {
    auto fields = topic.second->Fields();

    for (auto field : fields)
    {
      auto charts = field.second->Charts();

      for (auto chart : charts)
      {
        QString fieldFullPath = QString::fromStdString(
        topic.first + "-" + field.first);
        double x = this->dataPtr->time;
        double y = field.second->Value();

        emit plot(chart, fieldFullPath, x, y);
      }
    }
  }
  this->dataPtr->time++;
}

//////////////////////////////////////////////////////
void PlottingInterface::moveCharts()
{
  emit moveChart();
}
