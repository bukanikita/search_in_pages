#include "resultswidget.h"
#include "searchresult.h"
#include <QHeaderView>
#include <QDebug>

ResultsWidget::ResultsWidget(QWidget *parent) : QTableWidget(parent)
{
    QStringList labels;
    labels << tr("URL") << tr("Status");
    setColumnCount(labels.size());
    setHorizontalHeaderLabels(labels);
    horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ResultsWidget::addItem(const QString &url)
{
    int numberOfElements = rowCount();
    insertRow(numberOfElements);
    setItem(numberOfElements, 0, new QTableWidgetItem(url));
    setItem(numberOfElements, 1, new QTableWidgetItem(statusToString(SearchResult::Status::InProgress, 0)));
}

void ResultsWidget::updateItem(const QString &resultUrl, SearchResult::Status resultStatus, int resultRetCode)
{
    const QList<QTableWidgetItem *> items = findItems(resultUrl, Qt::MatchExactly);
    for (const auto urlItem : items) // really always only one item
    {
        int row = urlItem->row();
        if (QTableWidgetItem *statusItem = item(row, 1))
        {
            statusItem->setText(statusToString(resultStatus, resultRetCode));
        }
    }
    scrollToBottom();
    emit itemUpdated(rowCount());
}

void ResultsWidget::clearResults()
{
    setRowCount(0);
}
