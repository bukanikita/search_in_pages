#include <QtWidgets>
#include <thread>

#include "mainwindow.h"
#include "searchengine.h"
#include "utils.h"
#include "resultswidget.h"

Window::Window() : m_paused(false)
{
    QGroupBox *urlGroup = new QGroupBox(this);
    QHBoxLayout *urlLayout = new QHBoxLayout(this);
    m_urlLineEdit = new QLineEdit(this);
    m_urlLineEdit->setPlaceholderText(tr("Enter URL here:"));
    urlLayout->addWidget(m_urlLineEdit);
    urlGroup->setLayout(urlLayout);

    QGroupBox *spinGroup = new QGroupBox(this);
    QHBoxLayout *spinLayout = new QHBoxLayout(this);

    QLabel *threadsLabel = new QLabel(tr("Enter maximum number of threads here:"), this);
    m_threadsSpinBox = new QSpinBox(this);
    m_threadsSpinBox->setMinimum(1);
    // set maximum to avoid that user will enter any enormous number
    m_threadsSpinBox->setMaximum(std::thread::hardware_concurrency());

    QLabel *maxUTLsLabel = new QLabel(tr("Enter maximum number of urls for search here:"), this);
    m_urlsSpinBox = new QSpinBox(this);
    m_urlsSpinBox->setMinimum(1);
    m_urlsSpinBox->setMaximum(std::numeric_limits<int>::max());

    spinLayout->addWidget(threadsLabel);
    spinLayout->addWidget(m_threadsSpinBox);
    spinLayout->addWidget(maxUTLsLabel);
    spinLayout->addWidget(m_urlsSpinBox);

    spinGroup->setLayout(spinLayout);

    QGroupBox *textGroup = new QGroupBox(this);
    QHBoxLayout *textLayout = new QHBoxLayout(this);
    m_textLineEdit = new QLineEdit(this);
    m_textLineEdit->setPlaceholderText(tr("Enter text for search here:"));
    textLayout->addWidget(m_textLineEdit);
    textGroup->setLayout(textLayout);

    QGroupBox *buttonsGroup = new QGroupBox(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(this);
    m_startButton = new QPushButton(tr("Start"), this);
    m_pauseButton = new QPushButton(tr("Pause"), this);
    m_stopButton = new QPushButton(tr("Stop"), this);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);
    buttonsLayout->addWidget(m_startButton);
    buttonsLayout->addWidget(m_pauseButton);
    buttonsLayout->addWidget(m_stopButton);
    buttonsGroup->setLayout(buttonsLayout);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setMinimum(0);

    m_resultsView = new ResultsWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(urlGroup);
    mainLayout->addWidget(spinGroup);
    mainLayout->addWidget(textGroup);
    mainLayout->addWidget(buttonsGroup);
    mainLayout->addWidget(m_resultsView);
    mainLayout->addWidget(m_progressBar);
    setLayout(mainLayout);

    m_stopMessageBox = new InformationMessageBox("Stopping...", this);
    m_pauseMessageBox = new InformationMessageBox("Pausing...", this);

    setWindowTitle(tr("Search for text"));

    m_searchEngine = new SearchEngine(this);

    bool isOK = connect(m_startButton, &QPushButton::clicked, this, &Window::onStartClicked);
    Q_ASSERT(isOK);
    isOK = connect(m_stopButton,  &QPushButton::clicked, this, &Window::onStopClicked);
    Q_ASSERT(isOK);
    isOK = connect(m_pauseButton, &QPushButton::clicked, this, &Window::onPauseClicked);
    Q_ASSERT(isOK);
    isOK = connect(m_searchEngine, &SearchEngine::newTaskStarted, m_resultsView, &ResultsWidget::addItem);
    Q_ASSERT(isOK);
    isOK = connect(m_searchEngine, &SearchEngine::resultRead, m_resultsView, &ResultsWidget::updateItem);
    Q_ASSERT(isOK);
    isOK = connect(m_searchEngine, &SearchEngine::searchFinished, this, &Window::onSearchFinished);
    Q_ASSERT(isOK);
    isOK = connect(m_searchEngine, &SearchEngine::searchPaused, m_pauseMessageBox, &InformationMessageBox::hide);
    Q_ASSERT(isOK);
    isOK = connect(m_resultsView, &ResultsWidget::itemUpdated, m_progressBar, &QProgressBar::setValue);
    Q_ASSERT(isOK);
}

void Window::onStartClicked()
{
    const QString urlForSearch = m_urlLineEdit->text();
    // if we are in paused state, in any case, we should not verify urlForSearch again, because it is valid from previous run.
    if (!m_paused && !isValidUrl(urlForSearch))
    {
        QMessageBox::warning(this, "Warning!", "Invalid URL entered!");
        return;
    }

    m_startButton->setEnabled(false);
    m_pauseButton->setEnabled(true);

    if (m_paused)
    {
        m_paused = false;
        m_searchEngine->restart();
        return;
    }

    m_resultsView->clearResults();

    m_stopButton->setEnabled(true);

    m_urlLineEdit->setEnabled(false);
    m_textLineEdit->setEnabled(false);

    m_threadsSpinBox->setEnabled(false);
    m_urlsSpinBox->setEnabled(false);

    m_progressBar->reset();

    int urlsToSearch = m_urlsSpinBox->value();
    m_progressBar->setMaximum(urlsToSearch);

    m_resultsView->addItem(urlForSearch);

    m_searchEngine->start(urlForSearch, m_threadsSpinBox->value(), urlsToSearch, m_textLineEdit->text());
}

void Window::onStopClicked()
{
    m_stopMessageBox->show();
    m_searchEngine->stop();
}

void Window::onPauseClicked()
{
    m_pauseMessageBox->show();
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_searchEngine->pause();
    m_paused = true;
}

void Window::onSearchFinished()
{
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
    m_stopButton->setEnabled(false);

    m_urlLineEdit->setEnabled(true);
    m_textLineEdit->setEnabled(true);

    m_threadsSpinBox->setEnabled(true);
    m_urlsSpinBox->setEnabled(true);
    m_paused = false;
    m_stopMessageBox->hide();
}

Window::~Window()
{
    onStopClicked();
}
