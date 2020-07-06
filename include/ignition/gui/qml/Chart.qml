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

    // =============== Fields info Rectangle ================
    Rectangle{
        id: infoRect
        width: parent.width
        height: parent.height / 10
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
            field.y = Qt.binding( function() {return (infoRect.height - field.height)/2} );

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
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: enterAnimation.start();
                    onExited: exitAnimation.start();
                }

                Text {
                    id: fieldname
                    anchors.centerIn: parent
                    text: component.topic + "-"+ component.path
                    color: "white"
                    elide: Text.ElideRight
                }
            }

            function setText(text) {
                fieldname.text = text;
            }
            signal unsubscribe(string topic, string path);

            Rectangle {
                id: exitBtn
                radius: width / 2
                height: parent.height * 0.5;
                width: parent.width * 0.15
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
        property var colors: ["red","blue","cyan","yellow","green","lightGray"]
        property int indexColor: 0

        // for vertical line hover
        MouseArea {
            id:areaView
            anchors.fill:parent
            hoverEnabled: true
            property bool flag: false
            cursorShape: (multiChartsMode) ? Qt.PointingHandCursor : Qt.ArrowCursor

            onEntered: {
                if(flag && ! multiChartsMode)
                {
                    // TODO : draw a vertical line and show its point value
                    flag=false;
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
            }

            onClicked: main.clicked(chartID);
        }

        // initial animation
        Component.onCompleted:{
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

            // add the point
            chart.serieses[_fieldID].append(_x, _y);
        }
    }
}
