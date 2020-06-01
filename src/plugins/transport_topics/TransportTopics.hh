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

#ifndef TRANSPORT_HH
#define TRANSPORT_HH
#include <QObject>
#include <QString>
#include <string>
#include <memory>

#include <ignition/gui/Application.hh>
#include <ignition/gui/Plugin.hh>

#include <ignition/transport.hh>
#include <ignition/transport/Node.hh>
#include <ignition/transport/MessageInfo.hh>
#include <ignition/transport/Publisher.hh>

#include "TopicsModel.hh"

namespace ignition
{
namespace gui
{
namespace plugins
{
  class TransportTopicsPrivate;

  /// \brief handle Transport Topics Subscribing
  class TransportTopics : public ignition::gui::Plugin
  {
    Q_OBJECT
    private: std:: unique_ptr<TransportTopicsPrivate> data_ptr;

    /// \brief Constructor
    public: TransportTopics();

    /// \brief Destructor
    public: ~TransportTopics();

    /// \brief make the model from the available topics and messages
    public: void CreateModel();


    /// \brief get the Model of the topics and msgs
    public: QStandardItemModel *GetModel();

    /// \brief get the published value according to the topic's msg type
    /// wrap it in QVariant to send to javascript
    public: QVariant GetValue();

    /// \brief set the topic to register
    public: void SetTopic(const std::string &_topic);

    /// \brief unsubscribe from the subscribed topics
    public: void Unsubscribe();

    /// \brief subscribe to the topic of the selected index
    public: Q_INVOKABLE void subscribe(QModelIndex _index);

    /// \brief subscribe to the topic in data_ptr->topic
    public: void Subscribe(std::string _topic);

    /// \brief callback for subscribing to a topic
    public: void Callback(const google::protobuf::Message &_msg);

    /// \brief check the plotable types and get data from reflection
    public: double GetPlotData(const google::protobuf::Message &_msg,
                           const google::protobuf::FieldDescriptor *field);
  };

}
}
}


#endif
