#ifndef TABLEFORMER_H
#define TABLEFORMER_H

#include <variant>
#include "DataFrame/DataFrame.h"


class TableFormer
{
public:
    TableFormer();
    ~TableFormer();

    void set_columns_without_names(int count);
    void set_columns_names(std::list<std::string> row_list);
    void set_index_names(std::list<std::string> row_list);

    void add_row(std::list<std::string> row_list);

    std::vector<std::string> get_columns_names();
    std::vector<std::string> get_row_indexes();

    hmdf::Matrix<int> to_matrix();

protected:
    using index_type = std::string;

    hmdf::StdDataFrame<index_type> _creating_table;

    int _start_row_num_index = 0;
    bool _is_has_index, _is_has_columns;
};

#endif // TABLEFORMER_H
