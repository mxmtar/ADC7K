#include "adc7kdisplay.h"

#include <QtMath>

#include <QDebug>
#include <QPainter>
#include <QPalette>
#include <QSettings>

Adc7kDisplay::Adc7kDisplay(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("QGroupBox{color:lightgrey;background:black;border-style:outset;border-width:2px;border-radius:5px;border-color:beige;}QLabel{color:lightgrey;background:black;}");

    for (int i = 0; i < MaxChannelNumber; ++i) {
        __channelStack[i] = -1;
    }

    readSettings();

    __channelColor[0] = QColor(Qt::red);
    __channelColor[1] = QColor(Qt::green);
    __channelColor[2] = QColor(Qt::blue);
    __channelColor[3] = QColor(Qt::yellow);
    __channelColor[4] = QColor(Qt::cyan);
    __channelColor[5] = QColor(Qt::magenta);
    __channelColor[6] = QColor(255, 102, 0); // orange
    __channelColor[7] = QColor(244, 164, 96); // brown

    __xTime = false;
    __scaleX = 100.0f / __samplesDiv;
    __offsetX = 0;

    __yVoltage = false;
    for (int i = 0; i < MaxChannelNumber; ++i) {
        __pointsDiv[i] = 5000;
        __scaleY[i] = 100.0f / __pointsDiv[i];
        __offsetY[i] = 0;
        __min[i] = -0x2000;
        __med[i] = 0;
        __max[i] = 0x1fff;
    }

     __currentChannel = channelStackTop();

     __continuous = true;

     __showLimitsMarker = false;
     __showLimitsMarkerTimer = new QTimer();
     connect(__showLimitsMarkerTimer, SIGNAL(timeout()), this, SLOT(on_showLimitsMarkerTimer_timeout()));

    __mainLayout = new QHBoxLayout();

    __leftControlLayout = new QVBoxLayout();

    __channelsGroupBox = new QGroupBox();
    __channelsGroupBoxLayout = new QVBoxLayout();
    __channel1CheckBox = new QCheckBox("Channel 1");
    __channel1CheckBox->setStyleSheet("QCheckBox{border:none;color:red;}");
    __channel1CheckBox->setChecked(channelStackIsActive(0));
    connect( __channel1CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel1CheckBox_stateChanged(int)));
    __channel2CheckBox = new QCheckBox("Channel 2");
    __channel2CheckBox->setStyleSheet("QCheckBox{border:none;color:green;}");
    __channel2CheckBox->setChecked(channelStackIsActive(1));
    connect( __channel2CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel2CheckBox_stateChanged(int)));
    __channel3CheckBox = new QCheckBox("Channel 3");
    __channel3CheckBox->setStyleSheet("QCheckBox{border:none;color:blue;}");
    __channel3CheckBox->setChecked(channelStackIsActive(2));
    connect( __channel3CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel3CheckBox_stateChanged(int)));
    __channel4CheckBox = new QCheckBox("Channel 4");
    __channel4CheckBox->setStyleSheet("QCheckBox{border:none;color:yellow;}");
    __channel4CheckBox->setChecked(channelStackIsActive(3));
    connect( __channel4CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel4CheckBox_stateChanged(int)));
    __channel5CheckBox = new QCheckBox("Channel 5");
    __channel5CheckBox->setStyleSheet("QCheckBox{border:none;color:cyan;}");
    __channel5CheckBox->setChecked(channelStackIsActive(4));
    connect( __channel5CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel5CheckBox_stateChanged(int)));
    __channel6CheckBox = new QCheckBox("Channel 6");
    __channel6CheckBox->setStyleSheet("QCheckBox{border:none;color:magenta;}");
    __channel6CheckBox->setChecked(channelStackIsActive(5));
    connect( __channel6CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel6CheckBox_stateChanged(int)));
    __channel7CheckBox = new QCheckBox("Channel 7");
    __channel7CheckBox->setStyleSheet("QCheckBox{border:none;color:rgb(255,102,0);}");
    __channel7CheckBox->setChecked(channelStackIsActive(6));
    connect( __channel7CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel7CheckBox_stateChanged(int)));
    __channel8CheckBox = new QCheckBox("Channel 8");
    __channel8CheckBox->setStyleSheet("QCheckBox{border:none;color:rgb(244,164,96);}");
    __channel8CheckBox->setChecked(channelStackIsActive(7));
    connect( __channel8CheckBox, SIGNAL(stateChanged(int)), this, SLOT(on_channel8CheckBox_stateChanged(int)));
    __channelsGroupBoxLayout->addWidget(__channel1CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel2CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel3CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel4CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel5CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel6CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel7CheckBox);
    __channelsGroupBoxLayout->addWidget(__channel8CheckBox);
    __channelsGroupBox->setLayout(__channelsGroupBoxLayout);
    __leftControlLayout->addWidget(__channelsGroupBox);

    __yControlGroupBox = new QGroupBox();
    __yControlGroupBoxLayout = new QVBoxLayout();
    __channelsComboBox = new QComboBox();
    QStringList channels;
    channels << "Channel 1" << "Channel 2" << "Channel 3" << "Channel 4" << "Channel 5" << "Channel 6" << "Channel 7" << "Channel 8";
    __channelsComboBox->addItems(channels);
    __channelsComboBox->setCurrentIndex(__currentChannel);
    connect(__channelsComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &Adc7kDisplay::on_channelsComboBox_currentIndexChanged);
    __yControlGroupBoxLayout->addWidget(__channelsComboBox);
    __voltDivValueLabel = new QLabel(QString("%1").arg(__pointsDiv[__currentChannel]));
    __voltDivValueLabel->setAlignment(Qt::AlignCenter);
    __yControlGroupBoxLayout->addWidget(__voltDivValueLabel);
    __voltDivDial = new QDial();
    __voltDivDialValue = __voltDivDial->value();
    __voltDivDial->setWrapping(true);
    connect(__voltDivDial, SIGNAL(valueChanged(int)), this, SLOT(on_voltDivDial_valueChanged(int)));
    __yControlGroupBoxLayout->addWidget(__voltDivDial);
    __voltDivLabel = new QLabel(QString("Points/DIV"));
    __voltDivLabel->setAlignment(Qt::AlignCenter);
    __yControlGroupBoxLayout->addWidget(__voltDivLabel);
    __voltLevelDial = new QDial();
    __voltLevelDialValue = __voltLevelDial->value();
    __voltLevelDial->setWrapping(true);
    connect(__voltLevelDial, SIGNAL(valueChanged(int)), this, SLOT(on_voltLevelDial_valueChanged(int)));
    __yControlGroupBoxLayout->addWidget(__voltLevelDial);
    __voltLevelLabel = new QLabel(QString("Level"));
    __voltLevelLabel->setAlignment(Qt::AlignCenter);
    __yControlGroupBoxLayout->addWidget(__voltLevelLabel);
    __autoScaleButton = new QPushButton("Auto Scale");
    connect(__autoScaleButton, SIGNAL(clicked()), this, SLOT(autoscale()));
    __yControlGroupBoxLayout->addWidget(__autoScaleButton);
    __yControlGroupBox->setLayout(__yControlGroupBoxLayout);
    __leftControlLayout->addWidget(__yControlGroupBox);

    __leftControlLayout->addStretch(1);

    __rightControlLayout = new QVBoxLayout();
    __runStopButton = new QPushButton("Run");
    __runStopButton->setCheckable(true);
    connect(__runStopButton, SIGNAL(toggled(bool)), this, SLOT(runstop(bool)));
    __rightControlLayout->addWidget(__runStopButton);
    __singleButton = new QPushButton("Single");
    connect(__singleButton, SIGNAL(clicked()), this, SLOT(single()));
    __rightControlLayout->addWidget(__singleButton);


    __xControlGroupBox = new QGroupBox();
    __xControlGroupBoxLayout = new QVBoxLayout();
    __timeDivValueLabel = new QLabel(QString("%1").arg(__samplesDiv));
    __timeDivValueLabel->setAlignment(Qt::AlignCenter);
    __xControlGroupBoxLayout->addWidget(__timeDivValueLabel);
    __timeDivDial = new QDial();
    __timeDivDialValue = __timeDivDial->value();
    __timeDivDial->setSingleStep(1);
    __timeDivDial->setPageStep(1);
    connect(__timeDivDial, SIGNAL(valueChanged(int)), this, SLOT(on_timeDivDial_valueChanged(int)));
    __timeDivDial->setWrapping(true);
    __xControlGroupBoxLayout->addWidget(__timeDivDial);
    __timeDivLabel = new QLabel(QString("Samples/DIV"));
    __timeDivLabel->setAlignment(Qt::AlignCenter);
    __xControlGroupBoxLayout->addWidget(__timeDivLabel);
    __xControlGroupBox->setLayout(__xControlGroupBoxLayout);
    __rightControlLayout->addWidget(__xControlGroupBox);

    __rightControlLayout->addStretch(1);

    __mainLayout->addLayout(__leftControlLayout);
    __mainLayout->addStretch(1);
    __mainLayout->addLayout(__rightControlLayout);

    setLayout(__mainLayout);

    setEnabled(false);
}

