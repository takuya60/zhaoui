#include "RPMsgBackend.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 
#include <QDebug>


RPMsgBackend::RPMsgBackend(QObject *parent): IBackend(parent)
{

    m_fd =  open("/dev/ttyRPMSG0", O_RDWR | O_NONBLOCK);
    if (m_fd>=0)
    {
        m_notifier = new QSocketNotifier(m_fd,QSocketNotifier::Read,this);

        connect(m_notifier,&QSocketNotifier::activated,this,&RPMsgBackend::handleSocketActivated);

        m_notifier->setEnabled(true);
    }
    else
    {
        qDebug() << "Open Error!";
    }
    
}
RPMsgBackend:: ~RPMsgBackend()
{
    if (m_notifier)
    {
        m_notifier->setEnabled(false);
        delete m_notifier;
    }
    
    if (m_fd >= 0)
    {
        close(m_fd);
        m_fd = -1;
    }
}

// 开始刺激
void RPMsgBackend :: startStimulation(const StimulationParam &param)
{

    ControlPacket packet={0};
    packet.head = HEAD_CONTROL;
    packet.cmd = CMD_START;
    packet.freq = param.freq;
    packet.amp_neg= param.negAmp;
    packet.amp_pos= param.posAmp;
    packet.positive_width= param.posW;
    packet.negative_width= param.negW;
    packet.dead_pulse= param.dead;

    packet.checksum = calculateChecksum(&packet, sizeof(packet) - 1);

    sendControlPacket(packet);

}

// 停止刺激
void RPMsgBackend :: stopStimulation()
{
    ControlPacket packet={0};
    packet.head =HEAD_CONTROL;
    packet.cmd =CMD_STOP;

    packet.checksum = calculateChecksum(&packet, sizeof(packet) - 1);

    sendControlPacket(packet);
}

//更新参数
// void RPMsgBackend :: updateParameters(const StimulationParam &param)
// {
//     ControlPacket packet={0};
//     packet.head = HEAD_CONTROL;
//     packet.cmd = CMD_UPDATE;
//     packet.freq = param.freq;
//     packet.amp_neg= param.negAmp;
//     packet.amp_pos= param.posAmp;
//     packet.positive_width= param.posW;
//     packet.negative_width= param.negW;
//     packet.dead_pulse= param.dead;

//     packet.checksum = calculateChecksum(&packet, sizeof(packet) - 1);

//     sendControlPacket(packet);

// }

// 设置 PID 参数
void RPMsgBackend ::setPIDParameters(const PIDParam &pid)
{
    PIDPacket packet={0};
    packet.head = HEAD_PID;
    packet.kp = pid.kp;
    packet.ki =pid.ki;
    packet.kd = pid.kd;
    packet.integration_limit = pid.limit;
    packet.checksum = calculateChecksum(&packet, sizeof(packet) - 1);
    sendPIDPacket(packet);
}

//发送电刺激数据
void RPMsgBackend :: sendControlPacket(const ControlPacket &packet)
{
    ssize_t bytesWritten = write(m_fd, &packet, sizeof(packet));
    if (bytesWritten != sizeof(packet))
    {
        qDebug() << "Write ControlPacket Error!";
    }
}

//发送PID数据
void RPMsgBackend :: sendPIDPacket(const PIDPacket &packet)
{
    ssize_t bytesWritten = write(m_fd, &packet, sizeof(packet));
    if (bytesWritten != sizeof(packet))
    {
        qDebug() << "Write PIDPacket Error!";
    }
}


// socket激活的槽函数
void RPMsgBackend :: handleSocketActivated(int socket)
{
    uint8_t buffer[1024];
    ssize_t bytesRead=read(socket,buffer,sizeof(buffer));
    if (bytesRead <=0)
    {
        qDebug() << "Read Error!";
    }
    else
    {
        //解析包头
        if (buffer[0]==HEAD_WAVEFORM)
        {
            if (bytesRead == sizeof(WaveformPacket))
            {
                WaveformPacket* packet=(WaveformPacket*)buffer;
                
                // 校验和验证
                uint8_t checksum = calculateChecksum(packet, sizeof(WaveformPacket) - 1);
                if (checksum == packet->checksum)
                {
                    emit waveDataReceived(*packet);
                }
                else
                {
                    qDebug() << "WaveformPacket Checksum Error!";
                }
            }
            else
            {
                qDebug() << "WaveformPacket Size Error!";
            }
            
        }
        else if (buffer[0]==HEAD_STATUS)
        {
            if (bytesRead == sizeof(StatusPacket))
            {
                StatusPacket * packet=(StatusPacket*)buffer;
                // 校验和验证
                uint8_t checksum = calculateChecksum(packet, sizeof(StatusPacket) - 1);
                if (checksum == packet->checksum)
                {
                    emit statusDataReceived(*packet);
                }
                else
                {
                    qDebug() << "statusDataPacket Checksum Error!";
                }
            }
            else
            {
                qDebug() << "statusDataPacket Size Error!";
            }
            
        }
    }
}

