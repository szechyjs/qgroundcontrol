/*=====================================================================

QGroundControl Open Source Ground Control Station

(c) 2009 - 2011 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>

This file is part of the QGROUNDCONTROL project

    QGROUNDCONTROL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QGROUNDCONTROL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QGROUNDCONTROL. If not, see <http://www.gnu.org/licenses/>.

======================================================================*/

#include <QToolButton>
#include <QLabel>
#include "QGCToolBar.h"
#include "UASManager.h"
#include "MainWindow.h"

QGCToolBar::QGCToolBar(QWidget *parent) :
    QToolBar(parent),
    toggleLoggingAction(NULL),
    logReplayAction(NULL),
    mav(NULL)
{
    setObjectName("QGC_TOOLBAR");

    toggleLoggingAction = new QAction(QIcon(":"), "Logging", this);
    toggleLoggingAction->setCheckable(true);
    logReplayAction = new QAction(QIcon(":"), "Replay", this);
    logReplayAction->setCheckable(true);

    addSeparator();

    addAction(toggleLoggingAction);
    addAction(logReplayAction);

    // CREATE TOOLBAR ITEMS
    // Add internal actions
    // Add MAV widget
    symbolButton = new QToolButton(this);
    toolBarNameLabel = new QLabel("------", this);
    toolBarModeLabel = new QLabel("------", this);
    toolBarModeLabel->setStyleSheet("QLabel { margin: 0px 4px; font: 14px; color: #3C7B9E; }");
    toolBarStateLabel = new QLabel("------", this);
    toolBarStateLabel->setStyleSheet("QLabel { margin: 0px 4px; font: 14px; color: #FEC654; }");
    toolBarWpLabel = new QLabel("WP--", this);
    toolBarWpLabel->setStyleSheet("QLabel { margin: 0px 4px; font: 18px; color: #3C7B9E; }");
    toolBarDistLabel = new QLabel("--- ---- m", this);
    toolBarMessageLabel = new QLabel("No system messages.", this);
    toolBarMessageLabel->setStyleSheet("QLabel { margin: 0px 4px; font: 12px; font-style: italic; color: #3C7B9E; }");
    toolBarBatteryBar = new QProgressBar(this);
    toolBarBatteryBar->setStyleSheet("QProgressBar:horizontal { margin: 0px 4px 0px 0px; border: 1px solid #4A4A4F; border-radius: 4px; text-align: center; padding: 2px; color: #111111; background-color: #111118; height: 10px; } QProgressBar:horizontal QLabel { font-size: 9px; color: #111111; } QProgressBar::chunk { background-color: green; }");
    toolBarBatteryBar->setMinimum(0);
    toolBarBatteryBar->setMaximum(100);
    toolBarBatteryBar->setMaximumWidth(200);
    toolBarBatteryVoltageLabel = new QLabel("xx.x V");
    toolBarBatteryVoltageLabel->setStyleSheet(QString("QLabel { margin: 0px 2px 0px 4px; font: 14px; color: %1; }").arg(QColor(Qt::green).name()));
    //symbolButton->setIcon(":");
    symbolButton->setStyleSheet("QWidget { background-color: #050508; color: #DDDDDF; background-clip: border; } QToolButton { font-weight: bold; font-size: 12px; border: 0px solid #999999; border-radius: 5px; min-width:22px; max-width: 22px; min-height: 22px; max-height: 22px; padding: 0px; margin: 0px 0px 0px 20px; background-color: none; }");
    addWidget(symbolButton);
    addWidget(toolBarNameLabel);
    addWidget(toolBarModeLabel);
    addWidget(toolBarStateLabel);
    addWidget(toolBarBatteryBar);
    addWidget(toolBarBatteryVoltageLabel);
    addWidget(toolBarWpLabel);
    addWidget(toolBarDistLabel);
    addWidget(toolBarMessageLabel);
    //addWidget(new QSpacerItem(20, 0, QSizePolicy::Expanding));

    // DONE INITIALIZING BUTTONS

    setActiveUAS(UASManager::instance()->getActiveUAS());
    connect(UASManager::instance(), SIGNAL(activeUASSet(UASInterface*)), this, SLOT(setActiveUAS(UASInterface*)));
}

