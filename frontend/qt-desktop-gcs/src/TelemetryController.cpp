#include "TelemetryController.h"
#include <QRandomGenerator>
#include <cmath>

TelemetryController::TelemetryController(QObject *parent)
    : QObject(parent),
      m_latitude(28.4595),    // Coordinates centered on Gurugram
      m_longitude(77.0266),
      m_altitudeWGS84(250.0),
      m_altitudeAGL(0.0),
      m_satellites(18),
      m_groundSpeed(0.0),
      m_roll(0.0),
      m_pitch(0.0),
      m_yaw(0.0),
      m_batteryVoltage(16.8), // 4S Battery fully charged
      m_batteryRemaining(1.0),
      m_e2eeEnabled(true),
      m_fhssSyncActive(true),
      m_fhssChannel(12),
      m_signalJamming(25),    // Normal background RF noise
      m_spoofingState("NO_SPOOFING"),
      m_cpuLoad(15.4),
      m_armed(false),
      m_simulatedJamming(false)
{
    m_simTimer = new QTimer(this);
    connect(m_simTimer, &QTimer::timeout, this, &TelemetryController::updateSimulation);
    m_simTimer->start(100); // 10Hz updates for telemetry values
}

void TelemetryController::updateSimulation()
{
    // Attitude jitter simulation
    static double step = 0.0;
    step += 0.1;

    if (m_armed) {
        // Drone flying simulation
        m_groundSpeed = 12.5 + sin(step * 0.2) * 2.0;
        m_altitudeAGL += (15.0 - m_altitudeAGL) * 0.05; // climb to 15m AGL
        m_altitudeWGS84 = 250.0 + m_altitudeAGL;

        // Position movement (simulating small flight orbit)
        m_latitude += cos(step * 0.05) * 0.00002;
        m_longitude += sin(step * 0.05) * 0.00002;

        // Dynamic attitude changes matching movement
        m_roll = sin(step * 0.5) * 8.0;
        m_pitch = cos(step * 0.5) * 4.0;
        m_yaw = fmod(m_yaw + 0.5, 360.0);

        // Battery drain
        m_batteryRemaining = qMax(0.0, m_batteryRemaining - 0.0002);
        m_batteryVoltage = 14.0 + (m_batteryRemaining * 2.8);
    } else {
        // On ground state
        m_groundSpeed = 0.0;
        m_altitudeAGL = qMax(0.0, m_altitudeAGL - 1.0);
        m_altitudeWGS84 = 250.0;
        m_roll = sin(step * 0.1) * 0.5; // Slight drift
        m_pitch = cos(step * 0.1) * 0.5;
        
        // Recharge battery slowly on ground for demo robustness
        if (m_batteryRemaining < 1.0) {
            m_batteryRemaining = qMin(1.0, m_batteryRemaining + 0.001);
            m_batteryVoltage = 14.0 + (m_batteryRemaining * 2.8);
        }
    }

    // FHSS Channel Hopping simulation (hops every few ticks if sync is active)
    if (m_fhssSyncActive && QRandomGenerator::global()->bounded(10) < 3) {
        m_fhssChannel = QRandomGenerator::global()->bounded(1, 50);
        emit securityChanged();
    }

    // Jamming Simulation impact
    if (m_simulatedJamming) {
        m_signalJamming = QRandomGenerator::global()->bounded(200, 255);
        m_spoofingState = "SPOOFING";
        m_satellites = QRandomGenerator::global()->bounded(0, 4);
        m_fhssSyncActive = false;
        m_e2eeEnabled = false;
        emit rfHealthChanged();
        emit securityChanged();
    } else {
        m_signalJamming = QRandomGenerator::global()->bounded(10, 45);
        m_spoofingState = "NO_SPOOFING";
        m_satellites = 18;
    }

    m_cpuLoad = 12.0 + sin(step * 0.1) * 3.0;

    emit telemetryChanged();
    emit diagnosticsChanged();
}

bool TelemetryController::readyToFly() const
{
    // Secure defense drone pre-flight checks:
    // 1. Must not be jammed or spoofed.
    // 2. Battery must be > 20%.
    // 3. E2E encryption must be enabled.
    // 4. FHSS hopping must be active and synchronized.
    // 5. GPS lock must have sufficient satellites.
    if (m_simulatedJamming || m_signalJamming > 150) return false;
    if (m_batteryRemaining < 0.20) return false;
    if (!m_e2eeEnabled) return false;
    if (!m_fhssSyncActive) return false;
    if (m_satellites < 6) return false;
    return true;
}

