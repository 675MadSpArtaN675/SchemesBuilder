#include "graph_painter.hpp"

#include <QException>
#include <QDebug>
#include <QAnyStringView>
#include <QColumnView>

#include <boost/format.hpp>
#include <boost/phoenix.hpp>
#include <boost/lambda2.hpp>

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
    _graph_to_transform.reset();
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

    _connects_count = std::make_unique<QMap<unsigned int, unsigned long>>();
    for (const unsigned int& vertex_num : _graph_to_transform->get_nodes_numbers())
    {
		_connects_count->insert(vertex_num, 0);
    }

    NodePaintOptions opts(start_x, start_y);
    QQueue<NodePaintOptions> positions;

    log((boost::format("Start position for points: (%lf;%lf)") % opts.getX() % opts.getY()).str());
    double graph_dx = 1;
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
            double _y = pos.getY() + distance + std::abs(dx);

            pos.setY(_y);
            start_position->setY(_y);
            log((boost::format("Generated pos = (%lf;%lf)") % pos.getX() % pos.getY()).str());
        }

        log((boost::format("Node \'%s\' has pos (%lf:%lf)") % _node->get_name().toStdString() % pos.getX() % pos.getY()).str());

        if (!_node->additional_data().isNull()) {
			NodePaintOptions _opts = _node->additional_data().value<NodePaintOptions>();

            _opts.setX(pos.getX());
            _opts.setY(pos.getY());

            pos = _opts;
        }

		_node->set_additional_data(QVariant::fromValue(pos));

		log((boost::format("Calculating new positions for child nodes of %d node!") % _node->get_number()).str());
		QList<unsigned int> linked_nodes = _node->get_connected_nodes_nums();
        int size_count = linked_nodes.size();

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

            std::for_each(linked_nodes.begin(), linked_nodes.end(),
                          [this](const unsigned int& vertex_num){ this->_connects_count->operator[](vertex_num)++; });
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

    unsigned int dx = 2;
    _graph_to_transform->bfs_no_result(
               	boost::bind(&GraphPainter::paint_one_node, this,
                            boost::placeholders::_1, boost::placeholders::_2, widget, &_nodes, &dx));

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

    QSet<QPoint> _ready_lines;
    QList<QList<QPointF>> points;
    _graph_to_transform->bfs_no_result(
                boost::bind(&GraphPainter::create_lines_for_node, this,
                            boost::placeholders::_1, boost::placeholders::_2,
                            &points, &_nodes, &_ready_lines, canvas)
                        );

    log("Lines painting end! Lines count: " + std::to_string(points.size()));
    for (QList<QPointF> points : points) {
        for (QPointF point : points) {
            log("Point: " + std::to_string(point.x()) + " " + std::to_string(point.y()));
        }
    }

	return points;
}

void GraphPainter::create_lines_for_node(QSharedPointer<GraphNode>& _node, int level, QList<QList<QPointF>>* points, QMap<unsigned int, QQuickItem*>* nodes, QSet<QPoint>* _points_exists, QQuickItem* canvas)
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

                if (!_line.empty()) {
                    bool is_ended = false;
                    while (!is_ended) {
						for (int i = 0; i < _line.size() - 2; i += 2)
                        {
                            QPoint _point = _line[i].toPoint();
                            if (_points_exists->contains(_point))
                            {
                                std::for_each(_line.begin(), _line.end(),
                                        		[](QPointF& point) {
													point.setY(point.y() + 15);
											});

                                is_ended = false;
                                break;
                            }
                            else {
                                is_ended = true;
                            }
                        }
                    }

					for (int i = 0; i < _line.size() - 2; i += 2)
					{
						_points_exists->insert(_line[i].toPoint());
					}

                }

				log("Lines count: " + std::to_string(_line.size()) + " of node #" + std::to_string(_node->get_number()));
				points->append(_line);
			}
		}
    }
}

