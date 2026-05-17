#include "graphnodeonlyread.hpp"

GraphNodeOnlyRead::GraphNodeOnlyRead() : number{0}, name{""}, connected_nodes(), additional_data()
{ }

GraphNodeOnlyRead::GraphNodeOnlyRead(unsigned int _number, QString _name, QVariant _additional_data, QList<unsigned int> _connected_nodes) : number{_number}, name{_name}, connected_nodes(_connected_nodes), additional_data(_additional_data)
  
{ }

GraphNodeOnlyRead::GraphNodeOnlyRead(GraphNodeOnlyRead &other) : GraphNodeOnlyRead(other.number, other.name, other.additional_data, other.connected_nodes)
{ }
