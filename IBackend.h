# ifndef IBACKEND_H
# define IBACKEND_H
# include "ProtocolData.h"
# include <Qobject>

// 刺激参数结构体
struct StimulationParam
{
    int freq;
    float posAmp;
    float negAmp;
    int posW;
    int dead;
    int negW;
    StimulationParam() 
        : freq(50), posW(10), negW(10), dead(20), 
          posAmp(0.0f), negAmp(0.0f) {}
};
// PID 参数结构体
struct PIDSettings {
    float kp;
    float ki;
    float kd;
    float limit; // 积分限幅
    
    // 构造函数
    PIDSettings() : kp(1.0f), ki(0.0f), kd(0.0f), limit(100.0f) {}
};

class IBackend : public QObject
{
    Q_OBJECT

public:
    explicit IBackend(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~IBackend() {}

// 开始/停止刺激，更新参数
virtual void startStimulation(const StimulationParam &param) = 0;
virtual void stopStimulation() = 0;
virtual void updateParameters(const StimulationParam &param)=0;

// 设置 PID 参数
virtual void setPIDParameters(const PIDSettings &pid) = 0;

signals:
    // 波形数据包接收
    void waveDataReceived(const WaveformPacket &packet);
    // 状态数据包接收
    void statusDataReceived(const StatusPacket &packet);
    // 错误发生
    void errorOccurred(QString msg);


};

# endif