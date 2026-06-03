import QtQuick
import QtQuick.Controls

Item {
    required property string title_of_node

    Rectangle {
        width: parent.width
        height: parent.height

        border.color: "#000000"
        color: "white"

        TextArea {
            id: node_text
            anchors.fill: parent
            anchors.margins: 3

            text: title_of_node
            color: "black"

            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter

            topPadding: 1
            leftPadding: 2

            wrapMode: TextEdit.WrapAnywhere
        }
    }

    function get_text()
    {
        return node_text.text;
    }

    function set_text(value)
    {
        node_text.text = value;
    }
}
