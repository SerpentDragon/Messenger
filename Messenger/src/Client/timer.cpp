#include "../../include/Client/timer.h"

Timer::Timer(QObject *parent)
    : QObject{parent}
{}

void Timer::start(qint64 time, int signal, const std::vector<int>& ids)
{
    qint64 now = QDateTime::currentSecsSinceEpoch();
    qint64 diff = time - now;

    qDebug() << "\n--------\n" << diff << "\n--------\n";

    QTimer::singleShot(diff * 1000, this, [=, this]() {
        emit timeout(signal, ids);
        qDebug() << "SIGNAL EMITTED\n";
    });
}
