#include "eventtable.h"

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include <algorithm>
#include <functional>
#include <QCollator>

using namespace std;

EventTable::EventTable(QObject* parent) : QAbstractTableModel(parent)
{
}

EventTable::~EventTable()
{
}

void EventTable::write(QXmlStreamWriter* xml) const
{
	xml->writeStartElement("eventTable");

	assert(static_cast<size_t>(rowCount()) == label.size());
	assert(static_cast<size_t>(rowCount()) == type.size());
	assert(static_cast<size_t>(rowCount()) == position.size());
	assert(static_cast<size_t>(rowCount()) == duration.size());
	assert(static_cast<size_t>(rowCount()) == channel.size());
	assert(static_cast<size_t>(rowCount()) == description.size());

	for (unsigned int i = 0; i < label.size(); ++i)
	{
		xml->writeStartElement("event");

		xml->writeAttribute("label", QString::fromStdString(label[i]));
		xml->writeAttribute("type", QString::number(type[i]));
		xml->writeAttribute("position", QString::number(position[i]));
		xml->writeAttribute("duration", QString::number(duration[i]));
		xml->writeAttribute("channel", QString::number(channel[i]));

		if (description[i].empty() == false)
		{
			xml->writeTextElement("description", QString::fromStdString(description[i]));
		}

		xml->writeEndElement();
	}

	xml->writeEndElement();
}

#define readNumericAttribute(a_, b_)\
	{\
		bool ok;\
		auto tmp = xml->attributes().value(#a_).b_(&ok);\
		(a_).push_back(ok ? tmp : 0);\
	}

void EventTable::read(QXmlStreamReader* xml)
{
	assert(xml->name() == "eventTable");

	while (xml->readNextStartElement())
	{
		if (xml->name() == "event")
		{
			order.push_back(order.size());

			label.push_back(xml->attributes().value("label").toString().toStdString());
			readNumericAttribute(type, toInt);
			readNumericAttribute(position, toInt);
			readNumericAttribute(duration, toInt);
			readNumericAttribute(channel, toInt);

			description.push_back("");
			while (xml->readNextStartElement())
			{
				if (xml->name() == "description")
				{
					description.back() = xml->readElementText().toStdString();
				}
				else
				{
					xml->skipCurrentElement();
				}
			}
		}
		else
		{
			xml->skipCurrentElement();
		}
	}
}

#undef readIntAttribute

void EventTable::getEventsForRendering(int firstSample, int lastSample, vector<tuple<int, int, int>>* allChannelEvents, vector<tuple<int, int, int, int>>* singleChannelEvents)
{
	for (int i = 0; i < rowCount(); ++i)
	{
		if (position[i] <= lastSample && firstSample <= position[i] + duration[i] - 1)
		{
			if (channel[i] < 0)
			{
				allChannelEvents->emplace_back(type[i], position[i], duration[i]);
			}
			else
			{
				singleChannelEvents->emplace_back(type[i], channel[i], position[i], duration[i]);
			}
		}
	}

	std::sort(allChannelEvents->begin(), allChannelEvents->end(), [] (tuple<int, int, int> a, tuple<int, int, int> b) { return get<0>(a) < get<0>(b); });

	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<1>(a) < get<1>(b); });
	stable_sort(singleChannelEvents->begin(), singleChannelEvents->end(), [] (tuple<int, int, int, int> a, tuple<int, int, int, int> b) { return get<0>(a) < get<0>(b); });
}

QVariant EventTable::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		switch (section)
		{
		case 0:
			return "Label";
		case 1:
			return "Type";
		case 2:
			return "Position";
		case 3:
			return "Duration";
		case 4:
			return "Channel";
		case 5:
			return "Description";
		}
	}

	return QVariant();
}

QVariant EventTable::data(const QModelIndex& index, int role) const
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::DisplayRole || role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				return QString::fromStdString(label[row]);
			case 1:
				return type[row];
			case 2:
				return position[row];
			case 3:
				return duration[row];
			case 4:
				return channel[row];
			case 5:
				return QString::fromStdString(description[row]);
			}
		}
	}

	return QVariant();
}

bool EventTable::setData(const QModelIndex& index, const QVariant &value, int role)
{
	if (index.isValid() && index.row() < rowCount() && index.column() < columnCount())
	{
		int row = order[index.row()];

		if (role == Qt::EditRole)
		{
			switch (index.column())
			{
			case 0:
				label[row] = value.toString().toStdString();
				break;
			case 1:
				type[row] = value.value<decltype(type)::value_type>();
				break;
			case 2:
				position[row] = value.value<decltype(position)::value_type>();
				break;
			case 3:
				duration[row] = value.value<decltype(duration)::value_type>();
				break;
			case 4:
				channel[row] = value.value<decltype(channel)::value_type>();
				break;
			case 5:
				description[row] = value.toString().toStdString();
				break;
			}

			emit dataChanged(index, index);
			return true;
		}
	}

	return false;
}

bool EventTable::insertRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginInsertRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			std::stringstream ss;
			ss << "Event " << row + i;

			label.push_back(ss.str());
			type.push_back(0); // TODO: load current selected type
			position.push_back(0);
			duration.push_back(1);
			channel.push_back(-1);
			description.push_back("");

			order.insert(order.begin() + row + i, order.size());
		}

		endInsertRows();

		return true;
	}
	else
	{
		return false;
	}
}

bool EventTable::removeRows(int row, int count, const QModelIndex& /*parent*/)
{
	if (count > 0)
	{
		beginRemoveRows(QModelIndex(), row, row + count - 1);

		for (int i = 0; i < count; ++i)
		{
			int index = order[row + i];

			label.erase(label.begin() + index);
			type.erase(type.begin() + index);
			position.erase(position.begin() + index);
			duration.erase(duration.begin() + index);
			channel.erase(channel.begin() + index);
			description.erase(description.begin() + index);

			order.erase(order.begin() + row + i);

			for (auto& e : order)
			{
				if (e > index)
				{
					--e;
				}
			}
		}

		endRemoveRows();

		return true;
	}
	else
	{
		return false;
	}
}

void EventTable::sort(int column, Qt::SortOrder order)
{
	assert(0 <= column && column < columnCount());

	QCollator collator;
	collator.setNumericMode(true);

	function<bool (int, int)> predicate;

	if (order == Qt::AscendingOrder)
	{
		switch (column)
		{
		case 0:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) < 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return type[a] < type[b]; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return position[a] < position[b]; };
			break;
		case 3:
			predicate = [this] (int a, int b) { return duration[a] < duration[b]; };
			break;
		case 4:
			predicate = [this] (int a, int b) { return channel[a] < channel[b]; };
			break;
		case 5:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(description[a]), QString::fromStdString(description[b])) < 0; };
			break;
		default:
			return;
		}
	}
	else
	{
		switch (column)
		{
		case 0:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(label[a]), QString::fromStdString(label[b])) > 0; };
			break;
		case 1:
			predicate = [this] (int a, int b) { return type[a] > type[b]; };
			break;
		case 2:
			predicate = [this] (int a, int b) { return position[a] > position[b]; };
			break;
		case 3:
			predicate = [this] (int a, int b) { return duration[a] > duration[b]; };
			break;
		case 4:
			predicate = [this] (int a, int b) { return channel[a] > channel[b]; };
			break;
		case 5:
			predicate = [this, &collator] (int a, int b) { return collator.compare(QString::fromStdString(description[a]), QString::fromStdString(description[b])) > 0; };
			break;
		default:
			return;
		}
	}

	std::sort(this->order.begin(), this->order.end(), predicate);

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}
