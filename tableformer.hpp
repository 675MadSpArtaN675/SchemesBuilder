#ifndef TABLEFORMER_H
#define TABLEFORMER_H

#include <QObject>

#include <DataFrame/DataFrame.h>

#include "abstracttableformer.hpp"

class TableFormer : public AbstractTableFormer
{
    Q_OBJECT

public:
    explicit TableFormer(QObject* parent = nullptr);
    TableFormer(const TableFormer& other);
    virtual ~TableFormer();

    virtual void set_columns_without_names(int count) override;
    virtual void set_index_without_names(int count) override;

    virtual void set_columns_names(std::list<std::string> row_list) override;
    virtual void set_index_names(std::list<std::string> row_list) override;

    virtual void add_row(int row_index, std::list<std::string> row_list) override;
    virtual void add_row(std::string row_index, std::list<std::string> row_list) override;
    virtual void add_autoindexed_row(std::list<std::string> row_list) override;

    virtual void remove_row(int row_num) override;
    virtual void remove_row(std::string name) override;
    virtual void remove_column(int column_num) override;
    virtual void remove_column(std::string name) override;

    virtual std::unique_ptr<AbstractTableFormer> clone() override;

    TableFormer& operator=(const TableFormer& other);
};

#endif // TABLEFORMER_H
