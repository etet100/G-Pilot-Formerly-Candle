#include "frmmain.h"

/*
to be refactored/replaced by signals and slots

updateControlsState()
m_fileProcessedCommandIndex
m_currentModel
m_settings OK
m_timerStateQuery
m_deviceStatuses
m_toolDrawer
m_codeDrawer
m_lastDrawnLineIndex
m_currentDrawer
m_fileCommandIndex
m_updateParserStatus
m_connection
m_storedVars
m_heightMapMode
m_taskBarProgress
m_senderErrorBox
m_machineBoundsDrawer
m_absoluteCoordinates

completeTransfer();
restoreParserState();
updateOverride(ui->slbFeedOverride, ov.cap(1).toInt(), '\x91');
parser
list ???
jogContinuous();
storeOffsetsVars(response)
setupCoordsTextboxes
updateHeightMapInterpolationDrawer
sendNextFileCommands

ui->slbFeedOverride
ui->slbSpindleOverride
ui->slbSpindle
ui->slbRapidOverride->isChecked() ? ui->slbRapidOverride
ui->cmdFlood
ui->glwVisualizer
ui->txtConsole
ui->chkKeyboardControl
ui->tblHeightMap
ui->chkAutoScroll
ui->tblProgram
ui->cmdFileAbort

*/

