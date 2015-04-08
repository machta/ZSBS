#include "eventtypemanagerdelegate.h"

#include <QColorDialog>
#include <QLineEdit>
#include <QAction>

EventTypeManagerDelegate::EventTypeManagerDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

EventTypeManagerDelegate::~EventTypeManagerDelegate()
{
}

QWidget* EventTypeManagerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	switch (index.column())
	{
	case 3:
	{
		QLineEdit* lineEdit = new QLineEdit(parent);

		QAction* action = lineEdit->addAction(QIcon("edit.png"), QLineEdit::TrailingPosition);

		connect(action, &QAction::triggered, [this, lineEdit] ()
		{
			QColor color;
			color.setNamedColor(lineEdit->text());

			color = QColorDialog::getColor(color, lineEdit); // There is a bug in Qt implementation that selects a bad initial color. (https://bugreports.qt.io/browse/QTBUG-44154)

			if (color.isValid())
			{
				lineEdit->setText(color.name());

				emit const_cast<EventTypeManagerDelegate*>(this)->commitData(lineEdit);
				emit const_cast<EventTypeManagerDelegate*>(this)->closeEditor(lineEdit);
			}
		});

		return lineEdit;
	}
	}

	return QStyledItemDelegate::createEditor(parent, option, index);
}

void EventTypeManagerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	switch (index.column())
	{
	case 3:
	{
		QLineEdit* lineEdit = reinterpret_cast<QLineEdit*>(editor);
		lineEdit->setText(index.model()->data(index).value<QColor>().name());
		return;
	}
	}

	QStyledItemDelegate::setEditorData(editor, index);
}

void EventTypeManagerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	switch (index.column())
	{
	case 3:
	{
		QLineEdit* lineEdit = reinterpret_cast<QLineEdit*>(editor);
		model->setData(index, QVariant::fromValue(QColor(lineEdit->text())));
		return;
	}
	}

	QStyledItemDelegate::setModelData(editor, model, index);
}

void EventTypeManagerDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);
}