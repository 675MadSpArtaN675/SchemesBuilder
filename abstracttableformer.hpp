#ifndef ABSTRACTTABLEFORMER_HPP
#define ABSTRACTTABLEFORMER_HPP

#include <QObject>
#include <DataFrame/DataFrame.h>

#include "CoreLogger.hpp"

#define RESTRICT_FIRST_COLUMNS

class AbstractTableFormer : public QObject, protected CoreLogger
{
    Q_OBJECT

public:
    AbstractTableFormer(QObject* parent = nullptr);
    AbstractTableFormer(const AbstractTableFormer& other);
    virtual ~AbstractTableFormer();

    virtual void set_columns_without_names(int count) = 0;
    virtual void set_index_without_names(int count) = 0;

    virtual void set_columns_names(std::list<std::string> row_list) = 0;
    virtual void set_index_names(std::list<std::string> row_list) = 0;

    virtual void add_row(int row_index, std::list<std::string> row_list) = 0;
    virtual void add_row(std::string row_index, std::list<std::string> row_list) = 0;
    virtual void add_autoindexed_row(std::list<std::string> row_list) = 0;

    virtual void remove_row(int row_num) = 0;
    virtual void remove_row(std::string name) = 0;
    virtual void remove_column(int column_num) = 0;
    virtual void remove_column(std::string name) = 0;

    virtual void clear();
    virtual std::unique_ptr<AbstractTableFormer> clone() = 0;
    virtual hmdf::Matrix<int> to_matrix();

    std::vector<std::string> get_columns_names();
    std::vector<std::string> get_row_indexes();

    int get_last_row_num();
    int get_new_row_num();

protected:
    int get_index_num(std::string index_str);

    bool _is_has_index, _is_has_columns;

    using index_type = std::string;
    hmdf::StdDataFrame<index_type> _creating_table;
};

#endif // ABSTRACTTABLEFORMER_HPP
