#include "drawiosaver.hpp"


DrawioSaver::DrawioSaver(QObject* parent) : AbstractSaver(parent)
{ }

void DrawioSaver::save_logic(QFile &file)
{
    if (!is_base_element_load())
    {
        throw std::logic_error("Base element is not ready!");
    }

    QString _base = (*_elements)["base"];

    for (const unsigned int& _node : _graph->get_nodes_numbers())
    {

    }
}