void QGCToolBar::setLogPlayer(QGCMAVLinkLogPlayer* player)
{
    connect(toggleLoggingAction, SIGNAL(triggered(bool)), player, SLOT(playPause(bool)));
    connect(logReplayAction, SIGNAL(triggered(bool)), this, SLOT(logging(bool)));
}

void QGCToolBar::logging(bool enabled)
{
    // Stop logging in any case
    MainWindow::instance()->getMAVLink()->enableLogging(false);
    if (enabled)
    {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Specify MAVLink log file name"), QDesktopServices::storageLocation(QDesktopServices::DesktopLocation), tr("MAVLink Logfile (*.mavlink *.log *.bin);;"));

        if (!fileName.endsWith(".mavlink"))
        {
            fileName.append(".mavlink");
        }

        QFileInfo file(fileName);
        if (file.exists() && !file.isWritable())
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Critical);
            msgBox.setText(tr("The selected logfile is not writable"));
            msgBox.setInformativeText(tr("Please make sure that the file %1 is writable or select a different file").arg(fileName));
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.exec();
        }
        else
        {
            MainWindow::instance()->getMAVLink()->setLogfileName(fileName);
            MainWindow::instance()->getMAVLink()->enableLogging(true);
        }
    }
}

void QGCToolBar::addPerspectiveChangeAction(QAction* action)
{
    insertAction(toggleLoggingAction, action);
}

void QGCToolBar::setActiveUAS(UASInterface* active)
{
    // Do nothing if system is the same or NULL
    if ((active == NULL) || mav == active) return;

    if (mav)
    {
        // Disconnect old system
        disconnect(mav, SIGNAL(statusChanged(UASInterface*,QString,QString)), this, SLOT(updateState(UASInterface*,QString,QString)));
        disconnect(mav, SIGNAL(modeChanged(int,QString,QString)), this, SLOT(updateMode(int,QString,QString)));
        disconnect(mav, SIGNAL(nameChanged(QString)), this, SLOT(updateName(QString)));
        disconnect(mav, SIGNAL(systemTypeSet(UASInterface*,uint)), this, SLOT(setSystemType(UASInterface*,uint)));
        disconnect(mav, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(receiveTextMessage(int,int,int,QString)));
        disconnect(mav, SIGNAL(batteryChanged(UASInterface*,double,double,int)), this, SLOT(updateBatteryRemaining(UASInterface*,double,double,int)));
        if (mav->getWaypointManager())
        {
            disconnect(mav->getWaypointManager(), SIGNAL(currentWaypointChanged(int)), this, SLOT(updateCurrentWaypoint(int)));
            disconnect(mav->getWaypointManager(), SIGNAL(waypointDistanceChanged(double)), this, SLOT(updateWaypointDistance(double)));
        }
    }

    // Connect new system
    mav = active;
    connect(active, SIGNAL(statusChanged(UASInterface*,QString,QString)), this, SLOT(updateState(UASInterface*, QString,QString)));
    connect(active, SIGNAL(modeChanged(int,QString,QString)), this, SLOT(updateMode(int,QString,QString)));
    connect(active, SIGNAL(nameChanged(QString)), this, SLOT(updateName(QString)));
    connect(active, SIGNAL(systemTypeSet(UASInterface*,uint)), this, SLOT(setSystemType(UASInterface*,uint)));
    connect(active, SIGNAL(textMessageReceived(int,int,int,QString)), this, SLOT(receiveTextMessage(int,int,int,QString)));
    connect(active, SIGNAL(batteryChanged(UASInterface*,double,double,int)), this, SLOT(updateBatteryRemaining(UASInterface*,double,double,int)));
    if (active->getWaypointManager())
    {
        connect(active->getWaypointManager(), SIGNAL(currentWaypointChanged(quint16)), this, SLOT(updateCurrentWaypoint(quint16)));
        connect(active->getWaypointManager(), SIGNAL(waypointDistanceChanged(double)), this, SLOT(updateWaypointDistance(double)));
    }

    // Update all values once
    toolBarNameLabel->setText(mav->getUASName());
    toolBarNameLabel->setStyleSheet(QString("QLabel { font: bold 16px; color: %1; }").arg(mav->getColor().name()));
    symbolButton->setStyleSheet(QString("QWidget { background-color: %1; color: #DDDDDF; background-clip: border; } QToolButton { font-weight: bold; font-size: 12px; border: 0px solid #999999; border-radius: 5px; min-width:22px; max-width: 22px; min-height: 22px; max-height: 22px; padding: 0px; margin: 0px 4px 0px 20px; background-color: none; }").arg(mav->getColor().name()));
//    toolBarModeLabel->setStyleSheet("QLabel { font: 16px; color: #3C7B9E; }");
//    toolBarStateLabel->setStyleSheet("QLabel { font: 16px; color: #FEC654; }");
    toolBarModeLabel->setText(mav->getShortMode());
    toolBarStateLabel->setText(mav->getShortState());
    setSystemType(mav, mav->getSystemType());
}

