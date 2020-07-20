import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3

Column {
    id: main
    property int chartID: -1
    property bool multiChartsMode: false

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
            visible: true;
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

            property bool flag: true

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
            onPositionChanged: {
                if (flag) {
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
//                        var point = chart.mapToValue(mouseX, serieses[key]);
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
                chart.scrollRight(chart.width/10);
            }
            onDoubleClicked: chart.zoomReset();

            // ======== Under progress ==========
            property double zoomScale : 1
            property double prevX: chart.width/2
            property double prevY: chart.height/2
            property double prevWidth: chart.width
            property double prevHeight: chart.height
/*            onWheel:{
                // =========== Calculate angle ================
                var angle = 0.0;
                var wheelValue = wheel.angleDelta.y;
                if (wheelValue < 10 && wheelValue > 0)
                    angle = 0.01 * wheelValue;
                else if (wheelValue > -10 && wheelValue < 0)
                    angle = - 0.01 * wheelValue;
                else if (wheelValue === 120)
                    angle = 0.1;
                else if (wheelValue === -120)
                    angle = -0.1;


                // =============== Calculate Percentages ===============
                var percentageX_Width  = Math.abs(wheel.x - chart.width/2) / (chart.width/2);
                var percentageY_Height = Math.abs(wheel.y - chart.height/2) / (chart.height/2);

                console.log(percentageX_Width, wheel.x, chart.width/2, "   PREVX: ",prevX, prevWidth)

                if (wheel.x < prevX)
                    percentageX_Width *= -1;
                if (wheel.y < prevY)
                    percentageY_Height *= -1;

                if (zoomScale + angle > 0)
                    zoomScale += angle;

                prevX += percentageX_Width * (prevWidth/2);
                prevY += percentageY_Height * (prevHeight/2);
                prevWidth = chart.height/zoomScale;
                prevHeight = chart.width/zoomScale;
                var r = Qt.rect(prevX-prevWidth/2, prevY - prevHeight/2, prevWidth, prevHeight);

                chart.zoomReset();
                chart.zoomIn(r);
            }
*/
        }

        Text {
            id : ray;
            text: ""
            x : chart.width/2
            y: chart.height/2
        }

        // initial animation
        Component.onCompleted: {
            initialAnimation.start();
        }

        NumberAnimation {
            id: initialAnimation
            running: false
            target: view
            property: "scale"
            duration: 1200
            easing.type: Easing.OutBounce
            from:0 ; to:1;
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
            axisX: xAxis
            axisY: yAxis
            visible: false
        }

        function appendPoint(_fieldID, _x, _y) {
            console.log("(",_x,",",_y,")");

            // expand the chart boundries if needed
            if (xAxis.max -4 < _x)
                xAxis.max =_x +4 ;
            if (yAxis.max -4 < _y )
                yAxis.max = _y + 4 ;

            if (yAxis.min > _y)
                yAxis.min = _y - 1;
            if (xAxis.min > _x)
                xAxis.min = _x - 1;

            // add the point
            chart.serieses[_fieldID].append(_x, _y);
        }
    }
}
