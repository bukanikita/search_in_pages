#include <QString>
#include <QUrl>
#include <QDebug>

#include "utils.h"

InformationMessageBox::InformationMessageBox(const QString &text, QWidget *parent) : QMessageBox(parent)
{
    setWindowTitle(tr("Information"));
    setText(text);
    setStandardButtons(0);
}

bool isValidUrl(const QString &textUrl)
{
    bool ret = false;
    if (textUrl.startsWith("http://", Qt::CaseInsensitive))
    {
        ret = QUrl(textUrl, QUrl::StrictMode).isValid();
    }
    return ret;
}