Adc7kDisplay::~Adc7kDisplay()
{
    writeSettings();

    delete __showLimitsMarkerTimer;

    delete __channel1CheckBox;
    delete __channel2CheckBox;
    delete __channel3CheckBox;
    delete __channel4CheckBox;
    delete __channel5CheckBox;
    delete __channel6CheckBox;
    delete __channel7CheckBox;
    delete __channel8CheckBox;
    delete __channelsGroupBoxLayout;
    delete __channelsGroupBox;

    delete __channelsComboBox;
    delete __voltDivValueLabel;
    delete __voltDivDial;
    delete __voltDivLabel;
    delete __voltLevelDial;
    delete __voltLevelLabel;
    delete __autoScaleButton;
    delete __yControlGroupBoxLayout;
    delete __yControlGroupBox;

    delete __leftControlLayout;

    delete __runStopButton;
    delete __singleButton;

    delete __timeDivValueLabel;
    delete __timeDivDial;
    delete __timeDivLabel;
    delete __xControlGroupBoxLayout;
    delete __xControlGroupBox;

    delete __rightControlLayout;

    delete __mainLayout;
}

void Adc7kDisplay::setBoard(Adc7kBoard *board)
{
    if ((__board = board)) {
        connect(board, SIGNAL(dataUpdated(int)), this, SLOT(updateData(int)), Qt::UniqueConnection);
        connect(board, SIGNAL(finished()), this, SLOT(onBoardThreadFinished()), Qt::UniqueConnection);
        connect(board, SIGNAL(samplerLengthChanged(int)), this, SLOT(onBoardSamplerLengthChanged(int)), Qt::UniqueConnection);
        setEnabled(true);
        for (int i = 0; i < MaxChannelNumber; ++i) {
            __channelData[i].resize(board->samplerLength());
        }
    }
}

