#include "searchengine.h"
#include "utils.h"
#include <QRunnable>
#include <QThreadPool>
#include <QDebug>
#include <QNetworkReply>
#include <QEventLoop>
#include <QNetworkAccessManager>

SearchEngine::SearchEngine(QObject *parent) : QObject(parent), m_urlsForSearch(0), m_inPause(false)
{
    m_pool = new QThreadPool(this);
    qRegisterMetaType<SearchResult::Status>("SearchResult::Status");
    bool isOK = connect(this, &SearchEngine::taskFinished, this, &SearchEngine::onTaskFinished);
    Q_ASSERT(isOK);
}

void SearchEngine::start(const QString &searchUrl,
                         int searchThreads,
                         int maxPagesForSearch,
                         const QString &textForSearch)
{
    Q_ASSERT(searchThreads > 0 && maxPagesForSearch > 0);

    // -1 here, because we will start at least one task, so it handles this case
    m_urlsForSearch = maxPagesForSearch - 1;
    m_textForSearch = textForSearch;
    m_pool->setMaxThreadCount(searchThreads);
    m_pool->start(new SearchTask(this, searchUrl, textForSearch));
}

void SearchEngine::stop()
{
    QMutexLocker locker(&m_taskMutex);
    m_itemsToProcess.clear();
    m_processedUrl.clear();
    m_currentlyProcessingItems.clear();
    m_urlsForSearch = 0;
    if (m_inPause)
    {
        emit searchFinished();
    }
}

void SearchEngine::pause()
{
    m_inPause = true;
}

void SearchEngine::restart()
{
    m_inPause = false;
    startNewTasks();
}

void SearchEngine::onTaskFinished(QString resultUrl, SearchResult::Status resultStatus, int resultRetCode)
{
    emit resultRead(resultUrl, resultStatus, resultRetCode);

    QMutexLocker locker(&m_taskMutex);
    const bool anythingToProcess = !m_currentlyProcessingItems.empty();
    locker.unlock();
    if (anythingToProcess)
    {
        startNewTasks();
    }
    else if (m_pool->activeThreadCount() == 1) // current is still active
    {
        stop();
        emit searchFinished();
    }
}

void SearchEngine::onPopulateResults(const TaskResult &taskResult)
{
    QMutexLocker locker(&m_taskMutex);
    auto itItemsInWork = m_itemsInWork.find(taskResult.m_searchResults.m_url);
    if (itItemsInWork != m_itemsInWork.end())
    {
        m_itemsInWork.erase(itItemsInWork);
    }
    m_processedUrl.insert(taskResult.m_searchResults.m_url);
    for (auto it = taskResult.m_newUrls.cbegin(); it != taskResult.m_newUrls.cend() && m_urlsForSearch; ++it)
    {
        // if we did not process this url previously
        if (!m_processedUrl.contains(*it) &&
            !m_currentlyProcessingItems.contains(*it) &&
            !m_itemsToProcess.contains(*it) &&
            !m_itemsInWork.contains(*it))
        {
            // populate new item, which should be processed
            m_itemsToProcess.insert(*it);
            // handle number of urls, which we should to process
            --m_urlsForSearch;
        }
    }
    if (m_currentlyProcessingItems.empty())
    {
        m_currentlyProcessingItems = std::move(m_itemsToProcess);
    }
    emit taskFinished(taskResult.m_searchResults.m_url, taskResult.m_searchResults.m_status, taskResult.m_searchResults.m_pageReturnCode);
}

void SearchEngine::startNewTasks()
{
    if (m_inPause)
    {
        if (m_pool->activeThreadCount() == 1)
        {
            emit searchPaused();
        }
        return;
    }
    // current is still active
    int availableThreadsCuunt = m_pool->maxThreadCount() - m_pool->activeThreadCount() + 1;
    QStringList urlsToProcess;
    QMutexLocker locker(&m_taskMutex);
    for (int i = 0; i < m_currentlyProcessingItems.size() && i < availableThreadsCuunt; ++i)
    {
        auto it = m_currentlyProcessingItems.begin();
        urlsToProcess << *it;
        m_itemsInWork.insert(*it);
        m_currentlyProcessingItems.erase(it);
    }
    locker.unlock();
    for (const auto &item : urlsToProcess)
    {
        emit newTaskStarted(item);
        m_pool->start(new SearchTask(this, item, m_textForSearch));
    }
}

SearchTask::SearchTask(SearchEngine *engine, const QString &url, const QString &text) :
    m_url(url),
    m_textForSearch(text)
{
    bool isOK = connect(this, &SearchTask::populateResults, engine, &SearchEngine::onPopulateResults, Qt::DirectConnection);
    Q_ASSERT(isOK);
}

void SearchTask::run()
{
    QNetworkAccessManager networkManager;
    QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(m_url)));

    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();

    int pageStatusCode = -1;
    QSet<QString> urlsOnPage;
    SearchResult::Status status = SearchResult::Status::Failed;

    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (statusCode.isValid())
    {
        pageStatusCode = statusCode.toInt();
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        const QString replyData = reply->readAll();
        status = replyData.contains(m_textForSearch) ? SearchResult::Status::Found : SearchResult::Status::NotFound;

        // this part is taken from web, but I really did not have time to do it.
        QRegExp rxp("((?:http)://\\S+)\"");
        int pos = 0;
        while ((pos = rxp.indexIn(replyData, pos)) != -1)
        {
            urlsOnPage.insert(rxp.cap(1));
            pos += rxp.matchedLength();
        }
    }

    emit populateResults(TaskResult{urlsOnPage, SearchResult{m_url, status, pageStatusCode}});
}
