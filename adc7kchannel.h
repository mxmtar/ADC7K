#ifndef ADC7KCHANNEL_H
#define ADC7KCHANNEL_H

#include <QObject>

class Adc7kChannel : public QObject
{
    Q_OBJECT
public:
    explicit Adc7kChannel(QObject *parent = 0);

signals:

public slots:
};

#endif // ADC7KCHANNEL_H