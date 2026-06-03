#include <boost/test/included/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/random.hpp>

#include <QString>

#include "graphnode.hpp"
#include "graph_data.hpp"
#include "CoreLogger.hpp"

namespace uTest = boost::unit_test;

class GraphTest
{
    graph_data test_graph_1;
    graph_data test_graph_2;

public:
    GraphTest() : test_graph_1(QString("Graph_1")), test_graph_2(QString("Graph_2_Test"))
    {
		test_graph_2.add_vertex(5, "Eight");
        test_graph_2.add_vertex(6, "Six", QList<unsigned int>{ 5 , 1 });
    }

    void initialization_test() {
        graph_data _init(QString("Graph_3"));

        bool graph_comparsion = test_graph_1 == _init;
        BOOST_LOG_TRIVIAL(info) << "comparsion result: " << std::to_string(graph_comparsion);
    }

    void adding_test() {
		graph_data _adding_graph(test_graph_2);
        unsigned int node_index = 1;
        QString name = "Mad Node";

        _adding_graph.add_vertex(node_index, name, QList<unsigned int>{ 5, 4, 6 });

        GraphNode node = *_adding_graph.get_vertex(node_index);

        bool _node_comparsion = node.get_number() == node_index && node.get_name() == name;
        BOOST_TEST(_node_comparsion);
        BOOST_TEST(node.get_connected_nodes_nums().size() == 2);

        _adding_graph.add_vertex(node_index + 1, name + " 1");

        GraphNode node_2 = *_adding_graph.get_vertex(node_index + 1);

        bool node_comparsion_2 = node_2.get_number() == node_index + 1 && node_2.get_name() == name + " 1";
        BOOST_TEST_REQUIRE(node_comparsion_2);
        BOOST_TEST_REQUIRE(node.get_connected_nodes_nums().size() == 0);

        test_graph_1.add_vertex(std::move(node_2));
    }

    void removing_test(){
		graph_data _remove_test_graph(test_graph_1);

        _remove_test_graph.remove_vertex(2);

        BOOST_TEST_REQUIRE(!_remove_test_graph.get_vertex(2));
    }

    void vertex_connection_test() {
		graph_data _connect_vertexes(test_graph_2);

        _connect_vertexes.add_vertex(19, "Connect node");
        _connect_vertexes.add_vertex(29, "Connect node 2");

        _connect_vertexes.connect_vertex_to_vertex(19, 29);

        BOOST_TEST_REQUIRE(_connect_vertexes.get_vertex(19)->get_connected_nodes_nums()[0] == 758);
    }

    void transforming_test() {
		graph_data _transform("Pond");

        _transform.add_vertex(1, "Lodik");
        _transform.add_vertex(2, "Artyom");
        _transform.add_vertex(3, "Ivan");
        _transform.add_vertex(4, "Alex");

        QList<QList<int>> table;
        boost::random::mt19937 _engine;
        boost::random::uniform_smallint<unsigned short> generator(0, 1);

        QList<int> elements_connects_count;
        for (int i = 0; i < 4; i++)
        {
            QList<int> _im = QList<int>{generator(_engine), generator(_engine), generator(_engine), generator(_engine)};

            table.append(_im);
            elements_connects_count.count(1);
        }

        _transform.transform_links(table);

        BOOST_TEST_REQUIRE(_transform.size() == 4);

        for (int i = 0; i < _transform.size(); i++) {
     	   BOOST_TEST_REQUIRE(_transform.get_vertex(i + 1)->get_number() == i + 1);
        }
    }
};


boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
    logger_config("test_log");
    boost::shared_ptr<GraphTest> _test = boost::make_shared<GraphTest>();


    boost::unit_test::framework::master_test_suite().
            add(BOOST_TEST_CASE_NAME(boost::bind(&GraphTest::initialization_test, _test), "Init test"));

	// boost::unit_test::framework::master_test_suite().
 //            add(BOOST_TEST_CASE_NAME(boost::bind(&GraphTest::adding_test, _test), "Vertex adding test"));

 //    boost::unit_test::framework::master_test_suite().
 //            add(BOOST_TEST_CASE_NAME(boost::bind(&GraphTest::removing_test, _test), "Removing vertex test"));

 //    boost::unit_test::framework::master_test_suite().
 //            add(BOOST_TEST_CASE_NAME(boost::bind(&GraphTest::vertex_connection_test, _test), "Vertex connection test"));

 //    boost::unit_test::framework::master_test_suite().
 //            add(BOOST_TEST_CASE_NAME(boost::bind(&GraphTest::transforming_test, _test), "Transforming test"));

    clean_log();
    std::cout << "Tests ended!" << std::endl;
    return 0;
}