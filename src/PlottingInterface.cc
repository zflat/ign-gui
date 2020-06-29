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

namespace ignition
{
namespace gui
{
class PlottingIfacePrivate
{
    /// \brief responsible for transport messages and topics
    public: Transport* transport;
};

class TransportPrivate
{
  /// \brief Node for Commincation
  public: ignition::transport::Node node;
  /// \brief subscribed topics
  public: std::map<std::string, Topic*> topics;
};

}
}

using namespace ignition;
using namespace gui;

// ==================== Field =======================
Field::Field()
{
  this->value = 0;
}

//////////////////////////////////////////////////////
void Field::SetValue(double _value)
{
  this->value = _value;
}

//////////////////////////////////////////////////////
double Field::Value()
{
  return this->value;
}

//////////////////////////////////////////////////////
void Field::AddChart(int _chart)
{
  this->charts.insert(_chart);
}

//////////////////////////////////////////////////////
void Field::RemoveChart(int _chart)
{
  auto chartIt = this->charts.find(_chart);
  if (chartIt != this->charts.end())
    this->charts.erase(chartIt);
}

//////////////////////////////////////////////////////
int Field::ChartCount()
{
  return this->charts.size();
}

//////////////////////////////////////////////////////
std::set<int>& Field::Charts()
{
  return this->charts;
}

//////////////////////////////////////////////////////
Topic::Topic(std::string _name)
{
  this->name = _name;
}

//////////////////////////////////////////////////////
std::string Topic::Name()
{
  return this->name;
}

//////////////////////////////////////////////////////
void Topic::Register(std::string _fieldPath, int _chart)
{
  // if a new field create a new field and register the chart
  if (this->fields.count(_fieldPath) == 0)
    this->fields[_fieldPath] = new Field();

  this->fields[_fieldPath]->AddChart(_chart);
}

//////////////////////////////////////////////////////
void Topic::UnRegister(std::string _fieldPath, int _chart)
{
  this->fields[_fieldPath]->RemoveChart(_chart);

  // if no one registers to the field, remove it
  if (!this->fields[_fieldPath]->ChartCount())
    this->fields.erase(_fieldPath);
}

//////////////////////////////////////////////////////
int Topic::FieldCount()
{
  return this->fields.size();
}

//////////////////////////////////////////////////////
std::map<std::string, Field*>& Topic::Fields()
{
  return this->fields;
}

//////////////////////////////////////////////////////
void Topic::Callback(const google::protobuf::Message &_msg)
{
  for (auto fieldIt = this->fields.begin(); fieldIt != this->fields.end(); fieldIt++)
  {
      auto msgDescriptor = _msg.GetDescriptor();
      auto ref = _msg.GetReflection();

      google::protobuf::Message *valueMsg = nullptr;

      auto fieldFullPath = ignition::common::Split(fieldIt->first, '-');
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
          valueMsg = ref->MutableMessage
                  (const_cast<google::protobuf::Message *>(&_msg), field);

          ref = valueMsg->GetReflection();
      }

      std::string fieldName = fieldFullPath[pathSize-1];
      double data;

      if (valueMsg)
      {
        auto field = valueMsg->GetDescriptor()->FindFieldByName(fieldName);
        data = this->PlotData(*valueMsg, field);
      }
      else
      {
        auto field = msgDescriptor->FindFieldByName(fieldName);
        data = this->PlotData(_msg, field);
      }

      fieldIt->second->SetValue(data);
  }
}

//////////////////////////////////////////////////////
double Topic::PlotData(const google::protobuf::Message &_msg,
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
Transport :: Transport() : dataPtr(new TransportPrivate)
{
}

////////////////////////////////////////////
Transport :: ~Transport()
{
  // unsubscribe from all topics in the transport
  for (auto topicIt = this->dataPtr->topics.begin();
    topicIt != this->dataPtr->topics.end(); ++topicIt)
    this->dataPtr->node.Unsubscribe(topicIt->first);
}

////////////////////////////////////////////
void Transport :: Unsubscribe(std::string _topic, std::string _fieldPath, int _chart)
{
  if (this->dataPtr->topics.count(_topic))
  {
      this->dataPtr->topics[_topic]->UnRegister(_fieldPath, _chart);

      // if there is no registered fields, unsubscribe from the topic
      if (this->dataPtr->topics[_topic]->FieldCount() == 0)
      {
        this->dataPtr->topics.erase(_fieldPath);
        this->dataPtr->node.Unsubscribe(_topic);
      }
  }
}

////////////////////////////////////////////
void Transport::Subscribe(std::string _topic, std::string _fieldPath, int _chart)
{
  // check if topic found in the transport network
//    if (!this->TopicFound(_topic))
//        return;

  // new topic
  if (this->dataPtr->topics.count(_topic) == 0)
  {
    Topic *topicHandler = new Topic(_topic);
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
    // TODO : recreate the topics model
    return false;
  }
  return true;
}


// ================ Plotting Interface ==================
PlottingInterface :: PlottingInterface() : QObject(),
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
void PlottingInterface::unsubscribe(QString _topic, QString _fieldPath, int _chart)
{
  this->dataPtr->transport->Unsubscribe(_topic.toStdString(),
                                        _fieldPath.toStdString(),
                                        _chart);
}

//////////////////////////////////////////////////////
void PlottingInterface::setSimTime(double _timeout)
{
  this->timer->setInterval(_timeout);
}

//////////////////////////////////////////////////////
void PlottingInterface :: subscribe(QString _topic, QString _fieldPath, int _chart)
{
  this->dataPtr->transport->Subscribe(_topic.toStdString(),
                                      _fieldPath.toStdString(),
                                      _chart);
}

////////////////////////////////////////////
void PlottingInterface::InitTimer()
{
  this->timer = new QTimer();
  this->timer->setInterval(50);
  connect(this->timer,SIGNAL(timeout()),this,SLOT(UpdateGui()));
  this->timer->start();
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
        double x = this->time;
        double y = field.second->Value();

        emit plot(chart, fieldFullPath, x, y);
      }
    }
  }

  this->time++;
}
