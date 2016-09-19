#include <sys/mman.h>

#include <fcntl.h>
#include <unistd.h>

#include "adc7kchannel.h"
#include "adc7kboard.h"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStringList>
#include <QTextStream>

Adc7kChannel::Adc7kChannel(const QJsonObject &record, QObject *parent) : QObject(parent)
{
    __name = record["name"].toString();
    __path = "/dev/" + record["path"].toString();
    __path = __path.replace('!', '/');
    QJsonObject bufferobj = record["buffer"].toObject();
    __bufferAddress = bufferobj["address"].toInt();
    __bufferSize = bufferobj["size"].toInt();

    if ((__fd = open(__path.toUtf8().constData(), O_RDONLY | O_NONBLOCK)) > -1) {
        __data = mmap((void *)0, __bufferSize, PROT_READ, MAP_SHARED, __fd, 0);
    }

}

Adc7kChannel::~Adc7kChannel()
{
    if (__data) {
        munmap(__data, __bufferSize);
    }
    if (__fd > -1) {
        close(__fd);
    }
}

QString Adc7kChannel::name() const
{
    return __name;
}

QString Adc7kChannel::path() const
{
    return __path;
}

int Adc7kChannel::fd() const
{
    return __fd;
}

void *Adc7kChannel::data() const
{
    return __data;
}

unsigned long Adc7kChannel::bufferAddress() const
{
    return __bufferAddress;
}

size_t Adc7kChannel::bufferSize() const
{
    return __bufferSize;
}


