#include <math.h>
#include <time.h>

#include <QApplication>

#include "connectiondialog.h"
#include "locationserverdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "gpsutilityapplication.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusLabel = new QLabel();
    statusLabel->setText("No GPS Connected");
    statusBar()->addWidget(statusLabel);

    locationWidget = new LocationWidget();
    signalMeterWidget = new SignalMeter();

    addDockWidget(Qt::LeftDockWidgetArea, locationWidget);
    addDockWidget(Qt::BottomDockWidgetArea, signalMeterWidget);

    locationWidget->show();
    signalMeterWidget->show();

    mapDialog = new MapDialog();
    connect(mapDialog, SIGNAL(loadStarted()), this, SLOT(mapLoadStarted()));
    connect(mapDialog, SIGNAL(loadFinished(bool)), this, SLOT(mapLoadFinished(bool)));
    ui->mdiArea->addSubWindow(mapDialog);

    parser = new NMEAParser();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_actionOpen_Google_Maps_triggered()
{
}

void MainWindow::on_actionConnect_triggered()
{
    ConnectionDialog *dlg = new ConnectionDialog();
    int ret = dlg->exec();

    if (ret) {
	// what happened here?
        emit openSerialDevice(dlg->getSerialPort().toAscii());
    }
}

void MainWindow::on_actionDisconnect_triggered()
{
    emit closeSerialDevice();
}

void MainWindow::gpsConnected()
{
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
}

void MainWindow::gpsDisconnected()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    statusLabel->setText("No GPS Connected");
}

void MainWindow::mapLoadStarted()
{
    statusBar()->showMessage("Contacting Google Maps API server...", 30);
}

void MainWindow::mapLoadFinished(bool finished)
{
    if (finished) {
        statusBar()->clearMessage();
    }
    else {
        statusBar()->showMessage("Could not load map data");
    }
}

void MainWindow::on_actionShare_triggered()
{
    LocationServerDialog *dlg = new LocationServerDialog();
    dlg->show();
    dlg->exec();
}

void MainWindow::setLocation(double latitude, double longitude, double altitude, double heading)
{
    double deg = 0, min = 0, sec = 0;

    min = (latitude - floor(latitude)) * 60;
    sec = (min - floor(min)) * 60;

    locationWidget->setLatitude(QString("%1%2 %3' %4\"")
                             .arg(floor(latitude), 2, 'f', 0)
                             .arg(QChar(0x00B0))
                             .arg(floor(min))
                             .arg(sec, 2, 'f', 2, QChar('0')));

    min = (longitude - floor(longitude)) * 60;
    sec = (min - floor(min)) * 60;

    locationWidget->setLongitude(QString("%1%2 %3' %4\"")
                              .arg(floor(longitude), 2, 'f', 0)
                              .arg(QChar(0x00B0))
                              .arg(floor(min))
                              .arg(sec, 2, 'f', 2, QChar('0')));

    locationWidget->setAltitude(QString("%1 M").arg(altitude));
}

void MainWindow::setStatusBarText(const QString text)
{
    statusLabel->setText(text);
}

void MainWindow::setGpsTime(int hours, int minutes, double seconds)
{
    locationWidget->setTime(QString("%1:%2:%3")
                            .arg(nmeaParser->utcHours(), 2, 10, QChar('0'))
                            .arg(nmeaParser->utcMinutes(), 2, 10, QChar('0'))
                            .arg(nmeaParser->utcSeconds(), 2, 'g', -1, QChar('0')));
}
