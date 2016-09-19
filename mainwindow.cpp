#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    boardComboBox = new QComboBox();
    connect(boardComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &MainWindow::on_boardComboBox_currentIndexChanged);
    ui->mainToolBar->addWidget(boardComboBox);

    ui->boardInfoGroupBox->setEnabled(false);
    ui->boardInfoGroupBox->setVisible(false);
    ui->boardInfoToolBox->setCurrentIndex(0);
    ui->actionBoardInfo->setEnabled(false);

    if (Adc7kBoard::searchBoards()) {
        if (Adc7kBoard::boardList.size()) {
            for (int i = 0; i < Adc7kBoard::boardList.size(); ++i) {
                boardComboBox->addItem(Adc7kBoard::boardList[i]->name());
            }
            fillBoardInfo(Adc7kBoard::boardList[0]);
            ui->boardInfoGroupBox->setEnabled(true);
            ui->boardInfoGroupBox->setVisible(ui->actionBoardInfo->isChecked());
            ui->actionBoardInfo->setEnabled(true);
        } else {
            ui->statusBar->showMessage("ADC7K boards not found");
        }
    } else {
        ui->statusBar->showMessage("ADC7K subsystem not found");
    }
    readSettings();
}

MainWindow::~MainWindow()
{
    Adc7kBoard::releaseBoards();

    delete boardComboBox;
    delete ui;
}

void MainWindow::on_actionBoardInfo_toggled(bool arg)
{
    ui->boardInfoGroupBox->setVisible(arg);
}

void MainWindow::fillBoardInfo(Adc7kBoard *board)
{
    ui->typeValueLabel->setText(board->type());
    ui->nameValueLabel->setText(board->name());

    ui->samplingRateValueLabel->setText(QString("%1 Hz").arg(board->samplingRate()));
    ui->samplerLengthSpinBox->setValue(board->samplerLength());
    ui->samplerLengthSpinBox->setMinimum(1);
    ui->samplerLengthSpinBox->setMaximum(board->samplerMaxLength());

    ui->channels12BufferAddressValueLabel->setText(QString("0x%1").arg(board->channel("12")->bufferAddress(), 8, 16));
    ui->channels12BufferSizeValueLabel->setText(QString("%1 KiB").arg(board->channel("12")->bufferSize() / 1024));

    ui->channels34BufferAddressValueLabel->setText(QString("0x%1").arg(board->channel("34")->bufferAddress(), 8, 16));
    ui->channels34BufferSizeValueLabel->setText(QString("%1 KiB").arg(board->channel("34")->bufferSize() / 1024));

    ui->channels56BufferAddressValueLabel->setText(QString("0x%1").arg(board->channel("56")->bufferAddress(), 8, 16));
    ui->channels56BufferSizeValueLabel->setText(QString("%1 KiB").arg(board->channel("56")->bufferSize() / 1024));

    ui->channels78BufferAddressValueLabel->setText(QString("0x%1").arg(board->channel("78")->bufferAddress(), 8, 16));
    ui->channels78BufferSizeValueLabel->setText(QString("%1 KiB").arg(board->channel("78")->bufferSize() / 1024));

    ui->adc7kDisplayWidget->setBoard(board);
}

void MainWindow::on_boardComboBox_currentIndexChanged(int index)
{
    fillBoardInfo(Adc7kBoard::boardList[index]);
}

void MainWindow::on_samplerLengthSpinBox_valueChanged(int length)
{
    Adc7kBoard::boardList[boardComboBox->currentIndex()]->setSamplerLength(length);
}

void MainWindow::on_samplerDividerSpinBox_valueChanged(int value)
{
    Adc7kBoard::boardList[boardComboBox->currentIndex()]->setSamplerDivider(value);
}

void MainWindow::on_actionQuit_triggered()
{
    close();
}

void MainWindow::on_actionFull_Screen_toggled(bool fs)
{
    Qt::WindowStates state = windowState();

    if (fs) {
        state |= Qt::WindowFullScreen;
    } else {
        state &= ~Qt::WindowFullScreen;
    }
    ui->statusBar->setVisible(!fs);
    ui->mainToolBar->setVisible(!fs);
    setWindowState(state);
}

void MainWindow::readSettings()
{
    QSettings settings("Polygator", "ADC7K");

    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(800, 600)).toSize());
    move(settings.value("pos", QPoint(200, 200)).toPoint());
    ui->actionBoardInfo->setChecked(settings.value("boardinfo", true).toBool());
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings("Polygator", "ADC7K");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("boardinfo", ui->actionBoardInfo->isChecked());
    settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
    event->accept();
}

void MainWindow::on_adcResetPushButton_clicked()
{
    Adc7kBoard::boardList[boardComboBox->currentIndex()]->adcReset();
}

void MainWindow::on_adcWritePushButton_clicked()
{
    Adc7kBoard::boardList[boardComboBox->currentIndex()]->adcWrite(ui->adcAddressSpinBox->value(), ui->adcDataSpinBox->value());
}
