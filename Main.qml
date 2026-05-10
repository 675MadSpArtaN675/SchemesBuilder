import QtQuick
import QtQuick.Controls


Window {
    width: 1024
    height: 600
    visible: true
    title: qsTr("Graph Painter")

    TableLoadDialog {
        id: loader_sub_window

        modality: Qt.WindowModal
    }

    Rectangle {
        id: rectangle
        x: 6
        y: 8
        width: 745
        height: 560
        color: "#00ffffff"
        radius: 0
        border.width: 1
        clip: true

        Flickable {
            id: flickable
            x: 0
            y: 0
            width: 745
            height: 560
            contentHeight: graph_paint_area.height
            contentWidth: graph_paint_area.width
            pixelAligned: false
            antialiasing: true

            Canvas {
                id: graph_paint_area
                x: 0
                y: 0
                width: 5000
                height: 1000


            }
        }
    }

    Button {
        id: table_load
        x: 757
        y: 8
        width: 155
        height: 34
        text: "Загрузить таблицу"
        checkable: false
        highlighted: false
        flat: false

        onClicked: loader_sub_window.show()
    }

    GroupBox {
        id: groupBox
        x: 757
        y: 102
        width: 260
        height: 200
        clip: true
        title: "Элементы графа"

        Rectangle {
            id: rectangle1
            x: 0
            y: 0
            width: 240
            height: 160
            color: "#f1f1f1"
            border.width: 2
        }
    }

    GroupBox {
        id: groupBox1
        x: 757
        y: 316
        width: 260
        height: 250
        clip: true
        title: "Настройки графа"

        Rectangle {
            id: rectangle2
            x: 0
            y: 0
            width: 240
            height: 210
            color: "#f1f1f1"
            radius: 0
            border.width: 2
        }
    }

    TableLoadDialog {
        id: tableLoadDialog
        x: -39
        y: 561
    }

    TableLoadDialog {
        id: tableLoadDialog1
        x: -39
        y: 493
    }
}
