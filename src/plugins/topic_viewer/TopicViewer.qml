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
import QtQml.Models 2.11
import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3
import QtLocation 5.12

TreeView {
    id:tree
    model: TopicsModel

    Layout.minimumHeight: 400
    Layout.minimumWidth: 300
    anchors.fill: parent

    property int itemHeight: 30;

    // =========== Colors ===========
    property color oddColor: (Material.theme == Material.Light) ?
                                 Material.color(Material.Grey, Material.Shade100):
                                 Material.color(Material.Grey, Material.Shade800);

    property color evenColor: (Material.theme == Material.Light) ?
                                  Material.color(Material.Grey, Material.Shade200):
                                  Material.color(Material.Grey, Material.Shade900);

    property color highlightColor: Material.accentColor;

    // ===============================
    verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    headerVisible: false
    headerDelegate: Rectangle {
        visible: false
    }
    backgroundVisible: false;

    TableViewColumn
    {
        role: "name";
        width: parent.width;
    }

    // =========== Delegates ============
    rowDelegate: Rectangle {
        color: (styleData.selected)? highlightColor : (styleData.row % 2 == 0) ? evenColor : oddColor
        height: itemHeight;
    }

    itemDelegate: Item {
        id :item

        Image {
            id: icon
            source: "plottable_icon.svg"
            height: itemHeight * 0.6
            width: itemHeight * 0.6
            y : itemHeight * 0.2
            visible: model.plottable
        }

        Text {
            id : field
            text: (model === null) ? "" : model.name
            color: (Material.theme == Material.Light) ? Material.color(Material.Grey, Material.Shade800):
                                                        (styleData.selected) ? Material.color(Material.Grey, Material.Shade800):
                                                                               Material.color(Material.Grey, Material.Shade400);
            font.pointSize: 12
            anchors.leftMargin: 5
            anchors.left: icon.right
            y: icon.y

            /*
            MouseArea {
                id : textArea
                anchors.fill: parent
                hoverEnabled: true
                propagateComposedEvents: true

                onClicked: mouse.accepted = false
            }
            */
        }

        ToolTip {
            id: tool_tip
            delay: 1000
            text: (model === null) ? "Type ?" : "Type: " + model.type;
            // visible: textArea.containsMouse
        }
    }

    ///////////////////
    onDoubleClicked:  {
        tree.expandCollapseFolder();
        TopicViewer.print(index);
    }

    signal fieldSelected(int _index);

    function expandCollapseFolder(){
        if (tree.isExpanded(currentIndex))
            tree.collapse(currentIndex)
        else
            tree.expand(tree.currentIndex);
    }


    Component.onCompleted: tree.resizeColumnsToContents();
}
