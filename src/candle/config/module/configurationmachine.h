// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONMACHINE_H
#define CONFIGURATIONMACHINE_H

#include <QObject>
#include "configurationmodule.h"
#include <QVector3D>

class ConfigurationMachine : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(ConfigurationModule::MinMax spindleSpeedRange MEMBER m_spindleSpeedRange NOTIFY changed)
    Q_PROPERTY(ConfigurationModule::MinMax laserPowerRange MEMBER m_laserPowerRange NOTIFY changed)
    Q_PROPERTY(ConfigurationMachine::ReferencePositionDir referencePositionDirX MEMBER m_referencePositionDirX NOTIFY changed)
    Q_PROPERTY(ConfigurationMachine::ReferencePositionDir referencePositionDirY MEMBER m_referencePositionDirY NOTIFY changed)
    Q_PROPERTY(ConfigurationMachine::ReferencePositionDir referencePositionDirZ MEMBER m_referencePositionDirZ NOTIFY changed)
    Q_PROPERTY(bool overrideMaxTravel MEMBER m_overrideMaxTravel NOTIFY changed)
    Q_PROPERTY(QVector3D maxTravel MEMBER m_maxTravel NOTIFY changed)

    public:
        ConfigurationMachine(QObject *parent);
        QString getSectionName() override { return "machine"; }

        enum ReferencePositionDir : int {
            Negative,
            Positive
        };

        ConfigurationModule::MinMax spindleSpeedRange() const { return m_spindleSpeedRange; }
        double spindleSpeedRatio() const { return (m_spindleSpeedRange.max - m_spindleSpeedRange.min) / 100; }
        ConfigurationModule::MinMax laserPowerRange() const { return m_laserPowerRange; }
        ReferencePositionDir referencePositionDirX() const { return m_referencePositionDirX; }
        ReferencePositionDir referencePositionDirY() const { return m_referencePositionDirY; }
        ReferencePositionDir referencePositionDirZ() const { return m_referencePositionDirZ; }
        bool overrideMaxTravel() const { return m_overrideMaxTravel; }
        QVector3D maxTravel() const { return m_maxTravel; }

    private:
        MinMax m_spindleSpeedRange;
        MinMax m_laserPowerRange;
        ReferencePositionDir m_referencePositionDirX;
        ReferencePositionDir m_referencePositionDirY;
        ReferencePositionDir m_referencePositionDirZ;
        bool m_overrideMaxTravel;
        QVector3D m_maxTravel;
};

Q_DECLARE_METATYPE(ConfigurationMachine::ReferencePositionDir);

#endif // CONFIGURATIONMACHINE_H