void QGCToolBar::createCustomWidgets()
{

}

void QGCToolBar::updateWaypointDistance(double distance)
{
    toolBarDistLabel->setText(tr("%1 m").arg(distance, 6, 'f', 2, '0'));
}

void QGCToolBar::updateCurrentWaypoint(quint16 id)
{
    toolBarWpLabel->setText(tr("WP%1").arg(id));
}

void QGCToolBar::updateBatteryRemaining(UASInterface* uas, double voltage, double percent, int seconds)
{
    Q_UNUSED(uas);
    Q_UNUSED(seconds);
    toolBarBatteryBar->setValue(percent);
    toolBarBatteryVoltageLabel->setText(tr("%1 V").arg(voltage, 4, 'f', 1, ' '));
}

void QGCToolBar::updateState(UASInterface* system, QString name, QString description)
{
    Q_UNUSED(system);
    Q_UNUSED(description);
    toolBarStateLabel->setText(tr("%1").arg(name));
}

void QGCToolBar::updateMode(int system, QString name, QString description)
{
    Q_UNUSED(system);
    Q_UNUSED(description);
    toolBarModeLabel->setText(tr("%1").arg(name));
}

void QGCToolBar::updateName(const QString& name)
{
    toolBarNameLabel->setText(name);
}

/**
 * The current system type is represented through the system icon.
 *
 * @param uas Source system, has to be the same as this->uas
 * @param systemType type ID, following the MAVLink system type conventions
 * @see http://pixhawk.ethz.ch/software/mavlink
 */
void QGCToolBar::setSystemType(UASInterface* uas, unsigned int systemType)
{
    Q_UNUSED(uas);
        // Set matching icon
        switch (systemType) {
        case 0:
            symbolButton->setIcon(QIcon(":/images/mavs/generic.svg"));
            break;
        case 1:
            symbolButton->setIcon(QIcon(":/images/mavs/fixed-wing.svg"));
            break;
        case 2:
            symbolButton->setIcon(QIcon(":/images/mavs/quadrotor.svg"));
            break;
        case 3:
            symbolButton->setIcon(QIcon(":/images/mavs/coaxial.svg"));
            break;
        case 4:
            symbolButton->setIcon(QIcon(":/images/mavs/helicopter.svg"));
            break;
        case 5:
            symbolButton->setIcon(QIcon(":/images/mavs/unknown.svg"));
            break;
        default:
            symbolButton->setIcon(QIcon(":/images/mavs/unknown.svg"));
            break;
        }
}

void QGCToolBar::receiveTextMessage(int uasid, int componentid, int severity, QString text)
{
    Q_UNUSED(uasid);
    Q_UNUSED(componentid);
    Q_UNUSED(severity);
    toolBarMessageLabel->setText(text);
}

QGCToolBar::~QGCToolBar()
{
    delete toggleLoggingAction;
    delete logReplayAction;
}
