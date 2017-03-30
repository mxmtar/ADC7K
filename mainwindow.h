#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QComboBox>
#include <QList>
#include <QMainWindow>

#include "adc7kboard.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void on_boardComboBox_currentIndexChanged(int index);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_actionBoardInfo_toggled(bool checked);

    void on_samplerLengthSpinBox_valueChanged(int length);

    void on_samplerDividerSpinBox_valueChanged(int arg1);

    void on_actionQuit_triggered();

    void on_actionFull_Screen_toggled(bool arg1);

    void on_adcResetPushButton_clicked();

    void on_adcWritePushButton_clicked();

    void on_registerReadPushButton_clicked();

    void on_registerWritePushButton_clicked();

    void on_registerAddressSpinBox_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;

    QComboBox *boardComboBox;

    void fillBoardInfo(Adc7kBoard *board);

    void readSettings();
    void writeSettings();
};

#endif // MAINWINDOW_H