void frmMain::onConnectionLineReceived(QString data)
{
    assert(QThread::currentThread() == QCoreApplication::instance()->thread());

    if (m_communicator->m_reseting) {
        if (!m_communicator->dataIsReset(data)) return;
        else {
            m_communicator->m_reseting = false;
            m_timerStateQuery.setInterval(m_settings->queryStateTime());
        }
    }

    // Status response
    if (data[0] == '<') {
        DeviceState state = DeviceUnknown;

        m_communicator->m_statusReceived = true;

        // Update machine coordinates
        static QRegExp mpx("MPos:([^,]*),([^,]*),([^,^>^|]*)");
        if (mpx.indexIn(data) != -1) {
            QVector3D pos(
                mpx.cap(1).toDouble(),
                mpx.cap(2).toDouble(),
                mpx.cap(3).toDouble()
                );
            m_communicator->m_machinePos = pos;
            emit machinePosChanged(pos);
        }

        // Status
        static QRegExp stx("<([^,^>^|]*)");
        if (stx.indexIn(data) != -1) {
            state = m_deviceStatuses.key(stx.cap(1), DeviceUnknown);

            // Update status
            if (state != m_communicator->m_deviceState) {
                emit deviceStateChanged(state);
            }

            emit deviceStateReceived(state);

            // Update controls
            // moved to deviceStateReceived handler
            // ui->cmdCheck->setEnabled(state != DeviceRun && (m_communicator->m_senderState == SenderStopped));
            // ui->cmdCheck->setChecked(state == DeviceCheck);
            // ui->cmdHold->setChecked(state == DeviceHold0 || state == DeviceHold1 || state == DeviceQueue);
            // ui->cmdSpindle->setEnabled(state == DeviceHold0 || ((m_communicator->m_senderState != SenderTransferring) &&
            //                                                     (m_communicator->m_senderState != SenderStopping)));

            // // Update "elapsed time" timer
            // // moved to deviceStateReceived handler
            // if ((m_communicator->m_senderState == SenderTransferring) || (m_communicator->m_senderState == SenderStopping)) {
            //     QTime time(0, 0, 0);
            //     int elapsed = m_startTime.elapsed();
            //     ui->glwVisualizer->setSpendTime(time.addMSecs(elapsed));
            // }

            // Test for job complete
            if ((m_communicator->m_senderState == SenderStopping) &&
                ((state == DeviceIdle && m_communicator->m_deviceState == DeviceRun) || state == DeviceCheck))
            {
                completeTransfer();
            }

            // Abort
            static double x = sNan;
            static double y = sNan;
            static double z = sNan;

            if (m_communicator->m_aborting) {
                switch (state) {
                case DeviceIdle: // Idle
                    if ((m_communicator->m_senderState == SenderStopped) && m_communicator->m_resetCompleted) {
                        m_communicator->m_aborting = false;
                        restoreParserState();
                        m_communicator->restoreOffsets();
                        return;
                    }
                    break;
                case DeviceHold0: // Hold
                case DeviceHold1:
                case DeviceQueue:
                    if (!m_communicator->m_reseting && m_communicator->compareCoordinates(x, y, z)) {
                        x = sNan;
                        y = sNan;
                        z = sNan;
                        grblReset();
                    } else {
                        const QVector3D pos = m_communicator->m_machinePos;
                        x = pos.x();
                        y = pos.y();
                        z = pos.z();
                    }
                    break;
                case DeviceUnknown:
                case DeviceAlarm:
                case DeviceRun:
                case DeviceHome:
                case DeviceCheck:
                case DeviceDoor0:
                case DeviceDoor1:
                case DeviceDoor2:
                case DeviceDoor3:
                case DeviceJog:
                case DeviceSleep:
                    break;
                }
            }
        }

        // Store work offset
        static QVector3D workOffset;
        static QRegExp wpx("WCO:([^,]*),([^,]*),([^,^>^|]*)");

        if (wpx.indexIn(data) != -1)
        {
            workOffset = QVector3D(wpx.cap(1).toDouble(), wpx.cap(2).toDouble(), wpx.cap(3).toDouble());
        }

        // Update work coordinates
        QVector3D pos(
            m_communicator->m_machinePos.x() - workOffset.x(),
            m_communicator->m_machinePos.y() - workOffset.y(),
            m_communicator->m_machinePos.z() - workOffset.z()
            );
        m_communicator->m_workPos = pos;
        emit workPosChanged(pos);

        // Update tool position
        QVector3D toolPosition;
        if (!(state == DeviceCheck && m_fileProcessedCommandIndex < m_currentModel->rowCount() - 1)) {
            toolPosition = m_communicator->m_machinePos;
            m_toolDrawer.setToolPosition(m_codeDrawer->getIgnoreZ() ? QVector3D(toolPosition.x(), toolPosition.y(), 0) : toolPosition);
        }

        // Toolpath shadowing
        if (((m_communicator->m_senderState == SenderTransferring) || (m_communicator->m_senderState == SenderStopping)
             || (m_communicator->m_senderState == SenderPausing) || (m_communicator->m_senderState == SenderPausing2) || (m_communicator->m_senderState == SenderPaused)) && state != DeviceCheck) {
            GcodeViewParse *parser = m_currentDrawer->viewParser();

            bool toolOntoolpath = false;

            QList<int> drawnLines;
            QList<LineSegment*> list = parser->getLineSegmentList();

            for (int i = m_lastDrawnLineIndex; i < list.count()
                                               && list.at(i)->getLineNumber()
                                                      <= (m_currentModel->data(m_currentModel->index(m_fileProcessedCommandIndex, 4)).toInt() + 1); i++) {
                if (list.at(i)->contains(toolPosition)) {
                    toolOntoolpath = true;
                    m_lastDrawnLineIndex = i;
                    break;
                }
                drawnLines << i;
            }

            if (toolOntoolpath) {
                foreach (int i, drawnLines) {
                    list.at(i)->setDrawn(true);
                }
                if (!drawnLines.isEmpty()) m_currentDrawer->update(drawnLines);
            }
        }

        // Get overrides
        static QRegExp ov("Ov:([^,]*),([^,]*),([^,^>^|]*)");
        if (ov.indexIn(data) != -1)
        {
            updateOverride(ui->slbFeedOverride, ov.cap(1).toInt(), '\x91');
            updateOverride(ui->slbSpindleOverride, ov.cap(3).toInt(), '\x9a');

            int rapid = ov.cap(2).toInt();
            ui->slbRapidOverride->setCurrentValue(rapid);

            int target = ui->slbRapidOverride->isChecked() ? ui->slbRapidOverride->value() : 100;

            if (rapid != target) switch (target) {
                case 25:
                    m_connection->sendByteArray(QByteArray(1, char(GRBL_LIVE_RAPID_FULL_RATE)));
                    break;
                case 50:
                    m_connection->sendByteArray(QByteArray(1, char(GRBL_LIVE_RAPID_HALF_RATE)));
                    break;
                case 100:
                    m_connection->sendByteArray(QByteArray(1, char(GRBL_LIVE_RAPID_QUARTER_RATE)));
                    break;
                }

            // Update pins state
            QString pinState;
            static QRegExp pn("Pn:([^|^>]*)");
            if (pn.indexIn(data) != -1) {
                pinState.append(QString(tr("PS: %1")).arg(pn.cap(1)));
            }

            // Process spindle state
            static QRegExp as("A:([^,^>^|]+)");
            if (as.indexIn(data) != -1) {
                QString q = as.cap(1);
                m_communicator->m_spindleCW = q.contains("S");
                if (q.contains("S") || q.contains("C")) {
                    emit spindleStateReceived(true);
                    // to spindleStateReceived handler
                    // m_timerToolAnimation.start(25, this);
                    // ui->cmdSpindle->setChecked(true);
                } else {
                    emit spindleStateReceived(false);
                    // to spindleStateReceived handler
                    // m_timerToolAnimation.stop();
                    // ui->cmdSpindle->setChecked(false);
                }
                ui->cmdFlood->setChecked(q.contains("F"));

                if (!pinState.isEmpty()) pinState.append(" / ");
                pinState.append(QString(tr("AS: %1")).arg(as.cap(1)));
            } else {
                emit spindleStateReceived(false);
                // to spindleStateReceived handler
                // m_timerToolAnimation.stop();
                // ui->cmdSpindle->setChecked(false);
            }
            ui->glwVisualizer->setPinState(pinState);
        }

        // Get feed/spindle values
        static QRegExp fs("FS:([^,]*),([^,^|^>]*)");
        if (fs.indexIn(data) != -1) {
            ui->glwVisualizer->setSpeedState((QString(tr("F/S: %1 / %2")).arg(fs.cap(1)).arg(fs.cap(2))));
        }

        // Store device state
        m_communicator->setDeviceState(state);

        // Update continuous jog
        jogContinuous();

        // Emit status signal
        emit statusReceived(data);

        // Command response
    } else if (data.length() > 0) {

        if (m_communicator->m_commands.length() > 0 && !m_communicator->dataIsFloating(data)
            && !(m_communicator->m_commands[0].command != "[CTRL+X]" && m_communicator->dataIsReset(data))) {

            static QString response; // Full response string

            if ((m_communicator->m_commands[0].command != "[CTRL+X]" && m_communicator->dataIsEnd(data))
                || (m_communicator->m_commands[0].command == "[CTRL+X]" && m_communicator->dataIsReset(data))) {

                response.append(data);

                // Take command from buffer
                CommandAttributes ca = m_communicator->m_commands.takeFirst();
                QTextBlock tb = ui->txtConsole->document()->findBlockByNumber(ca.consoleIndex);
                QTextCursor tc(tb);

                QString uncomment = GcodePreprocessorUtils::removeComment(ca.command).toUpper();

                // Store current coordinate system
                if (uncomment == "$G") {
                    static QRegExp g("G5[4-9]");
                    if (g.indexIn(response) != -1) {
                        m_storedVars.setCS(g.cap(0));
                        m_machineBoundsDrawer.setOffset(QPointF(m_communicator->toMetric(m_storedVars.x()), m_communicator->toMetric(m_storedVars.y())) +
                                                        QPointF(m_communicator->toMetric(m_storedVars.G92x()), m_communicator->toMetric(m_storedVars.G92y())));
                    }
                    static QRegExp t("T(\\d+)(?!\\d)");
                    if (t.indexIn(response) != -1) {
                        m_storedVars.setTool(g.cap(1).toInt());
                    }
                }

                // TODO: Store firmware version, features, buffer size on $I command
                // [VER:1.1d.20161014:Some string]
                // [OPT:VL,15,128]

                // Restore absolute/relative coordinate system after jog
                if (uncomment == "$G" && ca.tableIndex == -2) {
                    if (ui->chkKeyboardControl->isChecked()) m_absoluteCoordinates = response.contains("G90");
                    else if (response.contains("G90")) m_communicator->sendCommand("G90", -1, m_settings->showUICommands());
                }

                // Process parser status
                if (uncomment == "$G" && ca.tableIndex == -3) {
                    // Update status in visualizer window
                    ui->glwVisualizer->setParserStatus(response.left(response.indexOf("; ")));

                    // Store parser status
                    if ((m_communicator->m_senderState == SenderTransferring) || (m_communicator->m_senderState == SenderStopping)) storeParserState();

                    // Spindle speed
                    QRegExp rx(".*S([\\d\\.]+)");
                    if (rx.indexIn(response) != -1) {
                        double speed = rx.cap(1).toDouble();
                        ui->slbSpindle->setCurrentValue(speed);
                    }

                    m_updateParserStatus = true;
                }

                // Offsets
                if (uncomment == "$#") storeOffsetsVars(response);

                // Settings response
                if (uncomment == "$$" && ca.tableIndex == -2) {
                    static QRegExp gs("\\$(\\d+)\\=([^;]+)\\; ");
                    QMap<int, double> set;
                    int p = 0;
                    while ((p = gs.indexIn(response, p)) != -1) {
                        set[gs.cap(1).toInt()] = gs.cap(2).toDouble();
                        p += gs.matchedLength();
                    }
                    QList<int> keys = set.keys();
                    if (keys.contains(13)) m_settings->setUnits(set[13]);
                    if (keys.contains(20)) m_settings->setSoftLimitsEnabled(set[20]);
                    if (keys.contains(22)) {
                        m_settings->setHomingEnabled(set[22]);
                        m_machineBoundsDrawer.setVisible(set[22]);
                    }
                    if (keys.contains(110)) m_settings->setRapidSpeed(set[110]);
                    if (keys.contains(120)) m_settings->setAcceleration(set[120]);
                    if (keys.contains(130) && keys.contains(131) && keys.contains(132)) {
                        QVector3D bounds = QVector3D(
                            m_settings->referenceXPlus() ? -set[130] : set[130],
                            m_settings->referenceYPlus() ? -set[131] : set[131],
                            m_settings->referenceZPlus() ? -set[132] : set[132]
                            );
                        m_settings->setMachineBounds(bounds);
                        m_machineBoundsDrawer.setBorderRect(QRectF(0, 0,
                                                                   m_settings->referenceXPlus() ? -set[130] : set[130],
                                                                   m_settings->referenceYPlus() ? -set[131] : set[131]));
                        qDebug() << "Machine bounds (3-axis)" << bounds;
                    } else if (keys.contains(130) && keys.contains(131)) {
                        // 2 axis
                        QVector3D bounds = QVector3D(
                            m_settings->referenceXPlus() ? -set[130] : set[130],
                            m_settings->referenceYPlus() ? -set[131] : set[131],
                            0
                            );
                        m_settings->setMachineBounds(QVector3D(
                            m_settings->referenceXPlus() ? -set[130] : set[130],
                            m_settings->referenceYPlus() ? -set[131] : set[131],
                            0));
                        m_machineBoundsDrawer.setBorderRect(QRectF(0, 0,
                                                                   m_settings->referenceXPlus() ? -set[130] : set[130],
                                                                   m_settings->referenceYPlus() ? -set[131] : set[131]));
                        qDebug() << "Machine bounds (2-axis)" << bounds;
                    }

                    setupCoordsTextboxes();
                }

                // Homing response
                if ((uncomment == "$H" || uncomment == "$T") && m_communicator->m_homing) m_communicator->m_homing = false;

                // Reset complete response
                if (uncomment == "[CTRL+X]") {
                    m_communicator->m_resetCompleted = true;
                    m_updateParserStatus = true;

                    // Query grbl settings
                    m_communicator->sendCommand("$$", -2, false);
                    m_communicator->sendCommand("$#", -2, false, true);
                }

                // Clear command buffer on "M2" & "M30" command (old firmwares)
                static QRegExp M230("(M0*2|M30)(?!\\d)");
                if (uncomment.contains(M230) && response.contains("ok") && !response.contains("Pgm End")) {
                    m_communicator->m_commands.clear();
                    m_communicator->m_queue.clear();
                }

                // Update probe coords on user commands
                if (uncomment.contains("G38.2") && ca.tableIndex < 0) {
                    static QRegExp PRB(".*PRB:([^,]*),([^,]*),([^,:]*)");
                    if (PRB.indexIn(response) != -1) {
                        m_storedVars.setCoords("PRB", QVector3D(
                                                          PRB.cap(1).toDouble(),
                                                          PRB.cap(2).toDouble(),
                                                          PRB.cap(3).toDouble()
                                                          ));
                    }
                }

                // Process probing on heightmap mode only from table commands
                if (uncomment.contains("G38.2") && m_heightMapMode && ca.tableIndex > -1) {
                    // Get probe Z coordinate
                    // "[PRB:0.000,0.000,0.000:0];ok"
                    // "[PRB:0.000,0.000,0.000,0.000:0];ok"
                    QRegExp rx(".*PRB:([^,]*),([^,]*),([^,:]*)");
                    double z = qQNaN();
                    if (rx.indexIn(response) != -1) {
                        z = m_communicator->toMetric(rx.cap(3).toDouble());
                    }

                    static double firstZ;
                    if (m_communicator->m_probeIndex == -1) {
                        firstZ = z;
                        z = 0;
                    } else {
                        // Calculate delta Z
                        z -= firstZ;

                        // Calculate table indexes
                        int row = (m_communicator->m_probeIndex / m_heightMapModel.columnCount());
                        int column = m_communicator->m_probeIndex - row * m_heightMapModel.columnCount();
                        if (row % 2) column = m_heightMapModel.columnCount() - 1 - column;

                        // Store Z in table
                        m_heightMapModel.setData(m_heightMapModel.index(row, column), z, Qt::UserRole);
                        ui->tblHeightMap->update(m_heightMapModel.index(m_heightMapModel.rowCount() - 1 - row, column));
                        updateHeightMapInterpolationDrawer();
                    }

                    m_communicator->m_probeIndex++;
                }

                // Change state query time on check mode on
                if (uncomment.contains(QRegExp("$[cC]"))) {
                    m_timerStateQuery.setInterval(response.contains("Enable") ? 1000 : m_settings->queryStateTime());
                }

                // Add response to console
                if (tb.isValid() && tb.text() == ca.command) {

                    bool scrolledDown = ui->txtConsole->verticalScrollBar()->value()
                                        == ui->txtConsole->verticalScrollBar()->maximum();

                    // Update text block numbers
                    int blocksAdded = response.count("; ");

                    if (blocksAdded > 0) for (int i = 0; i < m_communicator->m_commands.count(); i++) {
                            if (m_communicator->m_commands[i].consoleIndex != -1) m_communicator->m_commands[i].consoleIndex += blocksAdded;
                        }

                    tc.beginEditBlock();
                    tc.movePosition(QTextCursor::EndOfBlock);

                    tc.insertText(" < " + QString(response).replace("; ", "\r\n"));
                    tc.endEditBlock();

                    if (scrolledDown) ui->txtConsole->verticalScrollBar()->setValue(
                            ui->txtConsole->verticalScrollBar()->maximum());
                }

                // Check queue
                static bool processingQueue = false;
                if (m_communicator->m_queue.length() > 0 && !processingQueue) {
                    processingQueue = true;
                    while (m_communicator->m_queue.length() > 0) {
                        CommandQueue cq = m_communicator->m_queue.takeFirst();
                        SendCommandResult r = m_communicator->sendCommand(cq.command, cq.tableIndex, cq.showInConsole);
                        if (r == SendDone) {
                            break;
                        } else if (r == SendQueue) {
                            m_communicator->m_queue.prepend(m_communicator->m_queue.takeLast());
                            break;
                        }
                    }
                    processingQueue = false;
                }

                // Add response to table, send next program commands
                if (m_communicator->m_senderState != SenderStopped) {
                    // Only if command from table
                    if (ca.tableIndex > -1) {
                        m_currentModel->setData(m_currentModel->index(ca.tableIndex, 2), GCodeItem::Processed);
                        m_currentModel->setData(m_currentModel->index(ca.tableIndex, 3), response);

                        m_fileProcessedCommandIndex = ca.tableIndex;

                        if (ui->chkAutoScroll->isChecked() && ca.tableIndex != -1) {
                            ui->tblProgram->scrollTo(m_currentModel->index(ca.tableIndex + 1, 0));      // TODO: Update by timer
                            ui->tblProgram->setCurrentIndex(m_currentModel->index(ca.tableIndex, 1));
                        }
                    }

// Update taskbar progress
#ifdef WINDOWS
                    if (QSysInfo::windowsVersion() >= QSysInfo::WV_WINDOWS7) {
                        if (m_taskBarProgress) m_taskBarProgress->setValue(m_fileProcessedCommandIndex);
                    }
#endif
                    // Process error messages
                    static bool holding = false;
                    static QString errors;

                    if (ca.tableIndex > -1 && response.toUpper().contains("ERROR") && !m_settings->ignoreErrors()) {
                        errors.append(QString::number(ca.tableIndex + 1) + ": " + ca.command + " < " + response + "\n");

                        m_senderErrorBox->setText(tr("Error message(s) received:\n") + errors);

                        if (!holding) {
                            holding = true;         // Hold transmit while messagebox is visible
                            response.clear();

                            m_connection->sendByteArray(QByteArray("!"));
                            //m_serialPort.write("!");11001
                            m_senderErrorBox->checkBox()->setChecked(false);
                            qApp->beep();
                            int result = m_senderErrorBox->exec();

                            holding = false;
                            errors.clear();
                            if (m_senderErrorBox->checkBox()->isChecked()) m_settings->setIgnoreErrors(true);
                            if (result == QMessageBox::Ignore) {
                                m_connection->sendByteArray(QByteArray("~"));
                            } else {
                                m_connection->sendByteArray(QByteArray("~"));
                                ui->cmdFileAbort->click();
                            }
                        }
                    }

                    // Check transfer complete (last row always blank, last command row = rowcount - 2)
                    if ((m_fileProcessedCommandIndex == m_currentModel->rowCount() - 2) || uncomment.contains(QRegExp("(M0*2|M30)(?!\\d)"))) {
                        if (m_communicator->m_deviceState == DeviceRun) {
                            m_communicator->setSenderState(SenderStopping);
                        } else {
                            completeTransfer();
                        }
                    } else if ((m_fileCommandIndex < m_currentModel->rowCount())
                               && (m_communicator->m_senderState == SenderTransferring)
                               && !holding)
                    {
                        // Send next program commands
                        sendNextFileCommands();
                    }
                }

                // Tool change mode
                static QRegExp M6("(M0*6)(?!\\d)");
                if ((m_communicator->m_senderState == SenderPausing) && uncomment.contains(M6)) {
                    response.clear();

                    if (m_settings->toolChangePause()) {
                        QMessageBox::information(this, qApp->applicationDisplayName(),
                                                 tr("Change tool and press 'Pause' button to continue job"));
                    }

                    if (m_settings->toolChangeUseCommands()) {
                        if (m_settings->toolChangeUseCommandsConfirm()) {
                            QMessageBox box(this);
                            box.setIcon(QMessageBox::Information);
                            box.setText(tr("M6 command detected. Send tool change commands?\n"));
                            box.setWindowTitle(qApp->applicationDisplayName());
                            box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                            box.setCheckBox(new QCheckBox(tr("Don't show again")));
                            int res = box.exec();
                            if (box.checkBox()->isChecked()) m_settings->setToolChangeUseCommandsConfirm(false);
                            if (res == QMessageBox::Yes) {
                                m_communicator->sendCommands(m_settings->toolChangeCommands());
                            }
                        } else {
                            m_communicator->sendCommands(m_settings->toolChangeCommands());
                        }
                    }

                    m_communicator->setSenderState(SenderChangingTool);
                    updateControlsState();
                }
                // Pausing on button?
                if ((m_communicator->m_senderState == SenderPausing) && !uncomment.contains(M6)) {
                    if (m_settings->usePauseCommands()) {
                        m_communicator->sendCommands(m_settings->beforePauseCommands());
                        m_communicator->setSenderState(SenderPausing2);
                        updateControlsState();
                    }
                }
                if ((m_communicator->m_senderState == SenderChangingTool) && !m_settings->toolChangePause()
                    && m_communicator->m_commands.isEmpty())
                {
                    m_communicator->setSenderState(SenderTransferring);
                }

                // Switch to pause mode
                if ((m_communicator->m_senderState == SenderPausing || m_communicator->m_senderState == SenderPausing2) && m_communicator->m_commands.isEmpty()) {
                    m_communicator->setSenderState(SenderPaused);
                    updateControlsState();
                }

                // Scroll to first line on "M30" command
                if (uncomment.contains("M30")) ui->tblProgram->setCurrentIndex(m_currentModel->index(0, 1));

                // Toolpath shadowing on check mode
                if (m_communicator->m_deviceState == DeviceCheck) {
                    GcodeViewParse *parser = m_currentDrawer->viewParser();
                    QList<LineSegment*> list = parser->getLineSegmentList();

                    if ((m_communicator->m_senderState != SenderStopping) && m_fileProcessedCommandIndex < m_currentModel->rowCount() - 1) {
                        int i;
                        QList<int> drawnLines;

                        for (i = m_lastDrawnLineIndex; i < list.count()
                                                       && list.at(i)->getLineNumber()
                                                              <= (m_currentModel->data(m_currentModel->index(m_fileProcessedCommandIndex, 4)).toInt()); i++) {
                            drawnLines << i;
                        }

                        if (!drawnLines.isEmpty() && (i < list.count())) {
                            m_lastDrawnLineIndex = i;
                            QVector3D vec = list.at(i)->getEnd();
                            m_toolDrawer.setToolPosition(vec);
                        }

                        foreach (int i, drawnLines) {
                            list.at(i)->setDrawn(true);
                        }
                        if (!drawnLines.isEmpty()) m_currentDrawer->update(drawnLines);
                    } else {
                        foreach (LineSegment* s, list) {
                            if (!qIsNaN(s->getEnd().length())) {
                                m_toolDrawer.setToolPosition(s->getEnd());
                                break;
                            }
                        }
                    }
                }

                // Emit response signal
                emit responseReceived(ca.command, ca.tableIndex, response);

                response.clear();
            } else {
                response.append(data + "; ");
            }

        } else {
            // Unprocessed responses
            // Handle hardware reset
            if (m_communicator->dataIsReset(data)) {
                m_communicator->setSenderState(SenderStopped);
                m_communicator->setDeviceState(DeviceUnknown);

                m_fileCommandIndex = 0;

                m_communicator->m_reseting = false;
                m_communicator->m_homing = false;

                m_updateParserStatus = true;
                m_communicator->m_statusReceived = true;

                m_communicator->clearCommandsAndQueue();

                updateControlsState();
            }
            ui->txtConsole->appendPlainText(data);
        }
    } else {
        // Blank response
    }
}