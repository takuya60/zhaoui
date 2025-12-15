#ifndef TREATMENTMANAGER_H
#define TREATMENTMANAGER_H

#include <QObject>
#include <QTimer>
#include "IBackend.h"

enum class Runstate {
    Idle,
    Running,
    Paused
};

class TreatmentManager : public QObject
{
    Q_OBJECT
public:
    explicit TreatmentManager(IBackend *backend,QObject *parent =nullptr);
     ~TreatmentManager();

    void startTreatment(int duration);
    void stopTreatment();
    void updateParameters(const StimulationParam &param);
    void setPIDParameters(const PIDParam &pid);
    Runstate currentState() const { return m_state; }

private:
    IBackend *m_backend;
    StimulationParam m_currentParam;
    QTimer *m_timer;
    Runstate m_state;
    int m_remainingSeconds; 
private slots:
    void onTimerTick();
    void handleStatusPacket(const StatusPacket &packet);
    void handleWaveformPacket(const WaveformPacket &packet);

signals:
    void runstateChanged(Runstate State);
    void timeUpdated(int secondsLeft);

    void stateChanged(float impedance, uint8_t batteryPct, uint8_t errorCode);
    void waveformReceived(const QVector<float> &data);


};


#endif