#ifndef TIMER_H
#define TIMER_H

#include <QDateTime>
#include <QObject>
#include <QTimer>

class Timer : public QObject
{
    Q_OBJECT

public:

    explicit Timer(QObject *parent = nullptr);

    void start(qint64 unixTimestamp, int signal, const std::vector<int>& ids);

signals:

    void timeout(int msg_type, const std::vector<int>& id);
};

#endif // TIMER_H
