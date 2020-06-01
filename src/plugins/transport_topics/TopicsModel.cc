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
#include "TopicsModel.hh"

using namespace ignition;
using namespace gui;

TopicsModel :: TopicsModel() :QStandardItemModel()
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
}

//////////////////////////////////////////////////
void TopicsModel :: SetPlottingMode(bool _mode)
{
    this->plottingMode = _mode;
}

bool TopicsModel :: GetPlottingMode()
{
    return this->plottingMode;
}

//////////////////////////////////////////////////
void TopicsModel :: AddTopic(std::string _topic, std::string _msg)
{
    QStandardItem *topicItem = this->FactoryItem(_topic, _topic);
    QStandardItem *parent = this->invisibleRootItem();
    parent->appendRow(topicItem);

    // remove 'ignition.msgs.' from msg name
    try {
        _msg.erase(0, 14);
    } catch (...) {    }

    if (_msg == "Scene")
        return;
    this->AddField(topicItem , _msg, _msg);
}

//////////////////////////////////////////////////
void TopicsModel :: AddField(QStandardItem* _parentItem,
                             std::string _msgName,
                             std::string _msgType)
{
    QStandardItem *msgItem = this->FactoryItem(_msgName, _msgType);
    _parentItem->appendRow(msgItem);

    auto msg = ignition::msgs::Factory::New(_msgType);
    if (!msg)
        return;

    const google::protobuf::Descriptor *msgDescriptor = msg->GetDescriptor();
    if (!msgDescriptor)
        return;


    for (int i =0 ; i < msgDescriptor->field_count(); i++ )
    {
        auto msgField = msgDescriptor->field(i);

        auto messageType = msgField->message_type();
        if (messageType)
        {
            AddField(msgItem, msgField->name(), messageType->name());
//            std::cout << "message_type: " << messageType->name()
//            << " , msgField_name" << msgField->name() << std::endl;
        }
        else
        {
            // skip if it is not pltable type and plottingMode
            if (this->plottingMode && !this->IsPlotable(msgField->type()))
                continue;

            QStandardItem *msgFieldItem = this->FactoryItem
                    (msgField->name(), msgField->type_name());
            msgItem->appendRow(msgFieldItem);
        }
    }
}

//////////////////////////////////////////////////
QStandardItem* TopicsModel :: FactoryItem(std::string _name,
                                          std::string _type)
{
    QString name = QString::fromStdString(_name);
    QString type = QString::fromStdString(_type);

    QStandardItem *item = new QStandardItem(name);

    item->setData(QVariant(name), NAME_ROLE);
    item->setData(QVariant(type), TYPE_ROLE);

    return item;
}

//////////////////////////////////////////////////
QString TopicsModel :: MsgName(QModelIndex _index)
{
    QStandardItem *item = this->itemFromIndex(_index);
    if (!item)
        return "";

    QString topic = item->data(NAME_ROLE).toString();
    return topic;
}

//////////////////////////////////////////////////
QHash<int, QByteArray> TopicsModel :: roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NAME_ROLE] = NAME_KEY;
    roles[TYPE_ROLE] = TYPE_KEY;
    return roles;
}

//////////////////////////////////////////////////
std::string TopicsModel::GetParentTopicName(QModelIndex _index)
{
    QStandardItem *item = this->itemFromIndex(_index);
    QStandardItem *parent = item->parent();

    // get the next parent until you reach the first level parent
    while (parent)
    {
        item = parent;
        parent = parent->parent();
    }

    return item->data(NAME_ROLE).toString().toStdString();
}

std::vector<std::string> TopicsModel::GetFullPathItemName
(QModelIndex _index)
{
    std::vector<std::string> fullPath;
    QStandardItem *item = this->itemFromIndex(_index);


    while (item->parent())
    {
        fullPath.push_back(item->data(NAME_ROLE).toString().toStdString());
        item = item->parent();
    }

    std::reverse(fullPath.begin(), fullPath.end());

    if (fullPath.size() > 0)
        fullPath.erase(fullPath.begin());

    return fullPath;
}

/////////////////////////////////////////////////
bool TopicsModel::IsPlotable(
        const google::protobuf::FieldDescriptor::Type &_type)
{
    for (unsigned int j =0 ; j < this->plotableTypes.size() ; j++)
    {
        if (_type == this->plotableTypes[j])
        {
            return true;
        }
    }
    return false;
}

bool TopicsModel::IsPlotable(QModelIndex _index)
{
    QStandardItem *item = this->itemFromIndex(_index);
    if (item->hasChildren())
        return false;

    std::string msgType = item->data(TYPE_ROLE).toString().toStdString();
    auto msg = ignition::msgs::Factory::New(msgType);
    if (msg)
        return false;

    return true;
}
