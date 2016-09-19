#ifndef ADC7KBOARD_H
#define ADC7KBOARD_H

#include <QObject>

class Adc7kBoard : public QObject
{
    Q_OBJECT
public:
    explicit Adc7kBoard(QObject *parent = 0);

signals:

public slots:
};

#endif // ADC7KBOARD_H