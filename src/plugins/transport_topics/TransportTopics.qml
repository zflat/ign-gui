import QtQuick 2.0
import QtCharts 2.3
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

TreeView {
    width: 300
    height: parent.height

    id:tree
    model: TopicsModel

    TableViewColumn{title: "Topics"; role:"title" ;width: parent.width ;}

    rowDelegate: Rectangle {
        color: (styleData.selected)? "red" : (styleData.row % 2 == 0) ? "white" : "grey"
        height: 50
    }

    itemDelegate: Item {
        id :item
        Text {
            text: model.name
            color: "black"
        }
    }


    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff
    headerVisible: true
    onDoubleClicked:  { tree.subscribeTopic(); tree.expandCollapseFolder(); }

    signal fieldSelected(int _index);

    function subscribeTopic()
    {
        fieldSelected(currentIndex);
        TransportTopics.subscribe(currentIndex);
    }

    function expandCollapseFolder(){
        if (tree.isExpanded(currentIndex))
            tree.collapse(currentIndex)
        else
            tree.expand(tree.currentIndex);
    }
}
