#ifndef TELEMETRYCONTROLLER_H
#define TELEMETRYCONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QString>

class TelemetryController : public QObject
{
    Q_OBJECT

    // GPS & Position
    Q_PROPERTY(double latitude READ latitude WRITE setLatitude NOTIFY telemetryChanged)
    Q_PROPERTY(double longitude READ longitude WRITE setLongitude NOTIFY telemetryChanged)
    Q_PROPERTY(double altitudeWGS84 READ altitudeWGS84 WRITE setAltitudeWGS84 NOTIFY telemetryChanged)
    Q_PROPERTY(double altitudeAGL READ altitudeAGL WRITE setAltitudeAGL NOTIFY telemetryChanged)
    Q_PROPERTY(int satellites READ satellites WRITE setSatellites NOTIFY telemetryChanged)
    Q_PROPERTY(double groundSpeed READ groundSpeed WRITE setGroundSpeed NOTIFY telemetryChanged)
    
    // Attitude
    Q_PROPERTY(double roll READ roll WRITE setRoll NOTIFY telemetryChanged)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY telemetryChanged)
    Q_PROPERTY(double yaw READ yaw WRITE setYaw NOTIFY telemetryChanged)

    // Battery
    Q_PROPERTY(double batteryVoltage READ batteryVoltage WRITE setBatteryVoltage NOTIFY telemetryChanged)
    Q_PROPERTY(double batteryRemaining READ batteryRemaining WRITE setBatteryRemaining NOTIFY telemetryChanged)
    
    // Link & Security (FHSS / E2EE)
    Q_PROPERTY(bool e2eeEnabled READ e2eeEnabled WRITE setE2eeEnabled NOTIFY securityChanged)
    Q_PROPERTY(bool fhssSyncActive READ fhssSyncActive WRITE setFhssSyncActive NOTIFY securityChanged)
    Q_PROPERTY(int fhssChannel READ fhssChannel WRITE setFhssChannel NOTIFY securityChanged)
    Q_PROPERTY(int signalJamming READ signalJamming WRITE setSignalJamming NOTIFY rfHealthChanged)
    Q_PROPERTY(QString spoofingState READ spoofingState WRITE setSpoofingState NOTIFY rfHealthChanged)

    // System Diagnostics (READY TO FLY vs NO-GO)
    Q_PROPERTY(bool readyToFly READ readyToFly NOTIFY diagnosticsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY diagnosticsChanged)
    Q_PROPERTY(double cpuLoad READ cpuLoad WRITE setCpuLoad NOTIFY diagnosticsChanged)

public:
    explicit TelemetryController(QObject *parent = nullptr);

    // Getters
    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double altitudeWGS84() const { return m_altitudeWGS84; }
    double altitudeAGL() const { return m_altitudeAGL; }
    int satellites() const { return m_satellites; }
    double groundSpeed() const { return m_groundSpeed; }
    
    double roll() const { return m_roll; }
    double pitch() const { return m_pitch; }
    double yaw() const { return m_yaw; }

    double batteryVoltage() const { return m_batteryVoltage; }
    double batteryRemaining() const { return m_batteryRemaining; }

    bool e2eeEnabled() const { return m_e2eeEnabled; }
    bool fhssSyncActive() const { return m_fhssSyncActive; }
    int fhssChannel() const { return m_fhssChannel; }
    int signalJamming() const { return m_signalJamming; }
    QString spoofingState() const { return m_spoofingState; }

    bool readyToFly() const;
    QString statusMessage() const;
    double cpuLoad() const { return m_cpuLoad; }

    // Setters
    void setLatitude(double val);
    void setLongitude(double val);
    void setAltitudeWGS84(double val);
    void setAltitudeAGL(double val);
    void setSatellites(int val);
    void setGroundSpeed(double val);
    
    void setRoll(double val);
    void setPitch(double val);
    void setYaw(double val);

    void setBatteryVoltage(double val);
    void setBatteryRemaining(double val);

    void setE2eeEnabled(bool val);
    void setFhssSyncActive(bool val);
    void setFhssChannel(int val);
    void setSignalJamming(int val);
    void setSpoofingState(const QString &val);
    void setCpuLoad(double val);

    // Methods invokable from QML
    Q_INVOKABLE void armVehicle();
    Q_INVOKABLE void disarmVehicle();
    Q_INVOKABLE void toggleJammingSimulation();
    Q_INVOKABLE void abortMission();

signals:
    void telemetryChanged();
    void securityChanged();
    void rfHealthChanged();
    void diagnosticsChanged();

private slots:
    void updateSimulation();

private:
    QTimer *m_simTimer;

    // Simulated States
    double m_latitude;
    double m_longitude;
    double m_altitudeWGS84;
    double m_altitudeAGL;
    int m_satellites;
    double m_groundSpeed;
    
    double m_roll;
    double m_pitch;
    double m_yaw;

    double m_batteryVoltage;
    double m_batteryRemaining;

    bool m_e2eeEnabled;
    bool m_fhssSyncActive;
    int m_fhssChannel;
    int m_signalJamming;
    QString m_spoofingState;
    double m_cpuLoad;

    bool m_armed;
    bool m_simulatedJamming;
};

#endif // TELEMETRYCONTROLLER_H