void Adc7kDisplay::paintEvent(QPaintEvent *)
{
    int step;
    int offsetScreenX, offsetScreenY;
    int lvx, rvx;
    int index;

    QPainter painter(this);

    painter.setBrush(QColor(Qt::black));
    painter.drawRect(rect());

    offsetScreenX = 0;
    offsetScreenY = height() / 2;

    if ((__offsetX + offsetScreenX) < 0) {
        lvx = (0 - (__offsetX + offsetScreenX)) / __scaleX;
    } else {
         lvx = 0;
    }
    rvx = lvx + 2 + width() / __scaleX;

    painter.setWorldTransform(QTransform(1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, __offsetX + offsetScreenX, offsetScreenY, 1.0f));

    if (__samplesDiv > 100) {
        step = __samplesDiv / 100;
    } else {
        step = 1;
    }

    for (int j = 0; j < MaxChannelNumber; ++j) {
        index = __channelStack[j];
        if ((index > -1) && (index < MaxChannelNumber)) {
            painter.setPen(__channelColor[index]);
            for (int i = lvx, e = qMin((int)__channelData[index].size(), rvx) - step; i < e; i += step) {
                painter.drawLine(QPointF(__scaleX * i, __scaleY[index] * __channelData[index][i] - __offsetY[index]), QPointF(__scaleX * (i + step), __scaleY[index] * __channelData[index][i + step] - __offsetY[index]));
            }
        }
    }

    // draw grid
    painter.setWorldTransform(QTransform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    painter.setPen(Qt::darkGray);
    if (__xTime) {
        for (int i = (width() / 2) % 100, e = width(); i < e; i += 100) {
            painter.drawLine(i, 0, i, height());
        }
    } else {
        for (int i = 100, e = width(); i < e; i += 100) {
            painter.drawLine(i, 0, i, height());
        }
    }
    for (int i = height() - (height() / 2) % 100, e = 0; i > e; i -= 100) {
        painter.drawLine(0, i, width(), i);
    }

    // limits marker
    if (__showLimitsMarker) {
        painter.setWorldTransform(QTransform(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, offsetScreenY, 1.0f));
        if ((__currentChannel > -1) && (__currentChannel < MaxChannelNumber)) {
            painter.setPen(QPen(QBrush(Qt::white), 2.0f, Qt::DashDotDotLine));
            painter.drawLine(QPointF(0.0f, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __max[__currentChannel]), QPointF(width(), __offsetY[__currentChannel] - __scaleY[__currentChannel] * __max[__currentChannel]));
            painter.drawLine(QPointF(0.0f, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __med[__currentChannel]), QPointF(width(), __offsetY[__currentChannel] - __scaleY[__currentChannel] * __med[__currentChannel]));
            painter.drawLine(QPointF(0.0f, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __min[__currentChannel]), QPointF(width(), __offsetY[__currentChannel] - __scaleY[__currentChannel] * __min[__currentChannel]));
            painter.setPen(Qt::white);
            painter.drawRect(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __max[__currentChannel] + 2, 60, 20));
            painter.drawRect(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __med[__currentChannel] - 10, 60, 20));
            painter.drawRect(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __min[__currentChannel] - 22, 60, 20));
            painter.drawText(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __max[__currentChannel] + 2, 60, 20), Qt::AlignCenter, QString("%1").arg(__max[__currentChannel]));
            painter.drawText(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __med[__currentChannel] - 10, 60, 20), Qt::AlignCenter, QString("%1").arg(__med[__currentChannel]));
            painter.drawText(QRect(width() * 3 / 4, __offsetY[__currentChannel] -__scaleY[__currentChannel] * __min[__currentChannel] - 22, 60, 20), Qt::AlignCenter, QString("%1").arg(__min[__currentChannel]));
        }
    }
}

