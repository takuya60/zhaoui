#ifndef TREATMENTMANAGER_H
#define TREATMENTMANAGER_H

#include <QObject>
#include <QTimer>
#include "IBackend.h"

enum class DeviceState {
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

private:
    IBackend *m_backend;
    StimulationParam m_currentParam;
    QTimer *m_timer;
    DeviceState m_state;
    int m_remainingSeconds; 
private slots:
    void onTimerTick();

signals:

    void timeUpdated(int secondsLeft);

};


#endif