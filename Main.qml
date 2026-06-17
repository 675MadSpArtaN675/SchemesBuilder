import QtCore
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import QtQuick.Dialogs

import TableReader
import GraphPaint

Window {
	minimumWidth: 1148
    minimumHeight: 600

    title: qsTr("Graph Painter")

    visible: true

    Settings {
        id: application_settings

        property int box_width: 100
        property int box_height: 75
        property double distance_between_graph_levels: 100
        property int height_spacing: 15
		property double start_point_dx: 1.0

		property int canvas_w: 800
		property int canvas_h: 800
    }

    GraphPainter {
        id: painter

		start_point_dx: application_settings.start_point_dx

        options: GraphOptions {
           	box_width: Number(node_width.value)
            box_height: Number(node_height.value)
            distance: parseFloat(node_w_spacing.value)
            h_spacing: Number(node_h_spacing.value)
        }
    }

    TableLoadDialog {
        id: loader_sub_window

        modality: Qt.WindowModal
        onGraphBuilded: function(){
            let table = get_table();

			if (table.length > 0) {
				graph_base.clear();

				console.log("Getting link table...");
				painter.graph_to_transform = graph_ready;

				console.log("Creating links and calculating position...");
				painter.transform_graph(table);
				painter.calculate_positions(25, graph_paint_area.height / 3);

				console.log("Painting nodes...");
				painter.paint_on(graph_paint_area);
				let graph_nodes = painter.paint_lines(graph_paint_area);
				console.log("Created widgets: ", graph_nodes);

				graph_paint_area.line_points = graph_nodes;
				console.log("Requesting paint...");
				graph_paint_area.requestPaint();
				console.log("Done!");
			}
        }
    }

    Grid {
        id: app_grid

        padding: 5

        width: parent.width
        height: parent.height
        rowSpacing: 5

		Rectangle {
			id: graph_base
			clip: true

			width: 800
			height: 575

            radius: 2

            border.color: "black"
            color: "gray"

			Flickable {
                id: flickable_area
				x: 2
				y: 2

				boundsBehavior: Flickable.StopAtBounds
				flickDeceleration: 5000

				width: parent.width - 4
				height: parent.height - 4
                clip: true

                contentWidth: graph_paint_area.width
                contentHeight: graph_paint_area.height

				Canvas {
					id: graph_paint_area

					property var line_points: []

					width: canvas_width_.value
					height: canvas_height_.value

                    onPaint: {
                        console.log("Painting canvas elements...");
                        let ctx = getContext("2d");
                        ctx.reset();

                        console.log("Clearing canvas..");
                        ctx.fillStyle = Qt.rgba(255, 255, 255, 1);
                        ctx.fillRect(0, 0, width, height);

                        console.log("Render lines");

						let dx = 6;
						let existed_points = new Set();
                        for (let line of line_points) {
							let line_len = line.length - 1;
                            for (let i = 0; i < line_len; i++ ) {
                                let _point_1 = line[i];
                                let _point_2 = line[i + 1];

								ctx.beginPath();
								ctx.moveTo(_point_1.x, _point_1.y);
								ctx.lineTo(_point_2.x, _point_2.y);

								if (line_len - 2 < i) {
									let end_y_1 = _point_2.y + 5, end_y_2 = _point_2.y - 5;
									let end_x = _point_2.x - 10;

									ctx.moveTo(end_x, end_y_1);
									ctx.lineTo(_point_2.x, _point_2.y);

									ctx.moveTo(end_x, end_y_2);
									ctx.lineTo(_point_2.x, _point_2.y);
								}

								ctx.strokeStyle = Qt.rgba(0, 0, 0, 1);
								ctx.lineWidth = 1;
								ctx.stroke();
                                ctx.closePath();
                            }
                        }

                        console.log("Done");
                    }
                    Component.onCompleted: requestPaint()
				}
			}

			function clear()
            {
                if (graph_paint_area.children.length > 0)
				{
					painter.clear_cache();
					loader_sub_window.clear_cache();

                    graph_paint_area.line_points = [];
                    graph_paint_area.requestPaint();
				}
            }

		}

        Column {
			Column {
				width: 250
				height: 200

				spacing: 10
                leftPadding: 10

				MessageDialog {
					id: error_notifier
					text: "Не удалось сохранить файл!"
				}

				FileDialog {
					id: graph_image_saver

					fileMode: FileDialog.SaveFile
					nameFilters: ["PNG image (*.png)", "JPG/JPEG image (*.jpg *.jpeg)"]

					defaultSuffix: ".png"

					onAccepted: {
						let file = selectedFile;

						graph_paint_area.grabToImage(
							function(result){
								console.log("Saving file to image");
								let is_saved = result.saveToFile(file);

								console.log(is_saved.source);
								if (!is_saved)
								{
									error_notifier.open();
									console.log("Error of file saving");
								}
								else {
									console.log(`Graph saved in file: ${file}`);
								}
							}
						);
					}
				}

				Button {
					width: parent.width * 1.2
					height: 40

					text: "Создать граф по таблице"

					onClicked: loader_sub_window.show()
				}

				Button {
					width: parent.width * 1.2
					height: 40

					text: "Очистить граф"

					onClicked: graph_base.clear()
				}

				Button {
					width: parent.width * 1.2
					height: 40

					text: "Сохранить граф как изображение"

					onClicked: graph_image_saver.open()
				}
			}

			Column {
                leftPadding: 10
                spacing: 25

				Row {
					spacing: 25

					Column {
						Label {
							text: qsTr("Ширина ноды:")
						}

						SpinBox {
							id: node_width
							editable: true

							stepSize: 1
							value: application_settings.box_width
							from: 1
							to: 1000
						}
					}
					Column {
						Label {
							text: qsTr("Высота ноды:")
						}

						SpinBox {
							id: node_height
							editable: true

							stepSize: 1
							value: application_settings.box_height

							from: 1
							to: 1000
						}
					}
				}

				Column {
					Column {
						Label {
							text: qsTr("Отступ между\nнодами по ширине:")
						}

						SpinBox {
							id: node_w_spacing

							width: 150

							editable: true

							stepSize: 1
							value: application_settings.distance_between_graph_levels

							from: 1
							to: 500
						}
					}
					Column {
						Label {
							text: qsTr("Отступ между\nнодами по высоте:")
						}

						SpinBox {
							id: node_h_spacing

							width: 150

							editable: true

							stepSize: 1
							value: application_settings.height_spacing

							from: 1
							to: 500
						}
					}
				}

				Row {
					spacing: 25

					Column {
						Label {
							width: 150
							text: qsTr("Ширина полотна:")
						}

						SpinBox {
							id: canvas_width_
							width: 150

							editable: true

							stepSize: 1

							from: application_settings.canvas_w
							to: Math.pow(10, 8)
						}
					}

					Column {
						Label {
							width: 125
							text: qsTr("Длина полотна:")
						}

						SpinBox {
							id: canvas_height_
							width: 150

							editable: true

							stepSize: 1

							from: application_settings.canvas_h
							to: Math.pow(10, 8)
						}
					}
				}
			}
        }
    }
}
