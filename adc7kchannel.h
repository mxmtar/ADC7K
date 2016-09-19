#ifndef ADC7KCHANNEL_H
#define ADC7KCHANNEL_H

#include <QJsonObject>
#include <QObject>
#include <QString>

class Adc7kChannel : public QObject
{
    Q_OBJECT
public:
    Adc7kChannel(const QJsonObject &record, QObject *parent = 0);
    ~Adc7kChannel();

    QString name() const;
    QString path() const;
    int fd() const;
    void *data() const;

    unsigned long bufferAddress() const;
    size_t bufferSize() const;

public slots:

private:
    QString __name;
    QString __path;

    unsigned long __bufferAddress;
    size_t __bufferSize;

    int __fd;
    void *__data;
};

#endif // ADC7KCHANNEL_H
