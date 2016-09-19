#include <sys/select.h>

#include <unistd.h>

#include "adc7kboard.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>
#include <QVector>

QList<Adc7kBoard *> Adc7kBoard::boardList;

int Adc7kBoard::getChannelIndex(const QString &name)
{
    if (name == "12") {
        return 0;
    } else if (name == "34") {
        return 1;
    } else if (name == "56") {
        return 2;
    } else if (name == "78") {
        return 3;
    } else {
        return -1;
    }
}

Adc7kBoard::Adc7kBoard(const QJsonObject &record, QObject *parent) : QThread(parent)
{
    Adc7kChannel *channel;

    __updateFrequency = 25;

    __driver = record["driver"].toString();
    __path = "/dev/" + record["path"].toString();
    __path = __path.replace('!', '/');
    __name = record["path"].toString();
    __name = __name.remove("adc7k!board-");

    QFile file(__path);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray ba = file.readAll();
        file.close();
        QJsonDocument doc(QJsonDocument::fromJson(ba));
        QJsonObject obj = doc.object();
        __type = obj["type"].toString();
        QJsonObject samplerobj = obj["sampler"].toObject();
        __samplingRate = samplerobj["rate"].toInt();
        __samplerLength = samplerobj["length"].toInt();
        __samplerMaxLength = samplerobj["max_length"].toInt();
        __samplerDivider = samplerobj["divider"].toInt();
        QJsonArray channelArray = obj["channels"].toArray();
        for (int i = 0; i < channelArray.size(); ++i) {
            channel = new Adc7kChannel(channelArray[i].toObject());
            __channelList[getChannelIndex(channel->name())] = channel;
        }
    }
}

Adc7kBoard::~Adc7kBoard()
{
    stop();
    wait();
    for (int i = 0; i < MaxChannelNumber; ++i) {
        delete __channelList[i];
    }
}

QString Adc7kBoard::driver() const
{
    return __driver;
}

QString Adc7kBoard::path() const
{
    return __path;
}

QString Adc7kBoard::type() const
{
    return __type;
}

QString Adc7kBoard::name() const
{
    return __name;
}

QString Adc7kBoard::toString() const
{
    return "[" + __type + "] " + __name;
}

int Adc7kBoard::samplingRate() const
{
    return __samplingRate;
}

int Adc7kBoard::samplerLength() const
{
    return __samplerLength;
}

int Adc7kBoard::samplerMaxLength() const
{
    return __samplerMaxLength;
}

int Adc7kBoard::samplerDivider() const
{
    return __samplerDivider;
}

Adc7kChannel *Adc7kBoard::channel(int index) const
{
    if ((index >= 0) && (index < MaxChannelNumber)) {
        return __channelList[index];
    } else {
        return (Adc7kChannel *)0;
    }
}

Adc7kChannel *Adc7kBoard::channel(const QString &name) const
{
    for (int i = 0; i < MaxChannelNumber; ++i) {
        if (__channelList[i]->name() == name) {
            return __channelList[i];
        }
    }
    return (Adc7kChannel *)0;
}

void Adc7kBoard::samplerStart(bool continuous)
{
    QFile file(__path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QString("sampler.start(%1)").arg(continuous ? 1 : 0).toLatin1());
        file.close();
    }
}

void Adc7kBoard::samplerStop()
{
    QFile file(__path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QString("sampler.stop()").toLatin1());
        file.close();
    }
}

void Adc7kBoard::setSamplerLength(int length)
{
    if (length != __samplerLength) {
        QFile file(__path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QString("sampler.length(%1)").arg(length).toLatin1());
            file.close();
            __samplerLength = length;
            emit samplerLengthChanged(length);
        }
    }
}

void Adc7kBoard::setSamplerDivider(int divider)
{
    if (divider != __samplerDivider) {
        QFile file(__path);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(QString("sampler.divider(%1)").arg(divider).toLatin1());
            file.close();
            __samplerDivider = divider;
            emit samplerDividerChanged(divider);
        }
    }
}

void Adc7kBoard::setSamplerContinuous(bool continuous)
{
    __samplerContinuous = continuous;
}

void Adc7kBoard::setUpdateFrequency(int frequency)
{
    __updateFrequency = frequency;
}

void Adc7kBoard::adcReset()
{
    QFile file(__path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QString("adc.reset()").toLatin1());
        file.close();
    }
}

void Adc7kBoard::adcWrite(quint8 addr, quint16 data)
{
    QFile file(__path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QString("adc.write(0x%1,0x%2)").arg(addr, 2, 16).arg(data, 4, 16).toLatin1());
        file.close();
    }
}

void Adc7kBoard::run()
{
    bool done[MaxChannelNumber];
    int maxfd, res;
    ssize_t rlen;
    fd_set rfds;
    struct timeval timeout;

    for (int i = 0; i < MaxChannelNumber; ++i) {
        done[i] = false;
        __accumulatedSamplesCount[i] = 0;
    }

    samplerStart(__samplerContinuous);

    __run = true;

    while (__run) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        // prepare select
        __run = false;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;
        maxfd = 0;
        FD_ZERO(&rfds);
        for (int i = 0; i < MaxChannelNumber; ++i) {
            if ((done[i] == false) && (__channelList[i]->fd() > -1)) {
                FD_SET(__channelList[i]->fd(), &rfds);
                maxfd = qMax(__channelList[i]->fd(), maxfd);
                __run = true;
            }
        }
        if (!__run) {
            break;
        }
        res = select(maxfd + 1, &rfds, NULL, NULL, &timeout);
        if (res > 0) {
            for (int i = 0; i < MaxChannelNumber; ++i) {
                if ((__channelList[i]->fd() > -1) && (FD_ISSET(__channelList[i]->fd(), &rfds))) {
                    rlen = read(__channelList[i]->fd(), __channelList[i]->data(), __channelList[i]->bufferSize());
                    if (rlen > 0) {
                        emit dataReceived(i);
                        if (__accumulatedSamplesCount[i] == 0) {
                            emit dataUpdated(i);
                        }
                        __accumulatedSamplesCount[i] += __samplerLength;
                        if (__accumulatedSamplesCount[i] >= __samplingRate / __updateFrequency) {
                            __accumulatedSamplesCount[i] = 0;
                        }
                    } else if (rlen == 0) {
                        done[i] = true;
                    } else {
                        goto run_end;
                    }
                }
            }
        } else if (res < 0) {
            goto run_end;
        }
    }

run_end:
    return;
}

void Adc7kBoard::stop()
{
    samplerStop();
    __run = false;
}

bool Adc7kBoard::searchBoards()
{
    bool res;

    boardList.clear();

    QFile file("/dev/adc7k/subsystem");
    if ((res = file.open(QIODevice::ReadOnly))) {
        QByteArray ba = file.readAll();
        file.close();
        QJsonDocument doc(QJsonDocument::fromJson(ba));
        QJsonObject obj = doc.object();
        QJsonArray boardArray = obj["boards"].toArray();
        for (int i = 0; i < boardArray.size(); ++i) {
            boardList << new Adc7kBoard(boardArray[i].toObject());
        }
    }
    return res;
}

void Adc7kBoard::releaseBoards()
{
    while (!boardList.isEmpty()) {
        delete boardList.takeFirst();
    }
}
