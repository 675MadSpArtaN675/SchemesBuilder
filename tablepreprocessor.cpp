#include "tablepreprocessor.hpp"

#include <iostream>
#include <algorithm>
#include <exception>
#include <filesystem>

#include <boost/algorithm/string.hpp>

TablePreprocessor::TablePreprocessor() : TablePreprocessor(true, true, STANDART_DELIMITER)
{ }

TablePreprocessor::TablePreprocessor(std::string _delimiter) : TablePreprocessor(true, true, _delimiter)
{ }

TablePreprocessor::TablePreprocessor(bool is_has_header, bool is_has_index) : TablePreprocessor(is_has_header, is_has_index, STANDART_DELIMITER)
{ }

TablePreprocessor::TablePreprocessor(bool is_has_header, bool is_has_index, std::string delimiter)
{
    _is_has_header = is_has_header;
    _is_has_index = is_has_index;
    _delimiter = delimiter;
}

TablePreprocessor::~TablePreprocessor()
{
    close();
}

void TablePreprocessor::open(std::string path)
{
    try {
        _last_opened_file_path = path;
        _opened_file = std::ifstream(path);
    }
    catch (std::exception ex)
    {
        std::cout << ex.what() << std::endl;
    }
}

bool TablePreprocessor::is_file_exists()
{
    return std::filesystem::exists(_last_opened_file_path);
}

bool TablePreprocessor::is_header_has()
{
    return _is_has_header;
}

bool TablePreprocessor::is_index_has()
{
    return _is_has_index;
}

bool TablePreprocessor::validate_file()
{
    if (!is_file_exists())
    {
        return false;
    }

    calculate_lines();

    return _opened_file.is_open() && !_lines_and_their_pos.empty();
}

bool TablePreprocessor::is_eof()
{
    return _opened_file.eof();
}

void TablePreprocessor::set_header_row(bool is_has_header)
{
    _is_has_header = is_has_header;
}

void TablePreprocessor::set_is_index(bool is_has_index)
{
    _is_has_index = is_has_index;
}

std::list<std::string> TablePreprocessor::read_line()
{
    if (_opened_file.is_open() && !_opened_file.eof())
    {
        std::string _line;
        _line.resize(READ_BUFFER_SIZE);

        do {
            _opened_file.getline(_line.data(), READ_BUFFER_SIZE);
            boost::algorithm::trim(_line);
        }
        while(!_opened_file.eof() && _line.empty());

        if (!_opened_file.eof())
        {
            _opened_file.seekg(0, std::ios_base::beg);
        }

        if (!_line.empty()) {
            return split_line_by_delimiter(_line, _delimiter);
        }
    }

    throw std::logic_error("Empty Line!");
}

std::list<std::string> TablePreprocessor::read_line_num(int number)
{
    if (_lines_and_their_pos.empty())
    {
        calculate_lines();
    }

    if (_opened_file.is_open() && _lines_and_their_pos.contains(number))
    {
        std::string _line;
        _line.resize(READ_BUFFER_SIZE);

        int pos = _lines_and_their_pos[number];

        _opened_file.seekg(pos);
        _opened_file.getline(_line.data(), READ_BUFFER_SIZE);

        if (!_line.empty()) {
            return split_line_by_delimiter(_line, _delimiter);
        }
    }

    throw std::logic_error("Empty Line!");
}

std::list<std::string> TablePreprocessor::split_line_by_delimiter(std::string line, std::string delimiter)
{
    std::list<std::string> _splitted_rows;

    boost::algorithm::split(_splitted_rows, line, boost::algorithm::is_any_of(_delimiter), boost::algorithm::token_compress_on);
    std::for_each(_splitted_rows.begin(), _splitted_rows.end(), [](std::string& text){
       boost::algorithm::trim(text);
    });

    std::erase_if(_splitted_rows, [](std::string& text) { return text.empty(); });

    return _splitted_rows;
}

void TablePreprocessor::calculate_lines()
{
    if (_opened_file.is_open())
    {
        _lines_and_their_pos.clear();

        _opened_file.seekg(0, std::ios_base::beg);
        _lines_and_their_pos[1] = 0;

        int line_num = 2;
        while(_opened_file.eof())
        {
            std::string _line;
            _line.resize(READ_BUFFER_SIZE);

            _opened_file.getline(_line.data(), READ_BUFFER_SIZE);
            boost::algorithm::trim(_line);

            if (!_line.empty()) {
                _lines_and_their_pos[line_num++] = _opened_file.tellg();
            }

        }

        _line_count = line_num;
    }
}

int TablePreprocessor::line_count() { return _line_count; }

void TablePreprocessor::close()
{
    if (_opened_file.is_open()) {
        _opened_file.close();
    }
}