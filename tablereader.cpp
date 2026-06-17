#include "tablereader.hpp"

#include "tableformer.hpp"
#include "tableformerdatabasedtype.hpp"

#include "DataFrame/Utils/Matrix.h"

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

TableReader::TableReader(QObject *parent)
    : QObject{parent}, CoreLogger()
{ }

TableReader::~TableReader()
{ }

void TableReader::parse_file(QString file_path)
{
    processor_init();

    try {
		std::string filename = file_path.toStdString();
		_table_preprocessor->open(filename);

		if (_table_preprocessor->validate_file())
		{
			log("Парсим файл: " + filename);
			parse_file_cycle(file_path);
		}
		else
		{
			_table_preprocessor->close();
		}
    }
    catch (const std::invalid_argument& error)
    {
        log_error(std::string("Argument error: ") + error.what());
    }
    catch (const std::exception& error)
    {
		log_error(std::string("Error of file parsing: ") + error.what());
    }
}

void TableReader::parse_file_cycle(const QString& file_path)
{
    _table_former->clear();

	bool is_first_line = true;
	while (!_table_preprocessor->is_eof())
	{
		std::list<std::string> _line = _table_preprocessor->read_line();
		log("Длина считанной линии: " + std::to_string(_line.size()));

		if (_line.size() > 1)
		{
			if (is_first_line && !_is_headers_blocked) {
				log_debug("Setting header...");
				if (_is_header_need)
				{
					_table_former->set_columns_names(_line);
				}
				else
				{
					_table_former->set_columns_without_names(_line.size());
				}

				is_first_line = false;
			}

			if (_is_index_need && !_is_index_blocked) {
				_table_former->add_autoindexed_row(_line);
			}
			else
			{
				int last_new_row_index = _table_former->get_new_row_num();
                log("New row in index: " + std::to_string(last_new_row_index));

				_table_former->add_row(std::to_string(last_new_row_index), _line);
			}
		}
		else
		{
			std::string row_value = _line.front();
			log("Значение: " + row_value);

			boost::algorithm::trim(row_value);

			if (row_value == "end" || row_value == "e")
			{
				_table_preprocessor->close();
				break;
			}
		}
	}

	_last_file = file_path;
}

void TableReader::parse_file(QUrl file_path)
{
    QString file_path_str = file_path.toString();
    file_path_str = file_path_str.replace(QRegularExpression("^(file|qrc)://"), QString(""));

    parse_file(file_path_str);
}

void TableReader::clear()
{
	_table_former->clear();
    _table_preprocessor.reset();
}

void TableReader::switch_former(FormerEnum number)
{
    log("Switching a former. Input num: " + std::to_string((int) number));
	log("Is block: " + std::to_string(number >= FormerEnum::DatabasedType));

    if (number >= FormerEnum::DatabasedType) {
		_is_headers_blocked = true;
        _is_index_blocked = true;
    }
    else {
		_is_headers_blocked = false;
        _is_index_blocked = false;
    }

	switch (number)
    {
	case FormerEnum::Standart:
		_table_former.reset(new TableFormer(parent()));
		break;
    case FormerEnum::DatabasedType:
        _table_former.reset(new TableFormerDatabasedType(parent()));
		break;

    default:
        _table_former.reset();
        break;
    }
}

QString TableReader::get_description_of_node(int node_num)
{
    if (_table_former) {
		std::optional<std::string> node_descr = _table_former->get_description_by_node_num(node_num);

		return QString::fromStdString(node_descr.value_or(""));
    }

    return QString();
}

void TableReader::processor_init()
{
    std::string _delim = _delimiter.toStdString();
    if (!_table_preprocessor
            || _table_preprocessor->delimiter() != _delim
            || _table_preprocessor->is_header_has() != _is_header_need
            || _table_preprocessor->is_index_has() != _is_index_need)
    {
        _table_preprocessor.reset(new TablePreprocessor(_is_header_need, _is_index_need, _delim));
    }

    if (_table_former)
	{
    	_table_former->clear();
    }

    log((boost::format("Is header: %d, Is index: %d") % _is_header_need % _is_index_need).str());
}

QList<QList<int>> TableReader::get_table()
{
    log("Getting table...");
    if (_table_former) {
        try {
			hmdf::Matrix<int> _table = _table_former->to_matrix();
			QList<QList<int>> _result_table_to_qml;

			log_debug((boost::format("Matrix size %dx%d;") % _table.rows() % _table.cols()).str());
			for (int i = 0; i < _table.rows(); i++)
			{
				_result_table_to_qml.emplaceBack();

				for (int j = 0; j < _table.cols(); j++)
				{
					int value = _table(i, j);
					log_debug((boost::format("Insert element (%d; %d) = %d;") % i % j % value).str());

					_result_table_to_qml[i].push_back(value);
				}
			}

			_table_bind.setValue(QList<QList<int>>(_result_table_to_qml));
			return _result_table_to_qml;
        }
        catch (const std::exception& error)
        {
            log_error(std::string("Error of creating table: ") + error.what());
        }
    }

    log_error("Table is empty!");
    return QList<QList<int>>();
}

QBindable<QList<QList<int>> > TableReader::bindable_table() {
    return QBindable<QList<QList<int>>>(&_table_bind);
}

QList<QString> TableReader::get_titles()
{
    if (_table_former) {
        QList<QString> _result_titles;

        std::vector<std::string> _titles_raw = _table_former->get_row_indexes();
        std::transform(_titles_raw.cbegin(), _titles_raw.cend(), std::back_inserter(_result_titles), [](std::string title){ return QString::fromStdString(title); });

        _titles.setValue(QList<QString>(_result_titles));

        return _result_titles;
    }

    return QList<QString>();
}

QBindable<QList<QString> > TableReader::bindable_titles() {
    return QBindable<QList<QString>>(&_titles);
}

void TableReader::set_delimiter(QString delimiter)
{
    _delimiter = delimiter;
}

QString TableReader::get_delimiter()
{
    return _delimiter;
}

void TableReader::set_header_need(bool value)
{
    _is_header_need = value;
}

bool TableReader::is_header_need() {
    return _is_header_need;
}

void TableReader::set_index_need(bool value) {
    _is_index_need = value;
}

bool TableReader::is_index_need() {
    return _is_index_need;
}

QObject* TableReader::table_former() const
{
    return _table_former.get();
}

void TableReader::set_table_former(QObject* new_table_former)
{
    try {
		if (new_table_former != nullptr) {
            AbstractTableFormer* former = dynamic_cast<AbstractTableFormer*>(new_table_former);

			_table_former.reset(former->clone().release());
        }
	}
    catch (const std::bad_cast& _error)
	{
		log(std::string("Error of cast object of new former: ") + _error.what());
	}
}
