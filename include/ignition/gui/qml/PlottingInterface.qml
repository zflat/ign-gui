import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3
import "qrc:/qml"

Column
{
    id : main

    Layout.minimumWidth: 600
    Layout.minimumHeight: 600
    anchors.fill: parent

    property var charts: ({})
    property int mainChartID: 1

    property int idIncrementor: 0
    property int buttonsHeight: 50

    property bool multiChartsMode: false

    spacing: 5

    Rectangle {
        id: rowCharts
        width: parent.width
        height: (multiChartsMode) ? 150 : 0
        color: (Material.theme == Material.Light) ? Material.color(Material.Grey,Material.Shade200)
                                                  : Material.color(Material.BlueGrey, Material.Shade800)
        // Make it Scrolable
        ScrollView {
            anchors.fill: parent
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded
            ScrollBar.vertical.policy: ScrollBar.AlwaysOff
            clip: true
            // Horizontal Layout for the Charts
            Row {
                objectName: "rrrr"
                anchors.fill: parent
                id:rowChartsLayout
                spacing: 10
            }
        }
    }
    Column {
        id : chartsLayout

        width: parent.width
        height: parent.height ? parent.height - parent.buttonsHeight - rowCharts.height : 0

        property int chartMinHeight: 300
        property int heightFactor: 0
    }

    Button {
        id : addBtn

        width: 400
        height: buttonsHeight - 5
        anchors.right: parent.right
        text:  "Add"

        onClicked: {
            if (Object.keys(charts).length == 2)
            {
                multiChartsMode = true;
                var firstChart = true;
                for (var i = 0; i < chartsLayout.children.length; i++)
                {
                    // skip the first one
                    if (firstChart)
                    {
                        firstChart = false;
                        // make that chart has a full size
                        chartsLayout.heightFactor = 1
                        continue;
                    }
                    var chart = chartsLayout.children[i];
                    reallocateChart(chart);
                }
            } // end if

            addChart();
        }
    }

    function addChart()
    {
        main.idIncrementor++;

        var chartComponent = Qt.createComponent("Chart.qml");
        var chartObject;
        // if the mode is many charts that are organized in horizontal layout
        if (multiChartsMode)
        {
            // add it to the horizontal layout
            chartObject = chartComponent.createObject(rowChartsLayout);
            chartObject.width = 200;
            chartObject.height = Qt.binding( function() {return rowCharts.height * 0.9} );
            chartObject.y = Qt.binding( function() {return (rowCharts.height - chartObject.height)/2} );
            chartObject.multiChartsMode = true;
        }
        else
        {
            // if normal mode, add the chart to the normal vertical layout
            chartObject = chartComponent.createObject(chartsLayout);
            chartObject.height = Qt.binding( function() {return chartsLayout.height / chartsLayout.heightFactor});
            chartObject.width = Qt.binding( function() {return chartsLayout.width});
            // to change the height of each vertical chart
            chartsLayout.heightFactor ++ ;
        }

        // Chart ID
        chartObject.chartID = main.idIncrementor;
        charts[idIncrementor] = chartObject;
        // Signals and Slots
        chartObject.subscribe.connect(main.onSubscribe);
        chartObject.unSubscribe.connect(main.onUnSubscribe);
        chartObject.clicked.connect(main.onClicked);
    }

    function reallocateChart(chart)
    {
        // new attributes to fit in the horizontal layout
        chart.width = 200;
        chart.height = Qt.binding( function() {return rowCharts.height * 0.9} );
        chart.y = Qt.binding( function() {return (rowCharts.height - chart.height)/2} );
        // enable that to enable the some UI features in the chart
        chart.multiChartsMode = true;

        // change the layout from the vertical layout to the top horizontal one
        chart.parent = rowChartsLayout;
    }

    function onSubscribe(Id, topic, path)
    {
        PlottingIface.subscribe(topic, path, Id);
    }
    function onUnSubscribe(Id, topic, path)
    {
        PlottingIface.unsubscribe(topic, path, Id);
    }
    function onClicked(Id)
    {
//        addBtn.text = Id.toString()
        // if many charts mode & the selected chart is in the horizontal layout
        if (multiChartsMode && charts[Id].multiChartsMode)
        {
            // ======= main chart =========
            // change the main charts properties to fit in the horizontal layout
            charts[mainChartID].width = 200;
            charts[mainChartID].height = Qt.binding( function() {return rowCharts.height * 0.9} );
            charts[mainChartID].y = (rowCharts.height - charts[mainChartID].height)/2;
            charts[mainChartID].multiChartsMode = true;

            // swap the main chart with the position of the clicked chart
            charts[mainChartID].parent = rowChartsLayout;

            // ======= swapped chart =======
            charts[Id].parent = chartsLayout;
            charts[Id].x = 0;
            charts[Id].y = 0;
            charts[Id].height = Qt.binding( function() {return chartsLayout.height / chartsLayout.heightFactor});
            charts[Id].width = Qt.binding( function() {return chartsLayout.width});
            charts[Id].multiChartsMode = false;
            charts[Id].setChartOpacity(1);

            mainChartID = Id;
        }
    }

    Connections {
        target: PlottingIface
        onPlot : function (_chart, _fieldID, _x, _y)
        {
            charts[_chart].appendPoint(_fieldID, _x, _y);
        }
    }

    Component.onCompleted: {
        addChart();
    }
}

