#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include <QObject>
#include <QString>
#include <QRunnable>
#include <QSet>
#include <QVector>
#include <QMutex>
#include "searchresult.h"

class QThreadPool;
class SearchEngine;

struct TaskResult
{
    TaskResult(QSet<QString> &newUrls, const SearchResult &searchResult) :
        m_newUrls(std::move(newUrls)),
        m_searchResults(searchResult) { }
    QSet<QString> m_newUrls;
    SearchResult  m_searchResults;
};

class SearchTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SearchTask(SearchEngine *engine, const QString &url, const QString &text);

signals:
    void populateResults(const TaskResult &taskStatus);

private:
    void run();

private:
    QString m_url;
    QString m_textForSearch;
};

class SearchEngine : public QObject
{
    Q_OBJECT
public:
    explicit SearchEngine(QObject *parent = nullptr);
    void start(const QString &searchUrl,
               int searchThreads,
               int maxPagesForSearch,
               const QString &textForSearch);
    void stop();
    void pause();
    void restart();

signals:
    // we use different parameters here, because we don't want to declare metatype for SearchResult,
    // because it will require default constructor
    void taskFinished(QString resultUrl, SearchResult::Status resultStatus, int resultRetCode);
    void newTaskStarted(const QString &url);
    void resultRead(const QString &resultUrl, SearchResult::Status resultStatus, int resultRetCode);

    void searchFinished();
    void searchPaused();

private slots:
    void onTaskFinished(QString resultUrl, SearchResult::Status resultStatus, int resultRetCode);

public slots:
    void onPopulateResults(const TaskResult &taskResult);

private:
    void startNewTasks();

private:
    QThreadPool *m_pool;

    unsigned int m_urlsForSearch;
    QString m_textForSearch;

    QMutex m_taskMutex;

    QSet<QString> m_itemsInWork;
    QSet<QString> m_itemsToProcess;
    QSet<QString> m_currentlyProcessingItems;
    QSet<QString> m_processedUrl;

    bool m_inPause;
};

#endif // SEARCHENGINE_H
