#ifndef TABLEFORMERDATABASEDTYPE_HPP
#define TABLEFORMERDATABASEDTYPE_HPP

#include <QObject>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/algorithm/algorithm.hpp>
#include <boost/algorithm/string.hpp>

#include "tableformer.hpp"

class TableFormerDatabasedType : public TableFormer
{
    Q_OBJECT

public:
    TableFormerDatabasedType(QObject* parent = nullptr);
    TableFormerDatabasedType(const TableFormerDatabasedType& other);
    virtual ~TableFormerDatabasedType();

    virtual void add_row(int row_index, std::list<std::string> row_list) override;
    virtual hmdf::Matrix<int> to_matrix() override;
    virtual std::unique_ptr<AbstractTableFormer> clone() override;

    TableFormerDatabasedType& operator=(const TableFormerDatabasedType& other);

protected:
    void fill_table();

    std::map<int, std::string> _descriptions;
    std::map<int, std::list<int>> _links;
};

#endif // TABLEFORMERDATABASEDTYPE_HPP
