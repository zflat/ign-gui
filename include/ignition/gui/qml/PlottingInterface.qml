import QtQuick 2.0
import QtCharts 2.2
import QtQuick.Controls 2.2
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts 1.3
import "qrc:/qml"

Rectangle
{
    id : main

    Layout.minimumWidth: 600
    Layout.minimumHeight: 600
    anchors.fill: parent
    color: (Material.theme == Material.Light) ? Material.color(Material.Grey,Material.Shade100) : "transparent"

    property var charts: ({})
    property int mainChartID: 1

    property int idIncrementor: 0

    property bool multiChartsMode: false

    // Horizonal Layout to hold multi charts (small charts)
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
                id:rowChartsLayout
                anchors.fill: parent
                spacing: 10
            }
        }
    }

    // Vertical Layout to hold Main Charts (1 or 2 charts)
    Column {
        id : chartsLayout

        width: parent.width
        anchors.topMargin: 10
        anchors.top: rowCharts.bottom
        anchors.bottom: parent.bottom
        property int heightFactor: 0
    }


    Rectangle {
        id : addBtn

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10

        width: 50
        height: 50
        radius: width/2
        color: (Material.theme == Material.Light) ? Material.color(Material.Grey, Material.Shade700)
                                                  : Material.accentColor


        Text {
            text:  "+"
            font.weight: Font.bold
            font.pixelSize: parent.width/2
            color: "white"
            anchors.centerIn: parent
        }

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: {
                addBtn.opacity = 0.8;
                cursorShape = Qt.PointingHandCursor;
            }
            onExited: {
                addBtn.opacity = 1;
                cursorShape =  Qt.ArrowCursor
            }

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
            charts[Id].x = charts[mainChartID].x;
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
        onMoveChart :function() {
            addBtn.text = "start"
            for (var _chart of Object.keys(charts))
                charts[_chart].moveChart();
            addBtn.text = "end"
        }
    }

    Component.onCompleted: {
        addChart();
    }
}

