#include "drawiosaver.hpp"

#include <QStringBuilder>

#include <boost/lambda2.hpp>

DrawioSaver::DrawioSaver(QObject* parent) : AbstractSaver(parent)
{ }

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
    if (!is_base_element_load())
    {
        throw std::logic_error("Base element is not ready!");
    }

    QString _base = (*_elements)["base"];

    QString _result_build;
    QMap<unsigned int, QList<unsigned int>> _links;
    unsigned int last_num = 2;
    for (const unsigned int& _node_key : _graph->get_nodes_numbers())
    {
		QSharedPointer<GraphNode> _node = _graph->get_vertex(_node_key);
        unsigned int save_num = _node->get_number() + multiply_elements;

        _result_build += node_to_string(*_node, save_num) + "\n";

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
            _arrow_format = _arrow_format % last_num++ % QString() % start_node % end_node;

            _result_build += QString::fromStdString(_arrow_format.str()) + "\n";
        }
    }

    file.write(_result_build.toLatin1());
}

QString DrawioSaver::node_to_string(GraphNode &_node, const unsigned int& num)
{
    QVariant _additional_data = _node.additional_data();

    boost::format _rectangle_format((*_elements)["text_rectangle"].toStdString());
    _rectangle_format.bind_arg(0, num);

    unsigned int _x, _y;
    if (!_additional_data.isNull()) {
        NodePaintOptions _options = _additional_data.value<NodePaintOptions>();
        QString description = _options.description();
        _x = _options.getX();
        _y = _options.getY();

        if (!(description.isNull() || description.isEmpty()))
        {
            _rectangle_format.bind_arg(1, description);
        }
        else {
            _rectangle_format.bind_arg(1, _node.get_name());
        }
    }
    else {
		_rectangle_format.bind_arg(1, _node.get_name());
	}

    int param_counter = 2;
    QQueue<unsigned int> geometry_parameters { {_x, _y, _width, _height} };
    for (const unsigned int& _param : geometry_parameters)
    {
        _rectangle_format.bind_arg(param_counter, _param);

        param_counter++;
    }

	return QString::fromStdString(_rectangle_format.str());
}