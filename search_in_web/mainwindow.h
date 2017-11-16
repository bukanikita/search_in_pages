#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

class SearchEngine;
class QPushButton;
class QLineEdit;
class QSpinBox;
class ResultsWidget;
class InformationMessageBox;
class QProgressBar;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();
    ~Window();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onPauseClicked();

public slots:
    void onSearchFinished();

private:
    QLineEdit *m_urlLineEdit;
    QLineEdit *m_textLineEdit;

    QSpinBox *m_threadsSpinBox;
    QSpinBox *m_urlsSpinBox;

    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;

    ResultsWidget *m_resultsView;

    SearchEngine *m_searchEngine;

    bool m_paused;

    InformationMessageBox *m_stopMessageBox;
    InformationMessageBox *m_pauseMessageBox;

    QProgressBar *m_progressBar;
};


#endif // MAINWINDOW_H
