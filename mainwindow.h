#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QStackedWidget>
#include <QButtonGroup>
#include "WaveformWidget.h"

// ==========================================
// 页面 1: 刺激控制页
// ==========================================
class StimulationPage : public QWidget {
    Q_OBJECT
public:
    explicit StimulationPage(QWidget *parent = nullptr);

signals:
    // 信号：仅在点击“确认应用”后才发出，用于联动 PID 页面
    void posAmpChanged(double val);

private slots:
    void onSliderMoved();      // 新增：仅更新文字显示的槽函数
    void onBtnApplyClicked();  // 新增：确认应用参数的槽函数
    void toggleStimulation();
    void onLogicTimer();

private:
    void setupUi();

    WaveformWidget *m_waveWidget;
    QLabel *m_lblStatus;
    QLabel *m_lblTime;

    QPushButton *m_btnApply;   // 【新增】确认参数按钮
    QPushButton *m_btnAction;  // 开始/停止按钮

    // Sliders
    QSlider *m_slFreq;
    QSlider *m_slPosAmp;
    QSlider *m_slNegAmp;
    QSlider *m_slPosW;
    QSlider *m_slDead;
    QSlider *m_slNegW;
    QSlider *m_slDur;

    // Labels
    QLabel *m_valFreq;
    QLabel *m_valPosAmp;
    QLabel *m_valNegAmp;
    QLabel *m_valPosW;
    QLabel *m_valDead;
    QLabel *m_valNegW;
    QLabel *m_valDur;

    // Monitors
    QLabel *m_monV;
    QLabel *m_monI;
    QLabel *m_monR;
    QLabel *m_monQ;

    // Logic
    bool m_isRunning = false;
    int m_remainingTime = 60;
    double m_chargeAcc = 0;
    QTimer *m_logicTimer;

    // 【新增】当前生效的参数缓存
    // 只有点击 Apply 后，滑块的值才会存入这里，逻辑定时器和波形图都读取这些变量
    int m_activeFreq;
    double m_activePosAmp;
    double m_activeNegAmp;
    int m_activePosW;
    int m_activeDead;
    int m_activeNegW;
    int m_activeDur;
};

// ... (PidPage, BlankPage, MainWindow 的定义保持不变，此处省略以节省篇幅) ...
class PidPage : public QWidget {
    Q_OBJECT
public:
    explicit PidPage(QWidget *parent = nullptr);
public slots:
    void updateTargetTip(double val);
private slots:
    void onSliderChanged();
    void resetParams();
private:
    void setupUi();
    QSlider *m_slKp; QSlider *m_slKi; QSlider *m_slKd;
    QLabel *m_valKp; QLabel *m_valKi; QLabel *m_valKd; QLabel *m_lblInfo;
};

class BlankPage : public QWidget {
    Q_OBJECT
public:
    explicit BlankPage(QWidget *parent = nullptr);
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private slots:
    void onNavButtonClicked(int id);
private:
    void setupUi();
    void setupStyle();
    QStackedWidget *m_stackedWidget;
    QButtonGroup *m_navGroup;
    StimulationPage *m_pageStim;
    PidPage *m_pagePid;
    BlankPage *m_pageBlank;
};

#endif // MAINWINDOW_H
