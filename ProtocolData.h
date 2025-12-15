#ifndef PROTOCOLDATA_H
#define PROTOCOLDATA_H

#include <stdint.h>

// 波形包一次传输的采样点数量
// 建议：M4 采样率若为 10kHz，50个点 = 5ms 传输一次
#define WAVEFORM_BATCH_SIZE  50 

// --- 帧头定义 (区分包的类型) ---
#define HEAD_CONTROL  0xAA  // [下行] 普通控制包 (Start/Stop/Update)
#define HEAD_PID      0xDD  // [下行] PID 参数配置包 (工程模式用)
#define HEAD_WAVEFORM 0xBB  // [上行] 高速波形包 (画图用)
#define HEAD_STATUS   0xCC  // [上行] 低速状态包 (监控用)

// --- 指令类型 (用于 ControlPacket.cmd) ---
#define CMD_START     0x01  // 开始治疗 (复位定时器，从头输出)
#define CMD_STOP      0x02  // 停止治疗 (急停，输出归零)
#define CMD_UPDATE    0x03  // 更新参数 (平滑过渡，不重置定时器)

// --- 错误码 (用于 StatusPacket.error_code) ---
#define ERR_NONE      0x00  // 正常
#define ERR_ELECTRODE 0x01  // 电极脱落
#define ERR_OVER_CURR 0x02  // 过流保护
#define ERR_TIMEOUT   0x03  // 通信超时 / 内部错误

// =============================================================
// 2. 数据结构定义
// 使用 pack(1) 强制 1 字节对齐，防止跨平台内存错位
// =============================================================
#pragma pack(1)

/**
 * @brief 1. 普通控制包 (Linux -> M4)
 */
struct ControlPacket {
    uint8_t  head;        // 固定为 HEAD_CONTROL (0xAA)
    uint8_t  cmd;         // CMD_START / CMD_STOP / CMD_UPDATE
    
    // --- 时域参数 ---
    uint16_t freq;        // 频率 (Hz)
    uint16_t negative_width; // 脉宽 (us)
    uint16_t positive_width;
    // --- 死区参数 (双向波安全关键) ---
    uint16_t dead_pulse;  // 脉间死区 (us): 正波与负波的间隔
    //uint32_t dead_cycle;  // 周期间死区 (us): 两个波形周期之间的间隔
                          // (注: 若由 M4 自动计算，此字段可填 0)
    
    // --- 幅值参数 ---
    float    amp_pos;     // 正向幅值 (mA)
    float    amp_neg;     // 反向幅值 (mA) - 传入绝对值即可
    
    uint8_t  checksum;    // 校验和
};

/**
 * @brief 2. PID 配置包 (Linux -> M4)
 */
struct PIDPacket {
    uint8_t  head;        // 固定为 HEAD_PID (0xDD)
    uint8_t  reserved;    // 保留字节 (对齐用，可不填)
    
    // --- PID 系数 ---
    float    kp;
    float    ki;
    float    kd;
    
    // --- 积分限幅 ---
    float    integration_limit; // 防止积分饱和导致过充
    
    uint8_t  checksum;    // 校验和
};

/**
 * @brief 3. 高速波形包 (M4 -> Linux)
 * @note 批量传输 ADC 数据，用于 Qt Charts 画图和上位机算法分析
 */
struct WaveformPacket {
    uint8_t  head;        // 固定为 HEAD_WAVEFORM (0xBB)
    
    // --- 批量采样数据 (已转换为 mA) ---
    float    adc_batch[WAVEFORM_BATCH_SIZE]; 
    
    uint8_t  checksum;    // 校验和
};

/**
 * @brief 4. 低速状态包 (M4 -> Linux)
 * @note 每秒发送一次，用于系统监控
 */
struct StatusPacket {
    uint8_t  head;        // 固定为 HEAD_STATUS (0xCC)
    
    // --- 硬件回读 (用于双重校验) ---
    float    impedance;   // 接触阻抗 (Ohm)
    uint16_t real_freq;   // M4 定时器当前实际频率
    
    // --- 系统信息 ---
    uint8_t  battery_pct; // 电池电量 (0-100)
    uint8_t  error_code;  // 错误码 (见 ERR_ 宏)
    
    uint8_t  checksum;    // 校验和
};

#pragma pack(pop) // 恢复默认对齐

// 通用校验算法

/**
 * @brief 计算校验和 (简单累加法)
 * @param data 结构体指针
 * @param len  需要计算的长度 (通常是 sizeof(Struct) - 1)
 */
static inline uint8_t calculateChecksum(const void* data, int len) {
    const uint8_t* p = (const uint8_t*)data;
    uint8_t sum = 0;
    for (int i = 0; i < len; i++) {
        sum += p[i];
    }
    return sum;
}

#endif // PROTOCOLDATA_H