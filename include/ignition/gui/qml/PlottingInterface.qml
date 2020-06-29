import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3
import "qrc:/qml"

Column
{
    id : main

    Layout.minimumHeight: 600
    Layout.minimumWidth: 480
    anchors.fill: parent

    property var charts: { 1 : chart }

    property int idIncrementor: 1
    property int buttonsHeight: height / 20

    Column {
        id : chartsLayout

        width: parent.width
        height: parent.height ? parent.height - parent.buttonsHeight : 0

        property int chartMinHeight: 300
        property int heightFactor: 1

        Chart
        {
            id : chart

            chartID: 1

            height: chartsLayout.height / chartsLayout.heightFactor
            width: chartsLayout.width
        }

        function addChart()
        {
            main.idIncrementor++;
            chartsLayout.heightFactor ++ ;

            var chartComponent = Qt.createComponent("Chart.qml");
            var chartObject = chartComponent .createObject(
                        chartsLayout,
                        {
                            chartID: main.idIncrementor,
                            height: Qt.binding( function() {return chartsLayout.height / chartsLayout.heightFactor} ),
                            width : Qt.binding( function() {return chartsLayout.width} )
                        });

            charts[idIncrementor] = chartObject;
            chartObject.subscribe.connect(main.onSubscribe);
        }

        Component.onCompleted: {
            chart.subscribe.connect(main.onSubscribe);
        }


    }

    Button {
        id : addBtn

        width: 100
        height: 30
        anchors.right: parent.right
//        anchors.margins: 10

        text: "Add"

        onClicked: {
            chartsLayout.addChart();
        }
    }
    Text {
        id: name
        text: qsTr("text")
        width: 100
        height: 20
        color: "white"
    }

    Connections {
        target: PlottingIface
        onPlot : function (_chart, _fieldID, _x, _y)
        {
            charts[_chart].appendPoint(_fieldID, _x, _y);
        }
    }


    function onSubscribe(Id, topic, path)
    {
        PlottingIface.subscribe(topic, path, Id);
    }
}

