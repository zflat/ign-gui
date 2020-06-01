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

#include "TransportTopics.hh"
#include <ignition/common.hh>
#include <ignition/plugin/Register.hh>

namespace ignition
{
namespace gui
{
namespace plugins
{
  class TransportTopicsPrivate
  {
    /// \brief Node for Commincation
    public: ignition::transport::Node node;
    /// \brief topic to subscribe
    public: std::string topic;
    /// \brief Model to create it from the available topics and messages
    public: TopicsModel *model;
    /// \brief msg path to find the plotted msg
    public: std::vector<std::string> fieldFullPath;
    /// \brief y-axis value to plot, double to hold all types
    public: double yData;
  };
}
}
}


using namespace ignition;
using namespace gui;
using namespace plugins;

TransportTopics :: TransportTopics() : data_ptr(new TransportTopicsPrivate)
{
    this->data_ptr->model = new TopicsModel();
    this->data_ptr->model->SetPlottingMode(true);
    this->CreateModel();

    ignition::gui::App()->Engine()->rootContext()->setContextProperty(
        "TopicsModel", this->data_ptr->model);
}

////////////////////////////////////////////
TransportTopics :: ~TransportTopics()
{
    this->Unsubscribe();
}

////////////////////////////////////////////
void TransportTopics :: CreateModel()
{
    std::vector<std::string> allTopics;
    this->data_ptr->node.TopicList(allTopics);

    for (unsigned int i = 0; i < allTopics.size(); i++)
    {
        std::vector<ignition::transport::MessagePublisher> infoMsgs;

        this->data_ptr->node.TopicInfo(allTopics[i], infoMsgs);

        std::string msgType = infoMsgs[0].MsgTypeName();

        this->data_ptr->model->AddTopic(allTopics[i], msgType);
    }
}

////////////////////////////////////////////
void TransportTopics :: subscribe(QModelIndex _index)
{
    // check if it is a plotable field
    if (!this->data_ptr->model->IsPlotable(_index))
        return;

    std::string topic = this->data_ptr->model->GetParentTopicName(_index);
    std::vector<std::string> fullPath =
            this->data_ptr->model->GetFullPathItemName(_index);

    this->data_ptr->fieldFullPath = fullPath;
    this->Subscribe(topic);
}

////////////////////////////////////////////
QStandardItemModel* TransportTopics :: GetModel()
{
    return  this->data_ptr->model;
}

////////////////////////////////////////////
QVariant TransportTopics :: GetValue()
{
    return QVariant(this->data_ptr->yData);
}

////////////////////////////////////////////
void TransportTopics :: SetTopic(const std::string &_topic)
{
    this->data_ptr->topic = _topic;
}

////////////////////////////////////////////
void TransportTopics :: Unsubscribe()
{
    std::vector<std::string> subscribedTopics =
            this->data_ptr->node.SubscribedTopics();
    for (unsigned int i = 0; i < subscribedTopics.size(); i++)
    {
        this->data_ptr->node.Unsubscribe(subscribedTopics[i]);
    }
}

////////////////////////////////////////////
void TransportTopics :: Subscribe(std::string _topic)
{
    this->Unsubscribe();
    this->SetTopic(_topic);
    // get the typeMsg of the topic to know which callback to assign
    std::vector<ignition::transport::MessagePublisher> infoMsgs;
    this->data_ptr->node.TopicInfo(this->data_ptr->topic, infoMsgs);
//    std::string msgType = infoMsgs[0].MsgTypeName();

    this->data_ptr->node.Subscribe(
                this->data_ptr->topic, &TransportTopics::Callback, this);
}

void TransportTopics::Callback(const google::protobuf::Message &_msg)
{
    auto msgDescriptor = _msg.GetDescriptor();
    auto ref = _msg.GetReflection();

    google::protobuf::Message *valueMsg = nullptr;

    int pathSize = this->data_ptr->fieldFullPath.size();

    // loop until you reach the last field in the path
    for (int i = 0; i < pathSize-1 ; i++)
    {
        std::string fieldName = this->data_ptr->fieldFullPath[i];

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


    std::string fieldName = this->data_ptr->fieldFullPath[pathSize-1];
    double data;

    if (valueMsg)
    {
        auto field = valueMsg->GetDescriptor()->FindFieldByName(fieldName);
        data = this->GetPlotData(*valueMsg, field);
    }
    else
    {
        auto field = msgDescriptor->FindFieldByName(fieldName);
        data = this->GetPlotData(_msg, field);
    }

    this->data_ptr->yData = data;
    igndbg << "plotData= " << data << std :: endl;
}

double TransportTopics::GetPlotData(const google::protobuf::Message &_msg,
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
        igndbg << "Non Plotting Type" << std::endl;
        return 0;
    }
}

// Register this plugin
IGNITION_ADD_PLUGIN(ignition::gui::plugins::TransportTopics,
                    ignition::gui::Plugin)

