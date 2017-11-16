#ifndef UTILS_H
#define UTILS_H

#include <QMessageBox>

class InformationMessageBox : public QMessageBox
{
public:
    InformationMessageBox(const QString &text, QWidget *parent = nullptr);
};

bool isValidUrl(const QString &url);

#endif // UTILS_H
