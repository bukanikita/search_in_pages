#ifndef SEARCHRESULT_H
#define SEARCHRESULT_H

#include <QString>

struct SearchResult
{
    enum class Status {Found, NotFound, Failed, InProgress};
    SearchResult(const QString &url, Status status, int pageReturnCode) :
        m_url(url),
        m_status(status),
        m_pageReturnCode(pageReturnCode) { }
    QString m_url;
    Status m_status;
    int m_pageReturnCode;
};

QString statusToString(SearchResult::Status status, int code);

#endif // SEARCHRESULT_H
