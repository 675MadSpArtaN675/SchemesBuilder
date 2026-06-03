#include "graph_painter.hpp"

#include <QException>
#include <QDebug>
#include <QAnyStringView>
#include <QColumnView>

#include <boost/format.hpp>
#include <variant>
#include <cmath>

#include <cmath>

GraphPainter::GraphPainter(QObject* parent) : GraphPainter(nullptr, new GraphOptions(100, 100, 50, 15), parent)
{ }

GraphPainter::GraphPainter(graph_data* graph, GraphOptions* options, QObject* parent) : QObject(parent), _graph_to_transform(graph), _options(options), CoreLogger()
{
    if (parent != nullptr) {
        log(std::string("Parent obj name: ") + parent->objectName().toStdString());
        _engine = qmlEngine(parent);
    }

    _difference_between_start_points = 1;
}


GraphPainter::GraphPainter(GraphPainter &&other)
{
    _graph_to_transform = std::move(other._graph_to_transform);
    _options = std::move(other._options);

    _graph_table = std::move(_graph_table);
    get_engine();
}

GraphPainter::~GraphPainter()
{
	clear_cache();
    _options.reset();
}

void GraphPainter::transform_graph(QList<QList<int>> table)
{
    if (!_graph_to_transform && table.empty())
    {
        return;
    }

    _graph_to_transform->transform_links(table);
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

void GraphPainter::get_engine(QObject *from_get)
{
    if (from_get != nullptr)
    {
        _engine = qmlEngine(from_get);
        return;
    }

    if (parent() != nullptr)
    {
        _engine = qmlEngine(parent());
        return;
    }
}

void GraphPainter::calculate_positions(double start_x, double start_y)
{
    log("Calculating positions for nodes...");
    if (!_graph_to_transform)
    {
        return;
    }

    _graph_to_transform->reindex_vertexes_numbers();

    NodePaintOptions opts(start_x, start_y);
    QQueue<NodePaintOptions> positions;
    positions.enqueue(opts);

    log((boost::format("Start position for points: (%lf;%lf)") % opts.getX() % opts.getY()).str());
    double graph_dx = 2;
    _graph_to_transform->bfs_no_result(boost::bind(&GraphPainter::calculate_one_point, this, boost::placeholders::_1, boost::placeholders::_2, std::ref(positions), std::ref(graph_dx), &opts));

}

void GraphPainter::calculate_one_point(QSharedPointer<GraphNode>& _node, int level, QQueue<NodePaintOptions>& positions, double& dx, NodePaintOptions* start_position)
{
    try {
		log_debug("Getting position...");
        NodePaintOptions pos;
        double distance = this->_options->getText_box_w() + this->_options->getDistance();

        if (!positions.empty()) {
			pos = positions.dequeue();
        }
        else
        {
            log_debug("Queue is empty. Create new start pos...");
            pos = NodePaintOptions(*start_position);
            double _y = pos.getY() + distance * dx;

            pos.setY(_y);
            start_position->setY(_y);
            log((boost::format("Generated pos = (%lf;%lf)") % pos.getX() % pos.getY()).str());

            dx += 1;
        }

        log((boost::format("Node \'%s\' has pos (%lf:%lf)") % _node->get_name().toStdString() % pos.getX() % pos.getY()).str());
		_node->set_additional_data(QVariant::fromValue(pos));

		log((boost::format("Calculating new positions for child nodes of %d node!") % _node->get_number()).str());
		int size_count = _node->get_connected_nodes_nums().size();

        if (size_count > 0) {
			double dx = this->_options->getText_box_h() + this->_options->getH_spacing(),
					max_height = (dx) * (size_count / 2);

			log_debug("Pos calculated!");
			NodePaintOptions _options = NodePaintOptions(pos.getX() + distance, pos.getY() + max_height);

			log_debug("Adding to queue all child nodes positions");
			for (int i = 0; i < size_count; i++)
			{
				NodePaintOptions _pos = _options;
				positions.enqueue(std::move(_pos));

				_options.setX(_options.getX());
				_options.setY(_options.getY() - dx);
			}
        }

		log("Node ready!");
	}
	catch (const QException& error)
	{
		log_error(std::string("Error when calculating pos: ") + error.what());
	}
}

void GraphPainter::paint_on(QQuickItem* widget)
{
    if (!_graph_to_transform || widget == nullptr)
    {
        return;
    }
    get_engine(widget);

    QString parent_name = widget->objectName();

    log((boost::format("Painting on %s widget.") % parent_name.toStdString()).str());

    _graph_to_transform->bfs_no_result(
               	boost::bind(&GraphPainter::paint_one_node, this,
                            boost::placeholders::_1, boost::placeholders::_2, widget, &_nodes));

    log("Painting finished!");
}

void GraphPainter::clear_cache()
{
    if (!_nodes.empty())
    {
		for (QQuickItem* item : _nodes)
		{
			if (item != nullptr)
			{
				item->deleteLater();
			}
		}

        _nodes.clear();
    }
}

void GraphPainter::set_start_point_difference(double difference)
{
	_difference_between_start_points = difference;
}

double GraphPainter::start_point_difference()
{
	return _difference_between_start_points;
}

QList<QList<QPointF>> GraphPainter::paint_lines(QQuickItem* canvas)
{
    if (!_graph_to_transform || canvas == nullptr)
    {
        return QList<QList<QPointF>>();
    }

    QList<QList<QPointF>> points;
    _graph_to_transform->bfs_no_result(
                boost::bind(&GraphPainter::create_lines_for_node, this,
                            boost::placeholders::_1, boost::placeholders::_2,
                            &points, &_nodes, canvas)
                        );

    log("Lines painting end! Lines count: " + std::to_string(points.size()));
    for (QList<QPointF> points : points) {
        for (QPointF point : points) {
            log("Point: " + std::to_string(point.x()) + " " + std::to_string(point.y()));
        }
    }

	return points;
}

void GraphPainter::create_lines_for_node(QSharedPointer<GraphNode>& _node, int level, QList<QList<QPointF>>* points, QMap<unsigned int, QQuickItem*>* nodes, QQuickItem* canvas)
{
    log("Paint lines between parent and childs...");

	QQuickItem* _widget = nodes->value(_node->get_number(), nullptr);

    if (_widget != nullptr) {
		QList<unsigned int> _nodes_nums = _node->get_connected_nodes_nums();
		QSet<unsigned int> _nodes_nums_unique(_nodes_nums.begin(), _nodes_nums.end());

		for (const unsigned int& index : _nodes_nums_unique)
		{
			if (nodes->contains(index))
			{
				log_debug((boost::format("Paint line between node #%d and #%d...") %
						   _node->get_number()
						   % index).str()
						);

				QList<QPointF> _line = config_line(_widget, nodes->value(index, nullptr), canvas);

				log("Lines count: " + std::to_string(_line.size()) + " of node #" + std::to_string(_node->get_number()));
				points->append(_line);
			}
		}
    }
}

void GraphPainter::paint_one_node(QSharedPointer<GraphNode>& _node, int level, QQuickItem* parent_widget, QMap<unsigned int, QQuickItem*>* _nodes)
{
	try {
		log("Start paint widget for node #" + std::to_string(_node->get_number()));
		log_debug("Extraction additional data");
		QVariant _add_data = _node->additional_data();
		NodePaintOptions _pos = get<NodePaintOptions>(_add_data);

		log_debug((boost::format("Paint on: (%ld, %ld)") % _pos.getX() % _pos.getY()).str());

		QQuickItem* _widget;
		unsigned int index = _node->get_number();
		if (!_nodes->contains(index))
		{
            log("Creating widget...");
			_widget = create_box(*_node, _node->get_name(), parent_widget);

			if (_widget != nullptr) {
				set_pos(_widget, parent_widget, _pos);
				set_size(_widget);

				_nodes->insert(index, _widget);

                log("Painting child widgets...");
				QList<GraphNode> _nodes_to_create = get_nodes(_node->get_connected_nodes());

                if (_nodes_to_create.size() > 0) {
					create_box(_nodes_to_create, QList<QString>(), parent_widget, _nodes);
                }
			}
		}
        else {
            log("Getting widget...");
            _widget = _nodes->value(index, nullptr);
        }
	}
	catch (const std::exception& error)
	{
		log(std::string("Error when painting nodes: ") + error.what());
	}
}


void GraphPainter::set_pos(QQuickItem* _to_transform_widget, QQuickItem* parent_widget, NodePaintOptions& _pos)
{
    _to_transform_widget->setParentItem(parent_widget);

    _to_transform_widget->setX(_pos.getX());
	_to_transform_widget->setY(_pos.getY());

	log(std::string("Linking widget to parent widget. Visibility: ") + std::to_string(_to_transform_widget->isVisible()));

	parent_widget->childItems().append(_to_transform_widget);
}

QList<GraphNode> GraphPainter::get_nodes(QList<QWeakPointer<GraphNode>> _child_nodes)
{
	QList<GraphNode> _nodes_to_create;

	std::transform(
		_child_nodes.begin(), _child_nodes.end(),
		std::back_inserter(_nodes_to_create),
		[](QWeakPointer<GraphNode>& _node){
			return *_node.toStrongRef();
		}
    );

    return _nodes_to_create;
}

void GraphPainter::set_size(QQuickItem *_to_transform)
{
	_to_transform->setWidth(this->_options->getText_box_w());
	_to_transform->setHeight(this->_options->getText_box_h());
}

QList<QPointF> GraphPainter::config_line(QQuickItem *_first_widget, QQuickItem *_second_widget, QQuickItem* canvas)
{
	log("Choosen canvas object: " + canvas->objectName().toStdString());

    if (_first_widget != _second_widget) {
		QPointF _pos_1, _pos_2;

        if (_first_widget->x() > _second_widget->x())
        {
            _pos_1 = calculate_center_of_side(_second_widget, _Side::Right);
        	_pos_2 = calculate_center_of_side(_first_widget, _Side::Left);
        }
        else if (_first_widget->x() < _second_widget->x()){
            _pos_1 = calculate_center_of_side(_first_widget, _Side::Right);
        	_pos_2 = calculate_center_of_side(_second_widget, _Side::Left);
        }
        else {
            return QList<QPointF>();
        }

		log_debug("Calculating distance...");

		double distance = std::abs(_pos_1.x() - _pos_2.x());
		QList<QPointF> _lines_pos {
			_pos_1,
			QPointF(_pos_1.x() + distance / 2, _pos_1.y()),
			QPointF(_pos_1.x() + distance / 2, _pos_2.y()),
			_pos_2
		};

		return _lines_pos;
    }

    return QList<QPointF>();
}

QPointF GraphPainter::calculate_center_of_side(QQuickItem* _widget, _Side _side)
{
    double _x = _widget->x(), _y = _widget->y();
	double width = _widget->width(), height = _widget->height();

    switch (_side)
    {
	case _Side::Left:
    	{
            double result_x = _x,
                   result_y = _y + height / 2;
			return QPointF(result_x, result_y);
        }
		break;

	case _Side::Right:
    	{
        	double result_x = _x + width,
                   result_y = _y + height / 2;

			return QPointF(result_x, result_y);
    	}
		break;

    default:
        throw std::logic_error("Unknown side!");
    }
}


QQuickItem* GraphPainter::create_box(QList<GraphNode> _nodes, QList<QString> descr, QQuickItem* parent, QMap<unsigned int, QQuickItem*>* exit_widgets)
{
    if (_nodes.size() > 0) {
        int width = this->_options->getText_box_w() + 30, height = 0;
        for (int i = 0; i < _nodes.size(); i++)
		{
			QString _description;

			if (i < descr.size())
			{
				_description = descr[i];
			}

            GraphNode _node = _nodes[i];
			unsigned int index = _node.get_number();

            if (!exit_widgets->contains(index)){
				QQuickItem* _node_box = create_box(_node, _description, parent);
                NodePaintOptions _pos = _node.additional_data().value<NodePaintOptions>();

				set_size(_node_box);
                set_pos(_node_box, parent, _pos);

				exit_widgets->insert(index, _node_box);
				parent->childItems().append(_node_box);
            }

            height += this->_options->getText_box_h() + this->_options->getH_spacing();
		}

		return parent;
    }

    return nullptr;
}

QQuickItem* GraphPainter::create_box(GraphNode _node, QString descr, QQuickItem* parent)
{
    log_debug(std::string("Extracting info for node's widget with #") + std::to_string(_node.get_number()));
    QString moduleName("GraphPainter");
    QString item("GraphField");

    QString node_name = _node.get_name();
    int border_pad_w = round(_options->getText_box_w() / 4), border_pad_h = round(_options->getText_box_h() / 4);
    int width = _options->getText_box_w(), height = _options->getText_box_h();

    log("Info extracted!");
    if (!descr.isEmpty())
    {
        width = border_pad_w + round(descr.size() * 1.5);
        height = border_pad_h + round(descr.count("\n") * 1.5);
    }
    else {
        width = border_pad_w + round(node_name.size() * 1.5),
        height = border_pad_h + round(node_name.count("\n") * 1.5);
    }

    log("Creating component...");
    QVariantMap _options;
    _options["title_of_node"] = node_name;

    QQmlComponent* _comp = new QQmlComponent(_engine, parent);
    _comp->loadFromModule(QAnyStringView(moduleName), QAnyStringView(item));

    log_debug("Converting to component!");
    if (_comp->isReady()) {
        QObject* _object = _comp->createWithInitialProperties(_options);

        if (_object == nullptr) {
            QString error_message("Error to create component! Error: ");
            error_message += _comp->errorString();

            log_error(error_message.toStdString());
            return nullptr;
        }

        QQuickItem* _graph_node_box = static_cast<QQuickItem*>(_object);
        _graph_node_box->childItems().append(new QQuickItem(_graph_node_box));

        _graph_node_box->setWidth(width);
        _graph_node_box->setHeight(height);

        _graph_node_box->setParentItem(parent);
        log("Item is ready...");
        return _graph_node_box;
    }
    else if (_comp->isError())
    {
        QString error_message("Error to create component! Error: ");
        error_message += _comp->errorString();

        log_error(error_message.toStdString());
        return nullptr;
    }

    log_error("Error to create box");
    return nullptr;
}

graph_data* GraphPainter::graph_to_transform() const
{
    return _graph_to_transform.get();
}

void GraphPainter::set_graph_to_transform(graph_data* new_graph_to_transform)
{
    if (new_graph_to_transform != nullptr) {
        _graph_to_transform.reset(new graph_data(*new_graph_to_transform));
    }
}

GraphOptions* GraphPainter::options()
{
	return _options.get();
}

void GraphPainter::set_options(GraphOptions* _opts)
{
	_options.reset(_opts);
}

double GraphOptions::getText_box_w() const
{
    return text_box_w;
}

void GraphOptions::setText_box_w(double newText_box_w)
{
    text_box_w = newText_box_w;
}

double GraphOptions::getText_box_h() const
{
    return text_box_h;
}

void GraphOptions::setText_box_h(double newText_box_h)
{
    text_box_h = newText_box_h;
}

double GraphOptions::getDistance() const
{
    return distance;
}

void GraphOptions::setDistance(double newDistance)
{
    distance = newDistance;
}

double GraphOptions::getH_spacing() const
{
    return h_spacing;
}

void GraphOptions::setH_spacing(double newH_spacing)
{
    h_spacing = newH_spacing;
}

GraphOptions::GraphOptions(QObject* parent) : GraphOptions(0,0,0,0, parent)
{ }

GraphOptions::GraphOptions(double _text_box_w, double _text_box_h, double _distance, double _h_spacing, QObject* parent) : QObject(parent)
{
    text_box_w = _text_box_w;
    text_box_h = _text_box_h;
    distance = _distance;
    h_spacing = _h_spacing;
}

GraphOptions::GraphOptions(GraphOptions &_other)
{
	text_box_w = _other.text_box_w;
    text_box_h = _other.text_box_h;
    distance = _other.distance;
    h_spacing = _other.h_spacing;

}

GraphOptions::GraphOptions(GraphOptions &&_other)
{
	text_box_w = std::move(_other.text_box_w);
    text_box_h = std::move(_other.text_box_h);
    distance = std::move(_other.distance);
    h_spacing = std::move(_other.h_spacing);
}

double NodePaintOptions::getX() const
{
    return x;
}

void NodePaintOptions::setX(double X)
{
    x = X;
}

double NodePaintOptions::getY() const
{
    return y;
}

void NodePaintOptions::setY(double Y)
{
    y = Y;
}

QString NodePaintOptions::description() const
{
    return _description;
}

void NodePaintOptions::setDescription(const QString &description)
{
    _description = description;
}
