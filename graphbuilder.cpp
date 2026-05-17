#include "graphbuilder.hpp"

#include <boost/format.hpp>


GraphBuilder::GraphBuilder() : GraphBuilder(STANDART_GRAPH_NAME)
{ }

GraphBuilder::GraphBuilder(QString name) : _name{name}, CoreLogger()
{ }

GraphBuilder::GraphBuilder(GraphBuilder &other_builder)
{
    _name = other_builder._name;
    _nodes_links = other_builder._nodes_links;
    _temp_nodes = other_builder._temp_nodes;
}

GraphBuilder::GraphBuilder(GraphBuilder &&other_builder)
{
    _name = std::move(other_builder._name);
    _nodes_links = std::move(other_builder._nodes_links);
    _temp_nodes = std::move(other_builder._temp_nodes);
}

GraphBuilder::~GraphBuilder()
{ }

GraphBuilder &GraphBuilder::create_nodes_from_matrix(int_hmdf_matrix &_matrix, QList<QString> names)
{
    unsigned int max_index = get_max_index();

    int size_rows = _matrix.rows();
    int size_columns = _matrix.cols();

    if (size_rows != size_columns)
    {
        throw std::logic_error("Vector is not square!");
    }

    for (int i = 0; i < size_rows; i++)
    {
        _matrix(i, i) = 0;
    }

    for (int i = 0; i < size_rows; i++)
    {
        QString name = !names.isEmpty() ? names[i] : QString::fromStdString(std::to_string(max_index + 1 + i));
        GraphNode _node(max_index + 1 + i, name);

        for (int j = 0; j < size_columns; j++) {
            connect_node_to_node(i, j);
        }

        _temp_nodes.insert(i, std::move(_node));
    }

    return *this;
}

GraphBuilder &GraphBuilder::create_nodes_from_matrix(vector2d &_matrix, QList<QString> names)
{
    unsigned int max_index = get_max_index();

    int size_rows = _matrix.size();
    int size_columns = _matrix[0].size();

    if (size_rows != size_columns)
    {
        throw std::logic_error("Vector is not square!");
    }

    for (int i = 0; i < size_rows; i++)
    {
        _matrix[i][i] = 0;
    }

    for (int i = 0; i < size_rows; i++)
    {
        QString name = !names.isEmpty() ? names[i] : QString::fromStdString(std::to_string(max_index + 1 + i));
        GraphNode _node(max_index + 1 + i, name);

        for (int j = 0; j < size_columns; j++) {
            connect_node_to_node(max_index + 1 + i, max_index + 1 + j);
        }

        _temp_nodes.insert(i, std::move(_node));
    }

    return *this;
}

GraphBuilder &GraphBuilder::create_nodes_from_matrix(QList<QList<int> > &_matrix, QList<QString> names)
{
    vector2d _transformed_lists;
    std::transform(_matrix.begin(), _matrix.end(), std::back_inserter(_transformed_lists), [](QList<int>& _row){
        return std::vector<int>(_row.begin(), _row.end());
    });

    return create_nodes_from_matrix(_transformed_lists, names);
}

GraphBuilder &GraphBuilder::create_nodes_from_preprocessor(TableFormer* _matrix)
{
    std::vector<std::string> graph_names = _matrix->get_columns_names();
    hmdf::Matrix<int> _graph_matrix = _matrix->to_matrix();

    QList<QString> graph_names_performed;
    std::transform(graph_names.begin(), graph_names.end(), std::back_inserter(graph_names_performed), [](std::string& names){
        return QString::fromStdString(names);
    });

    create_nodes_from_matrix(_graph_matrix, graph_names_performed);

    return *this;
}

