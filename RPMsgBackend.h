#ifndef RPMSGBACKEND_H
#define RPMSGBACKEND_H
#include "IBackend.h"
#include <QSocketNotifier>
class RPMsgBackend : public IBackend
{
    Q_OBJECT

public:
    explicit RPMsgBackend(QObject *parent = nullptr);
            ~RPMsgBackend() override;
// 开始/停止刺激，更新参数
    void startStimulation(const StimulationParam &param)override;
    void stopStimulation()override;
    //void updateParameters(const StimulationParam &param)override;

// 设置 PID 参数
    void setPIDParameters(const PIDParam &pid)override;


private:
    int m_fd;
    QSocketNotifier *m_notifier;
    
    PIDParam m_currentPID;
    // 辅助函数
    void sendControlPacket(const ControlPacket &packet);
    void sendPIDPacket(const PIDPacket &pid);

private slots:
    void handleSocketActivated(int socket);

};



#endif // RPMSGBACKEND_H