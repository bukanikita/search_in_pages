#ifndef RESULTSWIDGET_H
#define RESULTSWIDGET_H

#include <QTableWidget>
#include "searchresult.h"

class ResultsWidget : public QTableWidget
{
    Q_OBJECT

public:
    explicit ResultsWidget(QWidget *parent = nullptr);
    void clearResults();

signals:
    void itemUpdated(int totalCount);

public slots:
    void addItem(const QString &url);
    void updateItem(const QString &resultUrl, SearchResult::Status resultStatus, int resultRetCode);
};

#endif // RESULTSWIDGET_H
