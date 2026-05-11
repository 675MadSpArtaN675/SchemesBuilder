#ifndef TABLEPREPROCESSOR_H
#define TABLEPREPROCESSOR_H

#define READ_BUFFER_SIZE 1024
#define STANDART_DELIMITER "|"

#include <string>
#include <map>
#include <list>
#include <fstream>

class TablePreprocessor
{
public:
    TablePreprocessor();
    TablePreprocessor(std::string _delimiter);
    TablePreprocessor(bool is_has_header, bool is_has_index);
    TablePreprocessor(bool is_has_header, bool is_has_index, std::string _delimiter);
    ~TablePreprocessor();

    void open(std::string path);

    bool is_file_exists();
    bool is_header_has();
    bool is_index_has();

    bool validate_file();
    bool is_eof();

    void set_header_row(bool is_has_header);
    void set_is_index(bool is_has_index);

    std::list<std::string> read_line();
    std::list<std::string> read_line_num(int number);

    int line_count();

    void close();

protected:
    std::list<std::string> split_line_by_delimiter(std::string line, std::string delimiter);
    void calculate_lines();

    int _line_count;
    bool _is_has_header, _is_has_index;

    std::string _delimiter;
    std::string _last_opened_file_path;
    std::ifstream _opened_file;

    std::map<int, int> _lines_and_their_pos;
};

#endif // TABLEPREPROCESSOR_H