void Adc7kDisplay::resizeEvent(QResizeEvent *)
{
    update();
}

void Adc7kDisplay::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        __mouseStartX = event->x();
        __mouseStartY = event->y();
    } else if (event->buttons() & Qt::MiddleButton) {
        __offsetX = 0;
//        if ((__currentChannel >= 0) && (__currentChannel < MaxChannelNumber)) {
//            __offsetY[__currentChannel] = 0;
//            __offsetWY[__currentChannel] = 0;
//        }
        update();
    }
}

void Adc7kDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        __offsetX += event->x() - __mouseStartX;
//        if ((__currentChannel >= 0) && (__currentChannel < MaxChannelNumber)) {
//            __offsetY[__currentChannel] += event->y() - __mouseStartY;
//            __offsetWY[__currentChannel] = -__offsetY[__currentChannel];
//        }
        __mouseStartX = event->x();
        __mouseStartY = event->y();
        update();
    }
}

void Adc7kDisplay::onBoardThreadFinished()
{
    __continuous = true;
    __runStopButton->setChecked(false);
}

void Adc7kDisplay::onBoardSamplerLengthChanged(int length)
{
    for (int i = 0; i < MaxChannelNumber; ++i) {
        __channelData[i].resize(length);
    }
}

void Adc7kDisplay::single()
{
    __continuous = false;
    __runStopButton->setChecked(true);
}

