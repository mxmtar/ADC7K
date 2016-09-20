#ifndef ADC7KDISPLAY_H
#define ADC7KDISPLAY_H

#include "adc7kboard.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QPushButton>
#include <QResizeEvent>
#include <QTimer>
#include <QVector>
#include <QVBoxLayout>
#include <QWidget>

class Adc7kDisplay : public QWidget
{
    Q_OBJECT
public:
    static const int MaxChannelNumber = 8;

    explicit Adc7kDisplay(QWidget *parent = 0);
    ~Adc7kDisplay();

    void setBoard(Adc7kBoard *board);

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:
    void single();
    void runstop(bool run);
    void onBoardThreadFinished();
    void onBoardSamplerLengthChanged(int length);
    void updateData(int index);
    void autoscale();

private slots:
    void on_timeDivDial_valueChanged(int value);
    void on_voltDivDial_valueChanged(int value);
    void on_voltLevelDial_valueChanged(int value);
    void on_channel1CheckBox_stateChanged(int state);
    void on_channel2CheckBox_stateChanged(int state);
    void on_channel3CheckBox_stateChanged(int state);
    void on_channel4CheckBox_stateChanged(int state);
    void on_channel5CheckBox_stateChanged(int state);
    void on_channel6CheckBox_stateChanged(int state);
    void on_channel7CheckBox_stateChanged(int state);
    void on_channel8CheckBox_stateChanged(int state);
    void on_channelsComboBox_currentIndexChanged(int index);
    void on_showLimitsMarkerTimer_timeout();

private:
    qreal __scaleX, __scaleY[MaxChannelNumber];

    int __offsetX, __offsetY[MaxChannelNumber];
    int __mouseStartX, __mouseStartY;

    bool __xTime;
    bool __yVoltage;

    int __samplesDiv, __pointsDiv[MaxChannelNumber];

    int __timeDivDialValue, __voltDivDialValue, __voltLevelDialValue;

    QHBoxLayout *__mainLayout;

    QVBoxLayout *__leftControlLayout;
    QGroupBox *__channelsGroupBox;
    QVBoxLayout *__channelsGroupBoxLayout;
    QCheckBox *__channel1CheckBox;
    QCheckBox *__channel2CheckBox;
    QCheckBox *__channel3CheckBox;
    QCheckBox *__channel4CheckBox;
    QCheckBox *__channel5CheckBox;
    QCheckBox *__channel6CheckBox;
    QCheckBox *__channel7CheckBox;
    QCheckBox *__channel8CheckBox;
    QGroupBox *__yControlGroupBox;
    QVBoxLayout *__yControlGroupBoxLayout;
    QComboBox *__channelsComboBox;
    QLabel *__voltDivValueLabel;
    QDial *__voltDivDial;
    QLabel *__voltDivLabel;
    QDial *__voltLevelDial;
    QLabel *__voltLevelLabel;
    QPushButton *__autoScaleButton;

    QVBoxLayout *__rightControlLayout;
    QPushButton *__runStopButton;
    QPushButton *__singleButton;
    QGroupBox *__xControlGroupBox;
    QVBoxLayout *__xControlGroupBoxLayout;
    QLabel *__timeDivValueLabel;
    QDial *__timeDivDial;
    QLabel *__timeDivLabel;

    bool __showLimitsMarker;
    QTimer *__showLimitsMarkerTimer;

    Adc7kBoard *__board;

    int __currentChannel;
    int __channelStack[MaxChannelNumber];
    QColor __channelColor[MaxChannelNumber];
    QVector<qint16> __channelData[MaxChannelNumber];
    qint16 __min[MaxChannelNumber], __med[MaxChannelNumber], __max[MaxChannelNumber];

    bool __autoScale;

    int inc125(int input, int limit);
    int dec125(int input);

    void channelStackExclude(int index);
    void channelStackTrim();
    void channelStackHoist(int index);
    int channelStackTop();
    int channelStackCount();
    bool channelStackIsActive(int index);

    bool __continuous;

    void readSettings();
    void writeSettings();
};

#endif // ADC7KDISPLAY_H
