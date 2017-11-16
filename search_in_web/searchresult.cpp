#include "searchresult.h"

QString statusToString(SearchResult::Status status, int code)
{
    QString ret;
    switch (status)
    {
    case SearchResult::Status::Found:
        ret = "Found";
        break;
    case SearchResult::Status::NotFound:
        ret = "NotFound";
        break;
    case SearchResult::Status::Failed:
        ret = "Error (" + QString::number(code) + ")";
        break;
    case SearchResult::Status::InProgress:
        ret = "In Progress";
        break;
    default:
        Q_ASSERT(false);
        break;
    }
    return ret;
}
