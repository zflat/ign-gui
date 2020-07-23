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
import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3

Rectangle {
    id: main
    property int chartID: -1
    property bool multiChartsMode: false
    color: "transparent"
    signal subscribe(real Id, string topic, string path);
    signal unSubscribe(real Id, string topic, string path);
    signal clicked(real Id);

    function appendPoint(_fieldID, _x, _y)
    {
        chart.appendPoint(_fieldID, _x, _y);
    }
    function setChartOpacity(_opacity)
    {
        chart.opacity = _opacity;
    }
    function moveChart()
    {
        chart.scrollRight(chart.width/10);
    }

    // =============== Fields info Rectangle ================
    Rectangle{
        id: infoRect
        width: parent.width
        height: (multiChartsMode) ? 0 : parent.height / 10
        color: (Material.theme == Material.Light) ? Material.color(Material.Grey,Material.Shade200)
                                                  : Material.color(Material.BlueGrey, Material.Shade800)
        // make it scrolable
        ScrollView {
            anchors.fill: parent
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            clip: true
            // Horizontal Layout for the fields
            Row {
                anchors.fill: parent
                id:row
                spacing: 10
            }
        }

        DropArea {
            anchors.fill: parent
            onDropped:
            {
                // topic and path is separated with ','
                if (drop.text.search(",") === -1)
                {
                    console.log("Error Parsing Dragged Item");
                    return;
                }

                var topic_path = drop.text.split(",");
                var topic = topic_path[0];
                var path = topic_path[1];

                // Field Full Path ID
                var ID = topic + "-" + path;

                // if the field is already attached
                if (ID in chart.serieses)
                    return;

                // add axis series to plot the field
                chart.addSeries(ID);

                // add field info component
                infoRect.addField(ID, topic, path);

                // attach the chart to the subscribed field
                subscribe(chartID, topic, path);
            }
        }

        // add field to the fields layout
        function addField(ID, topic, path)
        {
            var field = fieldInfo.createObject(row);
            field.width = 150;
            field.height = Qt.binding( function() {return infoRect.height * 0.8} );
            field.y = Qt.binding( function()
            {
                if (infoRect.height)
                    return (infoRect.height - field.height)/2;
                else
                    return 0;
            }
            );

            // update field data
            field.topic = topic;
            field.path = path;
       }
    }

    // ================ Field Component ====================
    Component {
        id: fieldInfo
        Rectangle {
            id: component
            radius: width/4
            property string topic: ""
            property string path: ""

            Rectangle {
                height: parent.height
                width: parent.width
                radius: width/4
                color: "#72bcd4"
                clip: true

                MouseArea {
                    id : fieldInfoMouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: enterAnimation.start();
                    onExited: exitAnimation.start();
                }

                Text {
                    id: fieldname
//                    anchors.centerIn: parent
                    text: component.topic + "/"+ component.path
                    color: "white"
                    elide: Text.ElideRight
                    width: parent.width * 0.9
                    anchors.verticalCenter: parent.verticalCenter
                }

                ToolTip {
                    id: tool_tip
                    delay: 1000
                    timeout: 2000
                    text: component.topic + "-"+ component.path;
                    visible: fieldInfoMouse.containsMouse
                    y: fieldInfoMouse.mouseY
                    x: fieldInfoMouse.mouseX
                    enter: null
                    exit: null
                }

            }

            function setText(text) {
                fieldname.text = text;
            }
            signal unsubscribe(string topic, string path);

            Rectangle {
                id: exitBtn
                radius: width / 2
                height: parent.height * 0.4;
                width: parent.width * 0.2
                color: "red"
                opacity: 0
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: parent.width * 0.05
                Text { anchors.centerIn: parent; text: "x"; color: "white"}

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        exitAnimation.start();
                        // unSubscribe from the transport
                        main.unSubscribe(main.chartID, component.topic, component.path);
                        // delete the series points and deattache it from the chart
                        chart.deleteSeries(component.topic + "-"+ component.path)
                        // delete the field info component
                        component.destroy();
                    }
                }
                NumberAnimation {
                    id: enterAnimation
                    target: exitBtn; property: "opacity"; duration: 200
                    easing.type: Easing.InOutQuad; from: 0; to: 1;
                }
                NumberAnimation {
                    id: exitAnimation
                    target: exitBtn; property: "opacity"; duration: 200;
                    easing.type: Easing.InOutQuad; from: 0.85; to: 0;
                }
            }
        }
    }

    // ================== Chart ============================
    ChartView {
        id : chart
        width: parent.width
        height: parent.height * 0.9
        anchors.top: infoRect.bottom

        // animations
        antialiasing: true
        opacity: 1
        backgroundRoundness: 10
        animationDuration: 400
        animationOptions: ChartView.SeriesAnimations

        theme: (Material.theme == Material.Light) ? ChartView.ChartThemeLight: ChartView.ChartThemeDark

        property var serieses: ({})
        property var textSerieses: ({})
        property var colors: ["red","blue","cyan","yellow","green","lightGray"]
        property int indexColor: 0

        Rectangle {
            id: hoverLine;
            visible: false;
            width: 2;
            height: chart.plotArea.height;
            x: chart.plotArea.x
            y:chart.plotArea.y
            color: "red"
        }

        Component {
            id: seriesText;
            Text {
                id: text
                text: "xxx"
                function setPointText(x,y) {
                    text.text = "(" + x.toString() + ", " + y.toString() + ")";
                }
            }
        }

        // for vertical line hover
        MouseArea {
            id:areaView
            anchors.fill:parent
            hoverEnabled: true

            cursorShape: (multiChartsMode) ? Qt.PointingHandCursor : Qt.ArrowCursor
            property bool flag: (hoverCheckBox.checkState === Qt.Checked) ? true : false

            onEntered: {
                if(flag && ! multiChartsMode)
                {
                    hoverLine.visible = true;
                }
                else if (multiChartsMode)
                {
                    chart.opacity = 0.7;
                }
            }
            onExited: {
                if (multiChartsMode)
                {
                    chart.opacity = 1;
                }
                hoverLine.visible = false;
            }

            // ========= Hover & Scroll ==========
            property double xHold: 0
            property double yHold: 0
            property double scrollShift : 30

            onPressed: {
                xHold = mouseX;
                yHold = mouseY;
                chart.animationOptions = ChartView.NoAnimation
            }
            onReleased: {
                chart.animationOptions = ChartView.SeriesAnimations
            }

            onPositionChanged: {
                if (multiChartsMode)
                    return

                if (pressed)
                {
                    // percentage of amount of drag
                    var xScroll = (mouseX - xHold) / (chart.plotArea.width)
                    var yScroll = (mouseY - yHold) / (chart.plotArea.height)
                    console.log(xScroll,yScroll)

                    // if xScroll or yScroll is -ne .. chart will scroll in the oposite direction
                    chart.scrollLeft(xScroll * scrollShift)
                    chart.scrollUp(yScroll * scrollShift)
                }
                else if (flag) {
                    // move the hover line with the x
                    hoverLine.x = mouseX

                    for (var ID in Object.keys(chart.textSerieses))
                    {
                        chart.textSerieses[ID].destroy();
                        delete chart.textSerieses[ID];
                    }
                    for (var key in Object.keys(serieses))
                    {
                        // get the value of the series at the mouse x position
                        // var point = chart.mapToValue(mouseX, serieses[key]);
                        // draw a text at that point to show the value of the series
                        var text = seriesText.createObject(chart, {
                                                               x : mouseX,
                                                               y : mouseY // change y to value of the curve
                                                           });
                        text.setPointText(mouseX, mouseY); // change  to value of the curve

                        textSerieses[key] = text;
                    }
                }
            }

            onClicked: {
                main.clicked(chartID);
                var axisWidth = xAxis.max - xAxis.min;
                var xPos = xAxis.min + ( (mouseX - chart.plotArea.x) / chart.plotArea.width ) * axisWidth
                var value = chart.mapToValue(5, lineSeries);
            }
            onDoubleClicked: chart.zoomReset();


            // ======== Zoom ==========
            property double shift: 15

            onWheel:{
                // the center of the plot
                var centerX = chart.plotArea.x + chart.plotArea.width/2
                var centerY = chart.plotArea.y + chart.plotArea.height/2

                // the percentage of the mouseX = how it moves far away from the plot center
                // ex: if the the plot width = 100 & mouseX = 75, so it moves the 50% away from the center (75-50)/50
                var factorX = (wheel.x - centerX) / (chart.plotArea.width/2); // %
                // same for y but with mouseY, centerY and Height
                var factorY = (wheel.y - centerY) / (chart.plotArea.height/2); // %


                var zoomType;
                if( wheel.angleDelta.y > 0)
                    // zoomIn
                    zoomType = 1;
                else
                    // zoomOut
                    zoomType = -1;


                // plot size (width & height) will always increase/decrese by 2*shift
                // (imagine the size is centered with shift distance at both sides of width (same of height) )

                // the location of zooming is determine by changing the x,y (top left corner) of the zoom rect
                // x,y increase/decrease
                var rect = Qt.rect(chart.plotArea.x + (factorX + 1) * shift * zoomType,
                               chart.plotArea.y + (factorY + 1) * shift * zoomType,
                               chart.plotArea.width  - 2 * shift * zoomType,
                               chart.plotArea.height - 2 * shift * zoomType
                               );

                chart.zoomIn(rect);
            }

        }

        Text {
            id : ray;
            text: ""
            x : chart.width/2
            y: chart.height/2
        }

        function addSeries(ID) {

            var newSeries = createSeries(ChartView.SeriesTypeLine, ID, xAxis, yAxis);
            newSeries.width = 2;
            newSeries.color = chart.colors[chart.indexColor % chart.colors.length]
            serieses[ID] = newSeries;

            chart.indexColor = (chart.indexColor + 1)  % chart.colors.length;
        }

        // delete a field series by its ID String
        function deleteSeries(ID) {
            // remove the points of the series from the chart
            removeSeries(serieses[ID]);
            // remove the series key from the serieses map
            delete serieses[ID];
        }

        ValueAxis {
            id : yAxis
            min: 0;
            max: 3;
            tickCount: 9
        }

        ValueAxis {
            id : xAxis
            min: 0
            max: 3
            tickCount: 9
        }

        // to just show the plot at begining
        LineSeries {
            id: lineSeries
            axisX: xAxis
            axisY: yAxis
            visible: false
        }

        function appendPoint(_fieldID, _x, _y) {
            console.log("(",_x,",",_y,")");

            // expand the chart boundries if needed
            if (xAxis.max  < _x)
                xAxis.max =_x ;
            if (yAxis.max  < _y )
                yAxis.max = _y ;

            if (yAxis.min > _y)
                yAxis.min = _y ;
            if (xAxis.min > _x)
                xAxis.min = _x ;

            // add the point
            chart.serieses[_fieldID].append(_x, _y);
        }
    }

    CheckBox {
        id: hoverCheckBox;
        visible: (main.multiChartsMode) ? false : true
        checkState: Qt.Unchecked
        anchors.right: chart.right
        anchors.top: chart.top
        anchors.margins: 20
        text: "hover"
    }

//    Behavior on x { NumberAnimation { duration: 100 } }
//    Behavior on y { NumberAnimation { duration: 100 } }
}