void Adc7kDisplay::runstop(bool run)
{
    if (run) {
        __board->setSamplerContinuous(__continuous);
        __board->start();
        __singleButton->setEnabled(false);
        __runStopButton->setText("Stop");
    } else {
        __board->stop();
        __board->wait();
        __singleButton->setEnabled(true);
        __runStopButton->setText("Run");
    }
}

void Adc7kDisplay::updateData(int index)
{
    qint16 sample;
    quint32 *datap = (quint32 *)__board->channel(index)->data();

    __min[index * 2 + 0] = 0x1fff;
    __max[index * 2 + 0] = -0x2000;
    __min[index * 2 + 1] = 0x1fff;
    __max[index * 2 + 1] = -0x2000;
    for (int i = 0; i < qMin(__board->samplerLength(), (int)(__board->channel(index)->bufferSize() / 4)); ++i) {
        __channelData[index * 2 + 0][i] = sample = (*datap & 0x3fff) - 0x2000;
        __min[index * 2 + 0] = qMin(__min[index * 2 + 0], sample);
        __max[index * 2 + 0] = qMax(__max[index * 2 + 0], sample);
        __channelData[index * 2 + 1][i] = sample = ((*datap >> 14) & 0x3fff) - 0x2000;
        __min[index * 2 + 1] = qMin(__min[index * 2 + 1], sample);
        __max[index * 2 + 1] = qMax(__max[index * 2 + 1], sample);
        ++datap;
    }
    __med[index * 2 + 0] = (__max[index * 2 + 0] + __min[index * 2 + 0] + 1) / 2;
    __med[index * 2 + 1] = (__max[index * 2 + 1] + __min[index * 2 + 1] + 1) / 2;
    if (index == 3) {
        update();
    }
}

void Adc7kDisplay::autoscale()
{
    int part_height;
    int part_pos = 0;
    int count = channelStackCount();

    if (count) {
        part_height = height() / count;
        for (int i = 0; i < MaxChannelNumber; ++i) {
            if (channelStackIsActive(i)) {
                __scaleY[i] = part_height; __scaleY[i] /= (__max[i] - __min[i]);
                __pointsDiv[i] = 100.f / __scaleY[i];
                __offsetY[i] = __med[i] * __scaleY[i] - height() / 2 + part_height / 2 + part_height * part_pos++;
            }
        }
        __showLimitsMarker = true;
        __showLimitsMarkerTimer->start(5000);
        update();
    }
    if ((__currentChannel >= 0) && (__currentChannel < MaxChannelNumber)) {
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[__currentChannel]));
    }
}

void Adc7kDisplay::on_timeDivDial_valueChanged(int value)
{
    if (value > __timeDivDialValue) {
        __samplesDiv = inc125(__samplesDiv, 1000000);
        __timeDivValueLabel->setText(QString("%1").arg(__samplesDiv));
        __scaleX = 100.0f / __samplesDiv;
        update();
    } else if (value < __timeDivDialValue) {
        __samplesDiv = dec125(__samplesDiv);
        __timeDivValueLabel->setText(QString("%1").arg(__samplesDiv));
        __scaleX = 100.0f / __samplesDiv;
        update();
    }
    __timeDivDialValue = value;
}