QString TelemetryController::statusMessage() const
{
    if (m_simulatedJamming) {
        return QString("🔴 NO-GO: ACTIVE RF JAMMING & GPS SPOOFING DETECTED");
    }
    if (m_batteryRemaining < 0.20) {
        return QString("🔴 NO-GO: BATTERY CAPACITY CRITICAL (< 20%)");
    }
    if (!m_e2eeEnabled) {
        return QString("🔴 NO-GO: E2E LINK ENCRYPTION DISENGAGED");
    }
    if (!m_fhssSyncActive) {
        return QString("🔴 NO-GO: FHSS HOPPING OUT OF SYNC");
    }
    if (m_satellites < 6) {
        return QString("🔴 NO-GO: INSUFFICIENT GPS SATELLITES FOR LOCK");
    }
    if (m_armed) {
        return QString("🟢 FLIGHT COMMAND ACTIVE: TARGET TRACKING ENGAGED");
    }
    return QString("🟢 READY TO FLY: SYSTEM HEALTH VERIFIED");
}

void TelemetryController::setLatitude(double val)
{
    if (qFuzzyCompare(m_latitude, val)) return;
    m_latitude = val;
    emit telemetryChanged();
}

void TelemetryController::setLongitude(double val)
{
    if (qFuzzyCompare(m_longitude, val)) return;
    m_longitude = val;
    emit telemetryChanged();
}

void TelemetryController::setAltitudeWGS84(double val)
{
    if (qFuzzyCompare(m_altitudeWGS84, val)) return;
    m_altitudeWGS84 = val;
    emit telemetryChanged();
}

void TelemetryController::setAltitudeAGL(double val)
{
    if (qFuzzyCompare(m_altitudeAGL, val)) return;
    m_altitudeAGL = val;
    emit telemetryChanged();
}

void TelemetryController::setSatellites(int val)
{
    if (m_satellites == val) return;
    m_satellites = val;
    emit telemetryChanged();
}

void TelemetryController::setGroundSpeed(double val)
{
    if (qFuzzyCompare(m_groundSpeed, val)) return;
    m_groundSpeed = val;
    emit telemetryChanged();
}

void TelemetryController::setRoll(double val)
{
    if (qFuzzyCompare(m_roll, val)) return;
    m_roll = val;
    emit telemetryChanged();
}

void TelemetryController::setPitch(double val)
{
    if (qFuzzyCompare(m_pitch, val)) return;
    m_pitch = val;
    emit telemetryChanged();
}

void TelemetryController::setYaw(double val)
{
    if (qFuzzyCompare(m_yaw, val)) return;
    m_yaw = val;
    emit telemetryChanged();
}

void TelemetryController::setBatteryVoltage(double val)
{
    if (qFuzzyCompare(m_batteryVoltage, val)) return;
    m_batteryVoltage = val;
    emit telemetryChanged();
}

void TelemetryController::setBatteryRemaining(double val)
{
    if (qFuzzyCompare(m_batteryRemaining, val)) return;
    m_batteryRemaining = val;
    emit telemetryChanged();
}

void TelemetryController::setE2eeEnabled(bool val)
{
    if (m_e2eeEnabled == val) return;
    m_e2eeEnabled = val;
    emit securityChanged();
    emit diagnosticsChanged();
}

void TelemetryController::setFhssSyncActive(bool val)
{
    if (m_fhssSyncActive == val) return;
    m_fhssSyncActive = val;
    emit securityChanged();
    emit diagnosticsChanged();
}

void TelemetryController::setFhssChannel(int val)
{
    if (m_fhssChannel == val) return;
    m_fhssChannel = val;
    emit securityChanged();
}

void TelemetryController::setSignalJamming(int val)
{
    if (m_signalJamming == val) return;
    m_signalJamming = val;
    emit rfHealthChanged();
    emit diagnosticsChanged();
}

void TelemetryController::setSpoofingState(const QString &val)
{
    if (m_spoofingState == val) return;
    m_spoofingState = val;
    emit rfHealthChanged();
    emit diagnosticsChanged();
}

void TelemetryController::setCpuLoad(double val)
{
    if (qFuzzyCompare(m_cpuLoad, val)) return;
    m_cpuLoad = val;
    emit diagnosticsChanged();
}

void TelemetryController::armVehicle()
{
    if (!readyToFly()) return;
    m_armed = true;
    emit diagnosticsChanged();
}

void TelemetryController::disarmVehicle()
{
    m_armed = false;
    emit diagnosticsChanged();
}

void TelemetryController::toggleJammingSimulation()
{
    m_simulatedJamming = !m_simulatedJamming;
    if (!m_simulatedJamming) {
        // Recover state
        m_fhssSyncActive = true;
        m_e2eeEnabled = true;
        m_satellites = 18;
    }
    emit diagnosticsChanged();
}

void TelemetryController::abortMission()
{
    m_armed = false;
    m_altitudeAGL = 0.0;
    m_groundSpeed = 0.0;
    emit diagnosticsChanged();
}
