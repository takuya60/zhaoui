#include "TreatManager.h"
#include <QDebug>
#include <QTimer>

TreatmentManager::TreatmentManager(IBackend *backend,QObject *parent)
: QObject(parent), m_backend(backend)
{
    m_state = Runstate::Idle;
    emit runstateChanged(Runstate::Idle);
    m_remainingSeconds = 0;
    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(1000);

    connect(m_timer, &QTimer::timeout, this, &TreatmentManager::onTimerTick);
    connect(m_backend, &IBackend::statusDataReceived,this,&TreatmentManager::handleStatusPacket);
    connect(m_backend,&IBackend::waveDataReceived,this,&TreatmentManager::handleWaveformPacket);
}
TreatmentManager:: ~TreatmentManager()
{
}

// 开始治疗
void TreatmentManager:: startTreatment(int duration)
{
    if (m_state==Runstate::Running)
    {
        qDebug() << "Treatment is already running!";
        return;
    }
    

    m_backend->startStimulation(m_currentParam);
    QTimer *timer = new QTimer(this);
    m_timer->start(duration * 1000);
    m_state=Runstate::Running;
    emit runstateChanged(Runstate::Running);
}

// 停止治疗
void TreatmentManager::stopTreatment()
{
    if (m_state!=Runstate::Running)
    {
        return;
    }

    if (m_timer->isActive()) 
    {
        m_timer->stop();
    }
    m_backend->stopStimulation();
    m_state=Runstate::Idle;
    emit runstateChanged(Runstate::Idle);
}

// 更新刺激参数
void TreatmentManager::updateParameters(const StimulationParam &param)
{
    m_currentParam = param;
    //m_backend->updateParameters(param);
}

void TreatmentManager::setPIDParameters(const PIDParam &pid)
{
    m_backend->setPIDParameters(pid);
}

// 处理波形点数据包
void TreatmentManager::handleWaveformPacket(const WaveformPacket &packet)
{
    QVector<float> data;
    data.reserve (WAVEFORM_BATCH_SIZE);
    for (int i = 0; i < WAVEFORM_BATCH_SIZE; i++) {
        data.append(packet.adc_batch[i]);
    }

    // 这里 之后用一个专门的analyz类去处理信号，分析实际频率等
    //然后把频率等跟data一起放在signal里


    emit waveformReceived(data);

} 

// 处理状态数据包
void TreatmentManager::handleStatusPacket(const StatusPacket &packet)
{

    // 之后跟据errorcode加入一些错误处理

    emit stateChanged(packet.impedance, packet.battery_pct, packet.error_code);

}


// 定时器槽函数
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