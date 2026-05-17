#ifndef TABLEFORMER_H
#define TABLEFORMER_H

#include <QObject>

#include <DataFrame/DataFrame.h>

#include "CoreLogger.hpp"

class TableFormer : public QObject, protected CoreLogger
{
    Q_OBJECT

public:
    TableFormer();
    TableFormer(const TableFormer& other);
    ~TableFormer();

    virtual void set_columns_without_names(int count);
    virtual void set_index_without_names(int count);

    virtual void set_columns_names(std::list<std::string> row_list);
    virtual void set_index_names(std::list<std::string> row_list);

    virtual void add_row(int row_index, std::list<std::string> row_list);
    void add_row(std::string row_index, std::list<std::string> row_list);
    void add_autoindexed_row(std::list<std::string> row_list);
    virtual void remove_row(int row_num);
    virtual void remove_column(int column_num);
    virtual void remove_row(std::string name);
    virtual void remove_column(std::string name);

    std::vector<std::string> get_columns_names();
    std::vector<std::string> get_row_indexes();

    int get_last_row_num();
    int get_new_row_num();

    void clear();
    hmdf::Matrix<int> to_matrix();

    TableFormer& operator=(const TableFormer& other);

protected:
    int get_index_num(std::string index_str);

    using index_type = std::string;

    hmdf::StdDataFrame<index_type> _creating_table;

    bool _is_has_index, _is_has_columns;
    int rows_count;
};

#endif // TABLEFORMER_H
