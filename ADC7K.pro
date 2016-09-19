#-------------------------------------------------
#
# Project created by QtCreator 2016-07-11T16:59:19
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ADC7K
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    adc7kboard.cpp \
    adc7kchannel.cpp \
    adc7kdisplay.cpp

HEADERS  += mainwindow.h \
    adc7kboard.h \
    adc7kchannel.h \
    adc7kdisplay.h

FORMS    += mainwindow.ui