void GraphPainter::paint_one_node(QSharedPointer<GraphNode>& _node, int level, QQuickItem* parent_widget, QMap<unsigned int, QQuickItem*>* _nodes, unsigned int* _dx)
{
    auto _get_description = [](GraphNode& child_node) {
		QVariant _var_obj = child_node.additional_data();

		if (_var_obj.isNull())
		{
			return QString();
		}

		return _var_obj.value<NodePaintOptions>().description();
	};

	try {
		log("Start paint widget for node #" + std::to_string(_node->get_number()));
		log_debug("Extraction additional data");
		QVariant _add_data = _node->additional_data();
		NodePaintOptions _pos = get<NodePaintOptions>(_add_data);

		log_debug((boost::format("Paint on: (%ld, %ld)") % _pos.getX() % _pos.getY()).str());

		QQuickItem* _widget;
		unsigned int index = _node->get_number();

        if (_connects_count->operator[](index) > 1)
        {
            double old_y = _pos.getY();
            old_y += _options->getDistance() * static_cast<double>(*_dx);

            _pos.setY(old_y);
            _dx++;
        }

		if (!_nodes->contains(index))
		{
            log("Creating widget...");
            QString description = (!_pos.description().isEmpty()) ? _pos.description() : QString();
			_widget = create_box(*_node, description, parent_widget);

			if (_widget != nullptr) {
				set_pos(_widget, parent_widget, _pos);
				set_size(_widget);

				_nodes->insert(index, _widget);

                log("Painting child widgets...");
                QList<QString> _descriptions_of_childs;
				QList<GraphNode> _nodes_to_create = get_nodes(_node->get_connected_nodes());
                std::transform(_nodes_to_create.begin(), _nodes_to_create.end(),
                               std::back_inserter(_descriptions_of_childs),
                               _get_description);

                if (_nodes_to_create.size() > 0) {
					create_box(_nodes_to_create, _descriptions_of_childs, parent_widget, _nodes);
                }
			}
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
    if (canvas == _first_widget || canvas == _second_widget)
    {
        return QList<QPointF>();
    }

	log("Choosen canvas object: " + canvas->objectName().toStdString());

    if (_first_widget != _second_widget) {
        log("Start line configuring...");
		QPointF _pos_1, _pos_2;

        double x_f = _first_widget->x(), y_f = _first_widget->y();
        double x_s = _second_widget->x(), y_s = _second_widget->y();

        double substract_between_y = y_f - y_s;
		double substract_between_x = x_f - x_s;
        log((boost::format("Subtract between points on x: %lf") % substract_between_x).str());
        log((boost::format("Subtract between points on y: %lf") % substract_between_y).str());

        _Side _first_wid_side = first_side_choose(substract_between_x, substract_between_y);
        _Side _second_wid_side = second_side_choose(substract_between_x, substract_between_y);

        _pos_1 = calculate_center_of_side(_first_widget, _first_wid_side);
        _pos_2 = calculate_center_of_side(_second_widget, _second_wid_side);

		log_debug("Calculating distance...");
        double _distance_x = _pos_2.x() - _pos_1.x();
        double _distance_y = _pos_2.y() - _pos_1.y();

        QPointF _pos_4, _pos_3;
        if (_first_wid_side == _Side::Left || _first_wid_side == _Side::Right) {
			_pos_4 = QPointF(_pos_1.x() + _distance_x / 2, _pos_2.y());
			_pos_3 = QPointF(_pos_1.x() + _distance_x / 2, _pos_1.y());
        }
		else {
			_pos_4 = QPointF(_pos_1.x(), _pos_2.y());
			_pos_3 = QPointF(_pos_1.x(), _pos_1.y() + _distance_y / 2);
        }

        log_debug("Positions of line are ready...");
        QList<QPointF> _lines_pos{
            _pos_1,
            _pos_3,
            _pos_4,
            _pos_2,
        };
        _lines_pos.append(get_arrow_points(_pos_4, _pos_2, 5, 10));

        log_debug("Line ready! Point count: " + std::to_string(_lines_pos.size()));
		return _lines_pos;
    }

	log_debug("Line ready!");
    return QList<QPointF>();
}

QList<QPointF> GraphPainter::get_arrow_points(QPointF point_start, QPointF point_end_, double dx, double dy)
{
    QPointF point_start_fict(point_start.x() - dx, point_start.y());
    QPointF point_end_fict(point_start.x() + dx, point_start.y());
    QPointF vector_1(point_end_fict - point_start_fict);
    QPointF vector_base(point_end_ - point_start);

    double direction = std::sqrt(std::pow(vector_base.x(), 2) + std::pow(vector_base.y(), 2));
    dy = direction - dy;
    QPointF direction_vec(vector_base.x() / direction, vector_base.y() / direction);
    if (multiply_check(vector_1, vector_base))
    {
        return {point_start_fict + direction_vec * dy, point_end_fict + direction_vec * dy};
    }

    point_start_fict = QPointF(point_start.x(), point_start.y() - dx);
    point_end_fict = QPointF(point_start.x(), point_start.y() + dx);
    vector_1 = QPointF(point_end_fict - point_start_fict);

    if (multiply_check(vector_1, vector_base))
    {
        return {point_start_fict + direction_vec * dy, point_end_fict + direction_vec * dy};
    }

    return QList<QPointF>();
}

bool GraphPainter::multiply_check(QPointF vector_1, QPointF vector_2)
{
    double _multiply_vectors = multiply_point_vectors(vector_1, vector_2);

    if (static_cast<int>(std::abs(_multiply_vectors)) == 0) {
        return true;
    }

    return false;
}

double GraphPainter::multiply_point_vectors(QPointF vector_1, QPointF vector_2)
{
    double vector_multiply = vector_1.x() * vector_2.x() + vector_1.y() * vector_2.y();
    log("Vector multiply result = " + std::to_string(vector_multiply));

    return vector_multiply;
}

GraphPainter::_Side GraphPainter::first_side_choose(double substract_between_x, double substract_between_y)
{
	log("Input x substract: " + std::to_string(substract_between_x));
    log("Input y substract: " + std::to_string(substract_between_y));

    if (substract_between_x > _min_distance_to_top && substract_between_y > _min_distance_in_height)
    {
        return _Side::Top;
    }
    else if (substract_between_x > _min_distance_to_top && substract_between_y < -_min_distance_in_height)
	{
		return _Side::Bottom;
	}
    else if (substract_between_x > _min_distance_to_top)
	{
		return _Side::Left;
	}
    else if (substract_between_x < -_min_distance_to_top)
	{
		return _Side::Right;
	}
    else if (substract_between_y > _min_distance_in_height) {
        return _Side::Bottom;
	}
	else if (substract_between_y < -_min_distance_in_height){
		return _Side::Top;
	}

    return _Side::Top;
}

GraphPainter::_Side GraphPainter::second_side_choose(double substract_between_x, double substract_between_y)
{
    log("Input x substract: " + std::to_string(substract_between_x));
    log("Input y substract: " + std::to_string(substract_between_y));

    if (substract_between_x > _min_distance_to_top)
	{
		return _Side::Right;
	}
    else if (substract_between_x < -_min_distance_to_top)
	{
		return _Side::Left;
	}
    else if (substract_between_y > _min_distance_in_height) {
        return _Side::Top;
	}
	else if (substract_between_y < -_min_distance_in_height){
		return _Side::Bottom;
	}

    return _Side::Top;
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
    case _Side::Top:
    	{
        	double result_x = _x + width / 2, result_y = _y;
            return QPointF(result_x, result_y);
    	}
        break;
    case _Side::Bottom:
		{
        	double result_x = _x + width / 2, result_y = _y + height;
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
        node_name = descr;
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