GraphBuilder &GraphBuilder::get_nodes_from_other_graph(const graph_data &_other_graph)
{
    QList<unsigned int> _nodes_numbers = _other_graph.get_nodes_numbers();
    unsigned int max_index = get_max_index();

    QList<GraphNode> _nodes;
    for (unsigned int& node_num : _nodes_numbers) {
        QSharedPointer<GraphNode> _node = _other_graph.get_vertex(node_num);
        GraphNode _node_copy = GraphNode(*_node);

        _node_copy.set_number(max_index + 1 + _node_copy.get_number());

        QList<unsigned int> connected_nodes_nums = _node_copy.get_connected_nodes_nums();
        std::for_each(connected_nodes_nums.begin(), connected_nodes_nums.end(), [&max_index](unsigned int& value){ value += 1 + max_index; });
        _node_copy.unconnect_all_nodes();

        _nodes.push_back(std::move(_node_copy));
    }

    return *this;
}

GraphBuilder &GraphBuilder::add_node(GraphNode _node)
{
    int node_num = _node.get_number();

    if (_temp_nodes.contains(node_num))
    {
        boost::format exception_format_str = boost::format("Node %d already exists!") % node_num;
        throw std::logic_error(exception_format_str.str());
    }

    _temp_nodes.insert(node_num, std::move(_node));

    return *this;
}

GraphBuilder &GraphBuilder::add_node(int node_num, QString name)
{
    if (_temp_nodes.contains(node_num))
    {
        boost::format exception_format_str = boost::format("Node %d already exists!") % node_num;
        throw std::logic_error(exception_format_str.str());
    }

    _temp_nodes.insert(node_num, GraphNode(node_num, name));

    return *this;
}

GraphBuilder &GraphBuilder::connect_node_to_node(unsigned int first_node_num, unsigned int second_node_num)
{
    if (!_temp_nodes.contains(first_node_num) || !_temp_nodes.contains(second_node_num))
    {
        boost::format exception_format_str = boost::format("To node %d can not connect %d!") % first_node_num % second_node_num;
        throw std::logic_error(exception_format_str.str());
    }

    if (_nodes_links.contains(first_node_num))
    {
        _nodes_links[first_node_num].insert(second_node_num);
    }
    else
    {
        _nodes_links[first_node_num] = QSet<unsigned int>{second_node_num};
    }


    return *this;
}

GraphBuilder &GraphBuilder::unconnect_node_from_node(unsigned int first_node_num, unsigned int second_node_num)
{
    if (!_temp_nodes.contains(first_node_num) || !_temp_nodes.contains(second_node_num))
    {
        boost::format exception_format_str = boost::format("To node %d can not connect %d!") % first_node_num % second_node_num;
        throw std::logic_error(exception_format_str.str());
    }

    if (_nodes_links.contains(first_node_num))
    {
        _nodes_links[first_node_num].remove(second_node_num);
    }

    return *this;
}

GraphBuilder &GraphBuilder::remove_node(unsigned int num)
{
    if (_temp_nodes.contains(num))
    {
        _temp_nodes.remove(num);
    }
    else
    {
        boost::format exception_format_str = boost::format("Node %d is not exists") % num;
        throw std::logic_error(exception_format_str.str());
    }

    if (_nodes_links.contains(num))
    {
        _nodes_links.remove(num);
    }

    return *this;
}

GraphBuilder &GraphBuilder::clear()
{
    _temp_nodes.clear();
    _nodes_links.clear();

    return *this;
}

graph_data GraphBuilder::build()
{
    if (_temp_nodes.size() > 0) {
        graph_data _data(_name);

        QList<GraphNode> _nodes = _temp_nodes.values();
        QList<unsigned int> _indexes = _nodes_links.keys();

        for (GraphNode node_key : _nodes)
        {
            int num = node_key.get_number();

            _data.add_vertex(std::move(node_key));
        }

        for (unsigned int& node_num : _indexes) {
            _data.connect_vertexes_to_vertex(node_num, _nodes_links[node_num].values());
        }

        return _data;
    }

    throw std::runtime_error("Graph has no any nodes!");
}

QString GraphBuilder::name() const
{
    return _name;
}

void GraphBuilder::setName(const QString &newName)
{
    _name = newName;
}

graph_data *GraphBuilder::last_created()
{
    return _cache.get();
}

unsigned int GraphBuilder::get_max_index()
{
    QList<unsigned int> builder_nodes = _temp_nodes.keys();

    return  *std::max(builder_nodes.begin(), builder_nodes.end());
}