void Adc7kDisplay::on_voltDivDial_valueChanged(int value)
{
    if ((__currentChannel >= 0) && (__currentChannel < MaxChannelNumber)) {
       if (value > __voltDivDialValue) {
           __pointsDiv[__currentChannel] = inc125(__pointsDiv[__currentChannel], 20000);
           __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[__currentChannel]));
           __scaleY[__currentChannel] = 100.0f / __pointsDiv[__currentChannel];
           update();
       } else if (value < __voltDivDialValue) {
           __pointsDiv[__currentChannel] = dec125(__pointsDiv[__currentChannel]);
           __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[__currentChannel]));
           __scaleY[__currentChannel] = 100.0f / __pointsDiv[__currentChannel];
           update();
       }
    }
    __voltDivDialValue = value;
}

void Adc7kDisplay::on_voltLevelDial_valueChanged(int value)
{
    if ((__currentChannel >= 0) && (__currentChannel < MaxChannelNumber)) {
       if (value > __voltLevelDialValue) {
           __offsetY[__currentChannel] -= 10;
           update();
       } else if (value < __voltLevelDialValue) {
           __offsetY[__currentChannel] += 10;
           update();
       }
    }
    __voltLevelDialValue = value;
}

int Adc7kDisplay::inc125(int input, int limit)
{
    int tmp;
    int man = input;
    int mul = 1;
    while (1) {
        if ((tmp = man / 10)) {
            man = tmp;
            mul *= 10;
        } else {
            man %= 10;
            break;
        }
    }
    if (man < 2) {
        man = 2;
    } else if (man < 5)  {
        man = 5;
    } else {
         mul *= 10;
         man = 1;
    }
    tmp = man * mul;
    if (tmp > limit) {
        tmp = limit;
    }
    return tmp;
}

int Adc7kDisplay::dec125(int input)
{
    int tmp;
    int man = input;
    int mul = 1;
    while (1) {
        if ((tmp = man / 10)) {
            man = tmp;
            mul *= 10;
        } else {
            man %= 10;
            break;
        }
    }
    if (man > 5) {
        man = 5;
    } else if (man > 2) {
        man = 2;
    } else if (man > 1) {
        man = 1;
    } else {
        mul /= 10;
        if (mul) {
            man = 5;
        } else {
            mul = 1;
        }
    }
    return man * mul;
}

void Adc7kDisplay::channelStackExclude(int index)
{
    index %= MaxChannelNumber;

    for (int i = 0; i < MaxChannelNumber; ++i) {
        if (__channelStack[i] == index) {
            __channelStack[i] = -1;
        }
    }
}

void Adc7kDisplay::channelStackTrim()
{
    int pos = 0;
    int stack[MaxChannelNumber];

    for (int i = 0; i < MaxChannelNumber; ++i) {
        stack[i] = -1;
    }

    for (int i = 0; i < MaxChannelNumber; ++i) {
        if (__channelStack[i] > -1) {
            stack[pos++] = __channelStack[i];
            channelStackExclude(__channelStack[i]);
        }
    }

    for (int i = 0; i < MaxChannelNumber; ++i) {
        __channelStack[i] = stack[i];
    }
}

void Adc7kDisplay::channelStackHoist(int index)
{
    index %= MaxChannelNumber;

    channelStackExclude(index);
    channelStackTrim();

    __channelStack[MaxChannelNumber - 1] = index;
}

int Adc7kDisplay::channelStackTop()
{
    int top = -1;

    for (int i = MaxChannelNumber - 1; i >= 0; --i) {
        if (__channelStack[i] > -1) {
            top = __channelStack[i];
            break;
        }
    }

    return top;
}

int Adc7kDisplay::channelStackCount()
{
    int count = 0;

    for (int i = 0; i < MaxChannelNumber; ++i) {
        if (__channelStack[i] > -1) {
            ++count;
        }
    }

    return count;
}

