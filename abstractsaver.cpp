#include "abstractsaver.hpp"


AbstractSaver::AbstractSaver(QObject *parent)
    : QObject{parent}, CoreLogger()
{
    _elements = std::make_unique<QMap<QString, QString>>(QMap<QString, QString>({ std::pair<QString, QString>("base", QString())}));
}

void AbstractSaver::load_options(QString filename)
{
    filename = filename.nullTerminate().trimmed();
    log((boost::format("Loading options of saver from file \"%s\"") % filename.toStdString()).str());

    if (filename.isEmpty() || !QFile::exists(filename) || QFile::permissions(filename) == QFileDevice::ReadUser)
    {
        return;
    }

    try {
		QFile _options_file(filename);

		if (_options_file.open(QFileDevice::ReadOnly))
		{
            log_debug("Loading options from file...");
			options_loading_logic(_options_file);
		}

		_options_file.close();
    }
    catch (const QException& error)
    {
		log_error(std::string("Error of interaction with options file: ") + error.what());
    }
}

void AbstractSaver::save(QString filename)
{
    filename = filename.nullTerminate().trimmed();
    log((boost::format("Saving graph to file: \"%s\"") % filename.toStdString()).str());

    if (filename.isEmpty())
    {
        return;
    }

    try {
		QFile _options_file(filename);

		if (_options_file.open(QFileDevice::WriteOnly | QFileDevice::Append | QFileDevice::Truncate))
		{
            log_debug("Saving to file...");
			save_logic(_options_file);
		}

		_options_file.close();
    }
    catch (const QException& error)
    {
		log_error(std::string("Error of saving graph in file: ") + error.what());
    }
}

void AbstractSaver::set_option(QString _key, QString _value)
{
    QString _key_clear = _key.nullTerminate().trimmed();
    QString _value_clear = _value.nullTerminate().trimmed();

    if (_key_clear.isEmpty() || _value_clear.isEmpty())
    {
        return;
    }

	(*_elements)[_key_clear] = _value_clear;
    recalculate_results();
}

QVariant AbstractSaver::get_option_value(QString _key)
{
    _key = _key.nullTerminate().trimmed();

    if (_key.isEmpty() || !_elements->contains(_key))
    {
		return QVariant();
    }

    recalculate_results();
    return (*_elements)[_key];
}

bool AbstractSaver::is_base_element_load()
{
	return (*_elements)["base"].nullTerminate().trimmed().isEmpty();
}

void AbstractSaver::clear()
{
	_elements->clear();
    _elements->insert("base", QString());
}

QString AbstractSaver::options_file()
{
	return _options_file_name;
}

void AbstractSaver::set_option_file(QString filename)
{
	_options_file_name = filename;
}

QBindable<QString> AbstractSaver::bindable_options_filename()
{
	return QBindable<QString>(&_options_file_name);
}

void AbstractSaver::set_graph(graph_data *_graph_data)
{
	_graph = _graph_data;
}

graph_data *AbstractSaver::graph()
{
	return _graph;
}

QBindable<graph_data*> AbstractSaver::bindable_graph()
{
	return QBindable<graph_data*>(&_graph);
}

void AbstractSaver::options_loading_logic(QFile &file)
{
    QRegularExpression title_expression("(?<option_name>[\\d\\s\\w]*):(?<option_result>.*)\\s?");
    QString previous_name;
	while (!file.atEnd())
    {
		QString line = file.readLine().nullTerminate().trimmed();

        if (line.isEmpty())
        {
            continue;
        }

		QRegularExpressionMatch _match = title_expression.match(line);
        if (_match.hasMatch() && _match.hasCaptured("option_name"))
        {
            QString name = _match.captured("option_name").nullTerminate().trimmed();

            if (!name.isEmpty() && !_elements->contains(name)) {
				QString result = _match.captured("option_result").nullTerminate().trimmed();

				set_option(name, result);

				previous_name = name;
            }
            else if (!name.isEmpty() && _elements->contains(name))
            {
                add_result_to_option(name, _match.captured("option_result"));
            }
            else
            {
                add_result_to_option(previous_name, _match.captured("option_result"));
			}
        }
        else
		{
			add_result_to_option(previous_name, line);
		}
    }
}

void AbstractSaver::recalculate_results()
{
	for (const QString& key_of_element : _elements->keys())
    {
        QString option = (*_elements)[key_of_element].nullTerminate().trimmed();

        (*_elements)[key_of_element] = option;
    }
}

void AbstractSaver::add_result_to_option(const QString& _key, const QString& _result)
{
    std::string _trimmed_result;
	boost::algorithm::trim_copy_if(
				std::back_inserter(_trimmed_result),
				_result,
				boost::is_any_of(": \n\t")
    );

	QString result = get_option_value(_key).value<QString>()
			+ QString::fromStdString(_trimmed_result).trimmed();

	set_option(_key, result);
}
