import QtQml
import QtCore

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import TableReader
import Painters

Window {
    width: 650
    height: 480
    title: "Загрузка таблицы..."

    required property Control canvas_where_paint

    property GraphData graph_ready: null
    property var titles: file_reader.titles
    property var graph_data: file_reader.table

    GraphPainter {
        id: gr_painter

        where_paint: canvas_where_paint
        graph_to_transform: graph_ready
    }

    TableReader {
        id: file_reader

        former_of_graph_table: TableFormer {}

        delimiter: '|'
        is_index_need: (is_index_read.checkState == Qt.Checked) ? true : false
        is_header_need: (is_col_titles_read.checkState == Qt.Checked) ? true : false
    }

    FileDialog {
        id: _file_getter
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0];

        fileMode: FileDialog.OpenFile
        nameFilters: ["Text File (*.txt *.dat)"]

        onAccepted: function() {
            let folder_current = _file_getter.currentFile;

            file_reader.parse_file(folder_current);
            let graph_data_ = file_reader.table;
            let titles_ = file_reader.titles;

            if (graph_data_.length > 0)
            {
                titles_ready.clear();
                let table_grid = table_frame;

                table_grid.add_headers(titles_, graph_data_.length);
                for (let i = 0; i < graph_data_.length; i++)
                {
                    let index = i.toString();
                    if (titles_.length > i) {
                        index = titles_[i];
                    }

                    titles_ready.push(index);
                    table_grid.add_row(index, graph_data_[i]);
                }
            }
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

            Grid {
                property Component comp: Component {
                    Rectangle {
                        required property int check_state
                        required property int side_size

                        width: side_size
                        height: side_size

                        CheckBox {
                            checked: (check_state > 0) ? true : false

                            anchors.centerIn: parent
                        }

                        border.width: 1
                        border.color: "#baa7a7"
                        color: "#bababa"
                    }
                }

                property Component title_comp: Component {
                    Rectangle {
                        required property int side_size
                        property string title_of_column: ""
                        property double multiply_width: 1.0
                        property double multiply_height: 1.0

                        width: side_size * multiply_width
                        height: side_size * multiply_height

                        border.width: 1
                        border.color: "#73657d"
                        Label {
                            text: title_of_column
                            font.family: "DejaVu Sans"
                            anchors.centerIn: parent
                        }
                    }
                }

                id: element_link_grid

                width: 388
                height: 438

                rowSpacing: 0
                columnSpacing: 0

                function get_checkboxes_data()
                {
                    let result = [];
                    for (let child of children)
                    {
                        result.push(Number(child.checked));
                    }

                    return result;
                }
            }
        }

        function add_headers(row_array, count)
        {
            set_size(count);
            element_link_grid.title_comp.createObject(element_link_grid, {title_of_column: "Вершины", side_size: 50, multiply_width: 1.5});

            let row_array_len = row_array.length;
            for (let i = 0; i < count; i++)
            {
                if (row_array_len > i)
                {
                    element_link_grid.title_comp.createObject(element_link_grid, {title_of_column: row_array[i], side_size: 50});
                }
                else {
                    element_link_grid.title_comp.createObject(element_link_grid, {title_of_column: i.toString(), side_size: 50});
                }
            }
        }

        function add_row(index, row_array)
        {
            element_link_grid.title_comp.createObject(element_link_grid, {title_of_column: index, side_size: 50, multiply_width: 1.5});

            for (let state of row_array) {
                element_link_grid.comp.createObject(element_link_grid, {check_state: state, side_size: 50});
            }
        }

        function set_size(rows_and_cols)
        {
            rows_and_cols += 1;

            element_link_grid.rows = rows_and_cols;
            element_link_grid.columns = rows_and_cols;
        }

        function get_table()
        {
            let result = [];
            let table_values = element_link_grid.get_checkboxes_data();

            for (let i = 0; i < titles_ready.length; i++)
            {
                result.push([]);
                for (let j = 0; j < titles_ready.length; j++)
                {
                    result[i][j] = table_values.shift();
                }
            }

            return result;
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
        x: 425
        y: 136
        width: 117
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
        text: "Размер стороны квадрата:"
    }

    Button {
        id: build_graph
        x: 503
        y: 431
        width: 136
        height: 34
        text: "Построить"

        onClicked: function() {
            let builder = GBuilder.new("Graph_1");
            let table = table_frame.get_table();
            let titles = parent.titles;

            builder.create_nodes_from_matrix(table, titles);

            graph_ready = builder.build();

            for (let item of canvas_where_paint.children)
            {
                delete(item);
            }

            gr_painter.transform_graph(table, canvas_where_paint);
            parent.close()
        }
    }

    CheckBox {
        id: is_index_read
        x: 425
        y: 180
        text: "Читать заголовки строк?"
    }

    CheckBox {
        id: is_col_titles_read
        x: 425
        y: 208
        text: "Читать заголовки столбцов?"
    }

}
