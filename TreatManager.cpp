#include "TreatManager.h"
#include <QDebug>
#include <QTimer>

TreatmentManager::TreatmentManager(IBackend *backend,QObject *parent)
: QObject(parent), m_backend(backend)
{
    m_state = DeviceState::Idle;
    m_remainingSeconds = 0;
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);

    connect(m_timer, &QTimer::timeout, this, &TreatmentManager::onTimerTick);
}
TreatmentManager:: ~TreatmentManager()
{
}
void TreatmentManager:: startTreatment(int duration)
{
    if (m_state==DeviceState::Running)
    {
        qDebug() << "Treatment is already running!";
        return;
    }
    

    m_backend->startStimulation(m_currentParam);
    QTimer *timer = new QTimer(this);
    m_timer->start(duration * 1000);
    m_state=DeviceState::Running;
}

void TreatmentManager::stopTreatment()
{
    if (m_state!=DeviceState::Running)
    {
        return;
    }

    if (m_timer->isActive()) 
    {
        m_timer->stop();
    }
    m_backend->stopStimulation();
    m_state=DeviceState::Idle;
}
void TreatmentManager::updateParameters(const StimulationParam &param)
{
    m_currentParam = param;
    //m_backend->updateParameters(param);
}

void TreatmentManager::setPIDParameters(const PIDParam &pid)
{
    m_backend->setPIDParameters(pid);
}

void TreatmentManager::onTimerTick()
{

    if (m_remainingSeconds > 0) {
        m_remainingSeconds--;

        emit timeUpdated(m_remainingSeconds);
    } else {
        // 停止
        stopTreatment();
    }

}