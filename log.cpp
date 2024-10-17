#include "log.h"
#include <QVBoxLayout>
#include <QDateTime>

LogWindow::LogWindow(QWidget *parent)
    : QWidget(parent), logTextEdit(new QTextEdit(this))
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    logTextEdit->setReadOnly(true);  // Make the log text read-only
    layout->addWidget(logTextEdit);
    setLayout(layout);
    setWindowTitle("Parking Activity Log");
    resize(400, 300);  // Set an appropriate size for the log window
}

void LogWindow::addLogMessage(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
}
