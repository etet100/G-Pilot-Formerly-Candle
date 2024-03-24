// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_CONSOLE_H
#define CONFIGURATION_CONSOLE_H

#include "module.h"

class ConfigurationConsole : public ConfigurationModule
{
    Q_OBJECT
    Q_PROPERTY(bool showProgramCommands MEMBER m_showProgramCommands NOTIFY changed)
    Q_PROPERTY(bool showUiCommands MEMBER m_showUiCommands NOTIFY changed)
    Q_PROPERTY(bool commandAutoCompletion MEMBER m_commandAutoCompletion NOTIFY changed)
    Q_PROPERTY(bool darkBackgroundMode MEMBER m_darkBackgroundMode NOTIFY changed)
    Q_PROPERTY(QStringList commandHistory MEMBER m_commandHistory NOTIFY changed)

    public:
        explicit ConfigurationConsole(QObject *parent = nullptr);
        ConfigurationConsole& operator=(const ConfigurationConsole&) { return *this; }
        QString getName() override { return "console"; }

        bool showProgramCommands() const { return m_showProgramCommands; }
        bool showUiCommands() const { return m_showUiCommands; }
        bool commandAutoCompletion() const { return m_commandAutoCompletion; }
        bool darkBackgroundMode() const { return m_darkBackgroundMode; }
        QStringList commandHistory() const { return m_commandHistory; }

    private:
        bool m_showProgramCommands;
        bool m_showUiCommands;
        bool m_commandAutoCompletion;
        bool m_darkBackgroundMode;
        QStringList m_commandHistory;
};

#endif // CONFIGURATION_CONSOLE_H