bool Adc7kDisplay::channelStackIsActive(int index)
{
    bool res = false;

    for (int i = 0; i < MaxChannelNumber; ++i) {
        if (__channelStack[i] == index) {
            res = true;
            break;
        }
    }

    return res;
}

void Adc7kDisplay::on_channel1CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(0);
        channelStackHoist(0);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(0);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel2CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
            __channelsComboBox->setCurrentIndex(1);
        channelStackHoist(1);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(1);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel3CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(2);
        channelStackHoist(2);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(2);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel4CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(3);
        channelStackHoist(3);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(3);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel5CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(4);
        channelStackHoist(4);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(4);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel6CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(5);
        channelStackHoist(5);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(5);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel7CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(6);
        channelStackHoist(6);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(6);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channel8CheckBox_stateChanged(int state)
{
    if (state == Qt::Checked) {
        __channelsComboBox->setCurrentIndex(7);
        channelStackHoist(7);
        update();
    } else if (state == Qt::Unchecked) {
        channelStackExclude(7);
        __channelsComboBox->setCurrentIndex(channelStackTop());
        update();
    }
}

void Adc7kDisplay::on_channelsComboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        __channel1CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[0]));
        break;
    case 1:
        __channel2CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[1]));
        break;
    case 2:
        __channel3CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[2]));
        break;
    case 3:
        __channel4CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[3]));
        break;
    case 4:
        __channel5CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[4]));
        break;
    case 5:
        __channel6CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[5]));
        break;
    case 6:
        __channel7CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[6]));
        break;
    case 7:
        __channel8CheckBox->setCheckState(Qt::Checked);
        __voltDivValueLabel->setText(QString("%1").arg(__pointsDiv[7]));
        break;
    default:
        __voltDivValueLabel->setText(QString(""));
        break;
    }
    __currentChannel = index;
    __showLimitsMarker = true;
    __showLimitsMarkerTimer->start(5000);
    update();
}

void Adc7kDisplay::readSettings()
{
    QSettings settings("Polygator", "ADC7K");

    settings.beginGroup("Adc7kDisplay");
    __samplesDiv = settings.value("samplesdiv", 100).toInt();
    if (settings.value("ch1en", true).toBool()) {
        __channelStack[7] = 0;
    }
    if (settings.value("ch2en", true).toBool()) {
        __channelStack[6] = 1;
    }
    if (settings.value("ch3en", true).toBool()) {
        __channelStack[5] = 2;
    }
    if (settings.value("ch4en", true).toBool()) {
        __channelStack[4] = 3;
    }
    if (settings.value("ch5en", true).toBool()) {
        __channelStack[3] = 4;
    }
    if (settings.value("ch6en", true).toBool()) {
        __channelStack[2] = 5;
    }
    if (settings.value("ch7en", true).toBool()) {
        __channelStack[1] = 6;
    }
    if (settings.value("ch8en", true).toBool()) {
        __channelStack[0] = 7;
    }
    settings.endGroup();
}

void Adc7kDisplay::writeSettings()
{
    QSettings settings("Polygator", "ADC7K");

    settings.beginGroup("Adc7kDisplay");
    settings.setValue("samplesdiv", __samplesDiv);
    settings.setValue("ch1en", __channel1CheckBox->isChecked());
    settings.setValue("ch2en", __channel2CheckBox->isChecked());
    settings.setValue("ch3en", __channel3CheckBox->isChecked());
    settings.setValue("ch4en", __channel4CheckBox->isChecked());
    settings.setValue("ch5en", __channel5CheckBox->isChecked());
    settings.setValue("ch6en", __channel6CheckBox->isChecked());
    settings.setValue("ch7en", __channel7CheckBox->isChecked());
    settings.setValue("ch8en", __channel8CheckBox->isChecked());
    settings.endGroup();
}

void Adc7kDisplay::on_showLimitsMarkerTimer_timeout()
{
    __showLimitsMarker = false;
    __showLimitsMarkerTimer->stop();
    update();
}
