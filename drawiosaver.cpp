#include "drawiosaver.hpp"

#include <QStringBuilder>

#include <boost/lambda2.hpp>

DrawioSaver::DrawioSaver(QObject* parent) : AbstractSaver(parent)
{
	_width = 150;
    _height = 45;
}

unsigned int DrawioSaver::box_width()
{
    return _width;
}

void DrawioSaver::set_box_width(unsigned int width)
{
    _width = width;
}

QBindable<unsigned int> DrawioSaver::bindable_box_width()
{
    return QBindable<unsigned int>(&_width);
}

unsigned int DrawioSaver::box_height()
{
    return _height;
}

void DrawioSaver::set_box_height(unsigned int height)
{
    _height = height;
}

QBindable<unsigned int> DrawioSaver::bindable_box_height()
{
    return QBindable<unsigned int>(&_height);
}

void DrawioSaver::save_logic(QFile &file)
{
    if (_graph != nullptr) {
		log_debug((QString("Now saving in ") + file.fileName()).toStdString());

		if (!is_base_element_load())
		{
			throw std::logic_error("Base element is not ready!");
		}

		QString _base = (*_elements)["base"];
        log_debug("Base element: " + _base.toStdString());

		QString _result_build;
		QMap<unsigned int, QList<unsigned int>> _links;
		unsigned int last_num = 2;
		log("Getting base element...");
		for (const unsigned int& _node_key : _graph->get_nodes_numbers())
		{
			log("Start parsing node: " + std::to_string(_node_key));

			QSharedPointer<GraphNode> _node = _graph->get_vertex(_node_key);
			unsigned int save_num = _node->get_number() + multiply_elements;
			log_debug("Saving #" + std::to_string(save_num) + " node.");

			_result_build += node_to_string(*_node, save_num) + "\n";

            log_debug("Save node to table...");
			QList<unsigned int> _connected = _node->get_connected_nodes_nums();
			_links[save_num] = QList<unsigned int>();
			std::transform(_connected.begin(), _connected.end(),
						   std::back_inserter(_links[save_num]),
						   boost::lambda2::_1 + multiply_elements);

			last_num = save_num;
		}
		_result_build += "\n";
		last_num++;

		for (auto iter = _links.constKeyValueBegin(); iter != _links.constKeyValueEnd(); iter++)
		{
			const unsigned int start_node = iter->first;
			const QList<unsigned int> _to_connect = iter->second;

			for (const unsigned int& end_node : _to_connect)
			{
				boost::format _arrow_format((*_elements)["arrow"].toStdString());
				_arrow_format = _arrow_format % last_num++ % std::string() % start_node % end_node;

				_result_build += QString::fromStdString(_arrow_format.str()) + "\n";
			}
		}

        boost::format _ready_drawio_file(_base.toStdString());
        _ready_drawio_file.bind_arg(1, _result_build.toStdString());

		file.write(QString::fromStdString(_ready_drawio_file.str()).toLatin1());
    }
}

QString DrawioSaver::node_to_string(GraphNode &_node, const unsigned int& num)
{
    log("Saving node #" + std::to_string(num));
    QVariant _additional_data = _node.additional_data();

    std::string format_string = (*_elements)["text_rectangle"].toStdString();
    log("Format string: " + format_string);

    boost::format _rectangle_format(format_string);
    _rectangle_format.bind_arg(1, num);

    unsigned int _x, _y;
    if (!_additional_data.isNull()) {
        NodePaintOptions _options = _additional_data.value<NodePaintOptions>();
        QString description = _options.description();
        _x = _options.getX();
        _y = _options.getY();

        if (!(description.isNull() || description.isEmpty()))
        {
            _rectangle_format.bind_arg(2, escapeXml(description).toStdString());
        }
        else {
            _rectangle_format.bind_arg(2, escapeXml(_node.get_name()).toStdString());
        }
    }
    else {
		_rectangle_format.bind_arg(2, escapeXml(_node.get_name()).toStdString());
	}

    int param_counter = 3;
    QQueue<unsigned int> geometry_parameters { {_x, _y, _width, _height} };
    for (const unsigned int& _param : geometry_parameters)
    {
        _rectangle_format.bind_arg(param_counter, _param);

        param_counter++;
    }

	return QString::fromStdString(_rectangle_format.str());
}