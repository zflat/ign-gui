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
#ifndef IGNITION_GUI_PLOTTINGINTERFACE_HH_
#define IGNITION_GUI_PLOTTINGINTERFACE_HH_

#include <QObject>
#include <QTimer>
#include <QString>
#include <QVariant>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <map>
#include <set>
#include <string>
#include <memory>

#include <ignition/transport/Node.hh>
#include <ignition/transport/MessageInfo.hh>
#include <ignition/transport/Publisher.hh>


namespace ignition
{
namespace gui
{
class PlotDataPrivate;

class PlotData
{
  /// \brief Constructor
  public: PlotData();

  /// \brief Destructor
  public: ~PlotData();

  /// \brief Set the field Value
  /// \param[in] _value the set value
  public: void SetValue(const double _value);

  /// \brief Get the field value
  /// \return value of the field
  public: double Value() const;

  /// \brief Register a chart that plot that field
  /// \param[in] _chart chart ID to be registered
  public: void AddChart(int _chart);

  /// \brief UnRegister a chart from plotting that field
  public: void RemoveChart(int _chart);

  /// \brief size of registered charts
  /// \return charts size
  public: int ChartCount();

  /// \brief Get all registered charts to that field
  /// \return set of registered charts
  public: std::set<int> &Charts();

  /// \brief Private data member.
  private: std::unique_ptr<PlotDataPrivate> dataPtr;
};

class TopicPrivate;

class Topic
{
  /// \brief Constructor
  public: Topic(std::string _name);

  /// \brief Destructor
  public: ~Topic();

  /// \brief get topic name
  public: std::string Name();

  /// \brief Register a chart to a field
  /// \param[in] _fieldPath model path to the field as an ID
  public: void Register(std::string _fieldPath, int _chart);

  /// \brief remove field from the topic
  public: void UnRegister(std::string _fieldPath, int _chart);

  /// \brief size of registered fields
  /// \return fields size
  public: int FieldCount();

  /// \brief get the registered topics
  /// \return topics list
  public: std::map<std::string, PlotData*> &Fields();

  /// \brief callback to subscribe to that topic
  /// \param[in] _msg the published msg from the topic
  public: void Callback(const google::protobuf::Message &_msg);

  /// \brief check the plotable types and get data from reflection
  private: double FieldData(const google::protobuf::Message &_msg,
                           const google::protobuf::FieldDescriptor *field);

  /// \brief Private data member.
  private: std::unique_ptr<TopicPrivate> dataPtr;
};

class TransportPrivate;

/// \brief handle Transport Topics Subscribing for one object(Chart)
class Transport
{
  /// \brief Constructor
  public: Transport();

  /// \brief Destructor
  public: ~Transport();

  /// \brief unsubscribe/deattatch a field from a certain chart
  /// \param[in] _topic topic name
  /// \param[in] _fieldPath field path ID
  /// \param[in] _chart chart ID
  public: void Unsubscribe(std::string _topic,
                           std::string _fieldPath,
                           int _chart);

  /// \brief subscribe/attatch a field from a certain chart
  /// \param[in] _topic topic name
  /// \param[in] _fieldPath field path ID
  /// \param[in] _chart chart ID
  public: void Subscribe(std::string _topic,
                         std::string _fieldPath,
                         int _chart);

  /// \brief is the topic exist in the transport network
  /// \param[in] _topic topic name
  /// \return True if found in the transport, False if not found
  public: bool TopicFound(const std::string &_topic);

  /// \brief get the registered topics
  /// \return topics list
  public: const std::map<std::string, Topic*> &Topics();

  private: std::unique_ptr<TransportPrivate> dataPtr;
};

class PlottingIfacePrivate;

/// \brief Plotting Interface
/// Responsible for plotting transport msgs-fields
/// Used by TransportPlotting Plugin & GazeboPlotting Plugin
/// Accepts dragged items from TopicViewer Plugin & ComponentInspector Plugin
class PlottingInterface : public QObject
{
  Q_OBJECT

  /// \brief update the plotting each timeout of the timer
  public slots: void UpdateGui();

  /// \brief Constructor
  public: explicit PlottingInterface();

  /// \brief Destructor
  public: ~PlottingInterface();

  /// \brief subscribe to a field to plotted on a chart
  /// \brief param[in] _topic the topic that includes that field
  /// \brief param[in] _fieldPath path to the field to reach it from the msg
  /// \brief param[in] _chart chart id to be attached to that field
  public slots: void subscribe(QString _topic,
                               QString _fieldPath,
                               int _chart);

  /// \brief unsubscribe from a field and deattach it from a chart
  /// \brief param[in] _topic the topic that includes that field
  /// \brief param[in] _fieldPath path to the field to reach it from the msg
  /// \brief param[in] _chart chart id to be deattached to that field
  public slots: void unsubscribe(QString _topic,
                                 QString _fieldPath,
                                 int _chart);

  /// \brief Get the timeout of updating the plot
  /// \return updating plot timeout
  public: float Timeout();

  /// \brief Get the Plotting Time
  /// \return Plotting Time
  public: float Time();

  /// \brief plot a point to a chart
  /// \param[in] _chart chart ID
  /// \param[in] _fieldID field path ID
  /// \param[in] _x x coordinates of the plot point
  /// \param[in] _y y coordinates of the plot point
  signals: void plot(int _chart, QString _fieldID, double _x, double _y);

  /// \brief signal to move the chart aka scroll it to the right
  signals: void moveChart();

  /// \brief called by Qml to register a chart to a component attribute
  /// \param[in] _entity entity id which has the component
  /// \param[in] _typeId component type id
  /// \param[in] _type component data type
  /// \param[in] _attribute component specefice attribte
  /// \param[in] _chart chart id
  public slots: void onComponentSubscribe(QString _entity,
                                          QString _typeId,
                                          QString _type,
                                          QString _attribute,
                                          int _chart);

  /// \brief called by Qml to remove a chart from a component attribute
  /// \param[in] _entity entity id which has the component
  /// \param[in] _typeId component type id
  /// \param[in] _type component data type
  /// \param[in] _attribute component specefice attribte
  /// \param[in] _chart chart id
  public slots: void onComponentUnSubscribe(QString _entity,
                                            QString _typeId,
                                            QString _attribute,
                                            int _chart);

  /// \brief Notify the gazebo plugin to subscribe to a component data
  /// \param[in] _entity entity id which has the component
  /// \param[in] _typeId component type id
  /// \param[in] _type component data type
  /// \param[in] _attribute component specefice attribte
  /// \param[in] _chart chart id
  signals: void ComponentSubscribe(uint64_t _entity,
                                   uint64_t _typeId,
                                   std::string _type,
                                   std::string _attribute,
                                   int _chart);

  /// \brief Notify the gazebo plugin to unsubscribe a component data
  /// \param[in] _entity entity id which has the component
  /// \param[in] _typeId component type id
  /// \param[in] _type component data type
  /// \param[in] _attribute component specefice attribte
  /// \param[in] _chart chart id
  signals: void ComponentUnSubscribe(uint64_t _entity,
                                     uint64_t _typeId,
                                     std::string _attribute,
                                     int _chart);

  /// \brief slot to to lestin to a timer to emit moveChart signal
  public slots: void moveCharts();

  /// \brief configration of the timer
  private: void InitTimer();

  /// \brief Private data member.
  private: std::unique_ptr<PlottingIfacePrivate> dataPtr;
};

}
}

#endif
