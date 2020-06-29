import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3

Column {
    property int chartID: -1

    signal subscribe(real Id, string topic, string path);
    signal unSubscribe(real Id, string topic, string path);

    function appendPoint(_fieldID, _x, _y)
    {
        chart.appendPoint(_fieldID, _x, _y);
    }

    Rectangle{
        width: parent.width
        height: parent.height / 10
        color: (Material.theme == Material.Light) ? Material.color(Material.Grey,Material.Shade200)
                                                  : Material.color(Material.BlueGrey, Material.Shade800)
        Text {
            id: text
            text: ""
            anchors.centerIn: parent
        }

//        Row {
//            id: fieldArea
//            anchors.fill: parent

//        }
//        Component {
//            id: fieldInfo
//            Row {
//                anchors.fill: parent
//                Text {
//                    id: fieldname
//                }
//                Button {
//                    id: exitBtn
//                    text: "X"
//                }
//            }
//        }

        DropArea {
            anchors.fill: parent
            onDropped:
            {
                text.text = drop.text;

                // topic and path is separated with ','
                var topic_path = drop.text.split(",");
                var topic = topic_path[0];
                var path = topic_path[1];

                chart.addSeries(topic,path);

                subscribe(chartID, topic, path);
            }
        }
    }

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
            onEntered: {
                if(flag)
                {
                    // TODO : draw a vertical line and show its point value
                    flag=false;
                }
            }
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

        function addSeries(topic, path) {
            var ID = topic + "-" + path;

            // if the field is already registered
            if (ID in chart.serieses)
                return;

            var newSeries = createSeries(ChartView.SeriesTypeLine, ID, xAxis, yAxis);
            newSeries.width = 2;
            newSeries.color = chart.colors[chart.indexColor % chart.colors.length]
            serieses[ID] = newSeries;

            chart.indexColor = (chart.indexColor + 1)  % chart.colors.length;
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
