import QtQml
import QtCore

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs

import TableReader
import GraphPaint

Window {
    id: table_win

    minimumWidth: 940
    minimumHeight: 660

    title: "Загрузка таблицы..."

    property var graph_ready: null

    property var titles: file_reader.titles
    property var graph_data: file_reader.table
    property var titles_ready: []

	TableReader {
        id: file_reader

        former_of_graph_table: TableFormer {}

        delimiter: '|'
        is_index_need: (is_index_read.checkState == Qt.Checked) ? true : false
        is_header_need: (is_column_read.checkState == Qt.Checked) ? true : false
    }

    GraphBuilder {
		id: builder

        reader: file_reader
	}

    FileDialog {
        id: _file_getter
        currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0];

        fileMode: FileDialog.OpenFile
        nameFilters: ["Text File (*.txt *.dat)"]
        onAccepted: function() {
            let folder_current = currentFile;

            file_reader.parse_file(folder_current);
            let graph_data_ = file_reader.table;
            let titles_ = file_reader.titles;

			create_graph(graph_data_, titles_);
        }
    }

	function clear_cache() {
		console.log("Clearing table...");
		table_frame.clear_grid();
		file_reader.clear();

		console.log("Clearing cache!");
        builder.clear();
		titles_ready = [];
        x_count.textFromValue(1);

		console.log("Graph cleared!");
	}

    function get_graph()
    {
        return graph_ready;
    }

    function get_table()
    {
        return table_frame.get_table();
    }

    function create_graph(graph_data_, titles_)
    {
		let table_grid = table_frame;
        let table_len = table_grid.get_size();
        let titles_size = table_grid.get_size() - 1;

        if (graph_data_.length > 0)
		{
			table_frame.clear_grid();
			titles_ready = [];

			for (let row of graph_data_)
			{
				console.log(row);
			}

			table_len = graph_data_.length;
			titles_size = titles_.length;

			console.log(`Размер стороны матрицы: ${table_len}`);
			console.log(`Количество столбцов: ${titles_size}`);

			x_count.textFromValue(table_len + 1);
			table_grid.set_size(table_len + 1);
			table_grid.add_headers(titles_, table_len);

            for (let i = 0; i < table_len; i++)
			{
				let index = i.toString();
				if (titles_size > i) {
					index = titles_[i];
				}

				titles_ready.push(index);
				table_grid.add_row(index, graph_data_[i]);
			}
		}
    }

    signal graphBuilded(var data)
    onGraphBuilded: console.log("Builded!");

    Frame {
        id: table_frame

        anchors.top: parent.top
        anchors.left: parent.left

        anchors.topMargin: 15
        anchors.leftMargin: 15

        width: 600
        height: 600
        padding: 0

        Pane {
            id: table_base

            anchors.fill: parent
            anchors.margins: 4

            background: Rectangle {
				implicitWidth: 1
				implicitHeight: 1

                border.color: "black"
                color: "#cacaca"
			}

            clip: true

            width: 440
            height: 440
            padding: 0

            Flickable {

				Grid {
					property Component comp: Component {
						Rectangle {
							required property int check_state
							required property int side_size

							width: side_size
							height: side_size

							CheckBox {
								id: vertex_link
								checked: (check_state > 0) ? Qt.Checked : Qt.Unchecked

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
								font.pointSize: 10
								anchors.centerIn: parent

								color: "#000000"
							}
						}
					}

					id: element_link_grid

					width: 400
					height: 440

					rowSpacing: 0
					columnSpacing: 0

					function get_checkboxes_data()
					{
						let result = [];
						for (let child of element_link_grid.children)
						{
							if (child.children[0].checked !== undefined) {
								let num = Number(child.children[0].checked);
								result.push(num);
							}
						}

						return result;
					}
				}
            }
        }

        function get_size()
        {
            return element_link_grid.columns;
        }

		function clear_grid()
        {
            for (let child of element_link_grid.children)
			{
				child.destroy();
			}
        }
        function write_table(table)
        {
            let rows = table.length + 1;
            let columns = table[0].length + 1;
            let elements_count = element_link_grid.children.length;

            if (rows * columns === elements_count)
            {
				for (let i = 1; i < rows; i++)
				{
					for (let j = 1; j < columns; j++)
					{
						element_link_grid.children[i * columns + j].children[0].checked = Boolean(table[i - 1][j - 1]);
					}
				}
            }
        }

        function resize_column(col_number)
        {
            let width = 0, height = 0;
            let matrix_size = get_size();
            for (let i = 0; i < matrix_size; i++)
            {
                for(let j = 0; j < matrix_size; j++)
                {
                    let index = i * matrix_size + j;
                    let child_item = element_link_grid.children[index];

                    if (j == 0)
                    {
                        width = child_item.width;
                        height = child_item.height;
                    }
                    else
                    {
                        child_item.width = width;
                        child_item.height = height;
                    }
                }
            }
        }

        function add_headers(row_array, count)
        {
            element_link_grid.title_comp.createObject(element_link_grid, {title_of_column: "Откуда/Куда", side_size: 50, multiply_width: 1.5});

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
                    result[i][j] = Number(table_values.shift());
                }
            }

            return result;
        }

    }

    Column {
        anchors.left: table_frame.right
        anchors.top: table_frame.top
        anchors.leftMargin: 15

        spacing: 15

		Column {
            spacing: 5

			Label {
				id: file_load_title

				text: qsTr("Загрузить таблицу\nиз файла:")

				color: "#000000"
			}

			Row {
                spacing: 5

				RoundButton {
					id: table_file_load

					width: 85
					height: 35
					radius: 12

					text: "Обзор..."

					onClicked: _file_getter.open()
				}

				RoundButton {
					id: help_by_load

					width: 30
					height: 30

					text: "?"
				}
			}
		}

		Column {
            spacing: 5

			Label {
				id: x_title

				width: 20
				height: 20

				text: "Размер стороны квадрата:"

				color: "#000000"
			}

			SpinBox {
				id: x_count
                editable: true

				width: 250
				height: 30

				to: 50
				from: 1

				onValueModified: {
					console.log("Clearing log");
					clear_cache();

					let line_vertex_count = value;
					let headers = [];
					let init_row = [];

					console.log(`Creating table with size: ${line_vertex_count}`);
					table_frame.set_size(line_vertex_count + 1);
					for (let i = 0; i < line_vertex_count; i++)
					{
						console.log(`Add column ${i}...`);
						headers.push(i.toString());
						init_row.push(0);
					}

					console.log(`Headers: ${headers}`);
					table_frame.add_headers(headers, line_vertex_count);
					titles_ready = headers;
					for (let index of headers)
					{
						console.log(`Init row: ${index} ${init_row}`);
						table_frame.add_row(index, Array.from(init_row));
					}
					console.log("Table created");
					console.log("Table: ", table_frame.get_table());
				}
			}
		}

        Column {
            spacing: 1

            Label {
                width: 150
                height: 20

                text: "Настройки: "
                anchors.bottomMargin: 5
            }

            CheckBox {
				id: is_index_read
                width: 250
                height: 50

				text: "Считывать заголовки\nстрок таблицы?"
			}

            CheckBox {
                id: is_column_read
                width: 250
                height: 50
                text: "Считывать заголовки\nтаблицы?"
            }
        }

		Column {
            spacing: 15

            Column {
                spacing: 1

				Label {
					width: 150
					height: 20

					text: "Выбор типа формера таблиц:"
				}

				ComboBox {
					id: former_type_chooser

					width: 250

					textRole: "text"
					model: ListModel {
						ListElement { text: "Standard"; value: TableReader.Standart }
						ListElement { text: "Databased Table"; value: TableReader.DatabasedType }
					}

					onActivated: function() {
                        console.log("Switching!");
						file_reader.switch_former(currentValue.value);
					}
				}
            }

            Column {
                spacing: 1

                Label {
                    width: 175
                    height: 25

                    text: "Указание связей..."
                }

                TextArea {
                    id: nodes_links

                    width: 275
                    height: 150

                    background: Rectangle {
                        implicitWidth: 1.0
                        implicitHeight: 1.0

                        border.width: 1
                        border.color: "black"

                        color: "white"
                    }

                    wrapMode: TextArea.WrapAnywhere
                }

				CheckBox {
					id: is_clear_old_links

					text: qsTr("Очищать старые связи?")
				}
            }
		}
    }
	Button {
		id: clear_table

        anchors.bottom: build_graph.bottom
        anchors.right: build_graph.left

        anchors.rightMargin: 10

		width: 135
		height: 35

		text: "Очистить"

		onClicked: clear_cache()
	}

    Button {
		id: build_graph

		width: 135
		height: 35

		anchors.right: parent.right
		anchors.bottom: parent.bottom

        anchors.rightMargin: 10
        anchors.bottomMargin: 10

		text: "Построить"

		onClicked: function() {
			let table = table_frame.get_table();
			console.log("Getting table: ", table);

			if (table.length > 0) {
                builder.clear();
				console.log("Applying table");
				console.log("Titles:", titles_ready);
				builder.create_nodes_from_matrix(table, titles_ready);

                console.log("Connecting to string...");
				builder.connect_all_vertexes_by_string(nodes_links.text, table, is_clear_old_links.checked);
                table_frame.write_table(table);

				console.log("Building graph");
				let graph_ready_ = builder.build_ptr();
				console.log("Build result", graph_ready_, !builder.is_empty());

				if (graph_ready_ !== null && !builder.is_empty()) {
					console.log("Graph cached");
					graph_ready = graph_ready_;
					graphBuilded(graph_ready_);
				}
			}
			console.log("Closing window");
			close();
		}
	}
}
