#include "graph_painter.hpp"

#include <QAnyStringView>

#include <cmath>

GraphPainter::GraphPainter() : GraphPainter(nullptr, nullptr)
{}

GraphPainter::GraphPainter(graph_data* graph, GraphOptions* options) : _graph_to_transform(graph), _options(options), CoreLogger()
{ }

GraphPainter::GraphPainter(GraphPainter &&other)
{
    _graph_to_transform = std::move(other._graph_to_transform);
    _options = std::move(other._options);

    _graph_table = std::move(_graph_table);
}

void GraphPainter::transform_graph(QList<QList<int>> table)
{
    if (!_graph_to_transform && table.empty())
    {
        return;
    }

    if (_graph_to_transform)
    {
        _graph_to_transform->transform_links(table);
    }
}

void GraphPainter::transform_graph(hmdf::Matrix<int> table)
{
    if (!_graph_to_transform && table.empty())
    {
        return;
    }

    QList<QList<int>> _to_perform(table.rows(), QList<int>());
    for (int i = 0; i < table.rows(); i++)
    {
        for (int j = 0; j < table.cols(); j++)
        {
            _to_perform[i][j] = table(i, j);
        }
    }

    transform_graph(_to_perform);
}

QQuickWidget* GraphPainter::create_box(QList<GraphNode> _nodes, QList<QString> descr, QWidget* parent)
{
    QAnyStringView _module(QString::fromStdString("QtQuick"));
    QAnyStringView _item(QString::fromStdString("Column"));

    QQuickWidget* _column = new QQuickWidget(_module, _item);
    _column->setParent(parent);

    for (int i = 0; i < _nodes.count(); i++)
    {
        QString _description;

        if (i < descr.size())
        {
            _description = descr[i];
        }

        create_box(_nodes[i], _description, _column);

    }

    return _column;
}

QQuickWidget* GraphPainter::create_box(GraphNode _node, QString descr, QWidget* parent)
{
    QAnyStringView _module(QString::fromStdString("GraphPainter"));
    QAnyStringView _item(QString::fromStdString("GraphField"));

    QString node_name = _node.get_name();
    int border_pad_w = round(_options->text_box_w / 4), border_pad_h = round(_options->text_box_h / 4);
    int width = _options->text_box_w, height = _options->text_box_h;

    if (!descr.isEmpty())
    {
        width = border_pad_w + round(descr.size() * 1.5);
        height = border_pad_h + round(descr.count("\n") * 1.5);
    }
    else {
        width = border_pad_w + round(node_name.size() * 1.5),
        height = border_pad_h + round(node_name.count("\n") * 1.5);
    }

    QQuickWidget* _graph_node_box = new QQuickWidget(_module, _item);

    _graph_node_box->setGeometry(QRect(0, 0, width, height));
    _graph_node_box->setProperty("text", _node.get_name());
    _graph_node_box->setParent(parent);

    return _graph_node_box;
}

graph_data* GraphPainter::graph_to_transform() const
{
    return _graph_to_transform.get();
}

void GraphPainter::set_graph_to_transform(graph_data* new_graph_to_transform)
{
    _graph_to_transform.reset(std::move(new_graph_to_transform));
}

GraphOptions::GraphOptions(double _text_box_w, double _text_box_h, double _distance, double _height)
{
    text_box_w = _text_box_w;
    text_box_h = _text_box_h;
    distance = _distance;
    heigth = _height;
}
