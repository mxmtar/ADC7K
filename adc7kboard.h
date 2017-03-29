#ifndef ADC7KBOARD_H
#define ADC7KBOARD_H

#include "adc7kchannel.h"

#include <vector>

#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QThread>
#include <QVector>

class Adc7kBoard : public QThread
{
    Q_OBJECT
public:
    static const int MaxChannelNumber = 4;
    static bool searchBoards();
    static void releaseBoards();
    static QList<Adc7kBoard *> boardList;

    Adc7kBoard(const QJsonObject &record, QObject *parent = 0);
    ~Adc7kBoard();

    QString driver() const;
    QString path() const;
    QString type() const;
    QString name() const;
    QString toString() const;
    int samplingRate() const;
    int samplerLength() const;
    int samplerMaxLength() const;
    int samplerDivider() const;

    Adc7kChannel *channel(int index) const;
    Adc7kChannel *channel(const QString &name) const;

    int getChannelIndex(const QString &name);

    void stop();
    void run() Q_DECL_OVERRIDE;

signals:
    void dataReceived(int ch);
    void dataUpdated(int ch);
    void samplerLengthChanged(int length);
    void samplerDividerChanged(int length);

public slots:
    void samplerStart(bool continuous);
    void samplerStop();
    void setSamplerLength(int length);
    void setSamplerDivider(int divider);
    void setSamplerContinuous(bool continuous);
    void setUpdateFrequency(int frequency);
    void adcReset();
    void adcWrite(quint8 addr, quint16 data);
    quint32 regRead(quint32 addr);
    void regWrite(quint32 addr, quint32 data);

private:
    QString __driver;
    QString __path;
    QString __type;
    QString __name;
    int __samplingRate;
    int __samplerLength;
    int __samplerMaxLength;
    int __samplerDivider;
    bool __samplerContinuous;

    int __updateFrequency;

    Adc7kChannel *__channelList[MaxChannelNumber];
    int __accumulatedSamplesCount[MaxChannelNumber];
    bool __run;
};

#endif // ADC7KBOARD_H
