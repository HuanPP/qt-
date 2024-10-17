#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include <QString>
#include <QTextEdit>

class LogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    void addLogMessage(const QString &message);

private:
    QTextEdit *logTextEdit;
};

#endif // LOG_H
