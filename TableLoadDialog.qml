import QtQml

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import TableReader

Window {
    width: 650
    height: 480
    title: "Загрузка таблицы..."

    TableReader {
        id: file_reader

        is_index_need: (is_index_read.checkState == Qt.Checked) ? true : false
        is_header_need: (is_col_titles_read.checkState == Qt.Checked) ? true : false
    }

    FileDialog {
        id: _file_getter
        currentFolder: StandartPaths.standardLocations(StandardPaths.HomeLocation)[0];

        fileMode: FileDialog.OpenFile
        nameFilters: ["Text File (*.txt, *.dat)"]

        onAccepted: function() {
            let folder_current = _file_getter.currentFile;

            file_reader.parse_file(folder_current);
        }
    }

    RoundButton {
        id: table_file_load
        x: 428
        y: 60

        width: 85
        height: 35
        radius: 12

        text: "Обзор..."

        onClicked: _file_getter.open()

    }

    Frame {
        id: table_frame
        x: 15
        y: 15
        width: 400
        height: 450
        padding: 0
        baselineOffset: 6

        Pane {
            id: table_base
            x: 6
            y: 6
            width: 388
            height: 438
            padding: 0

            GridLayout {
                id: element_link_grid
                x: 0
                y: 0
                width: 388
                height: 438
                rows: 0
                columns: 0

            }
        }
    }

    Label {
        id: file_load_title
        x: 428
        y: 18
        text: qsTr("Загрузить таблицу\nиз файла:")
    }

    RoundButton {
        id: help_by_load
        x: 520
        y: 62
        width: 30
        height: 30
        text: "?"
    }

    SpinBox {
        id: x_count
        x: 442
        y: 110
        width: 60
        height: 30
        to: 100
        from: 1
    }

    SpinBox {
        id: y_count
        x: 567
        y: 110
        width: 60
        height: 30
        to: 100
        from: 1
    }

    Label {
        id: x_title
        x: 425
        y: 117
        width: 16
        height: 18
        text: "X:"
    }

    Label {
        id: y_title
        x: 548
        y: 117
        width: 20
        height: 18
        text: "Y:"
    }

    Button {
        id: build_graph
        x: 503
        y: 431
        width: 136
        height: 34
        text: "Построить"

        onClicked: function() {
            let parsed_data = file_reader.get_table();
        }
    }

    CheckBox {
        id: is_index_read
        x: 421
        y: 157
        text: "Читать заголовки строк?"
    }

    CheckBox {
        id: is_col_titles_read
        x: 421
        y: 185
        text: "Читать заголовки столбцов?"
    }

}
