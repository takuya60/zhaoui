#ifndef R_MAINWINDOW_H
#define R_MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QTimer>
#include <QStackedWidget>
#include <QButtonGroup>
#include "WaveformWidget.h"
#include "TreatManager.h"


// ==========================================
// 页面 1: 刺激控制页
// ==========================================

class StimulationPage : public QWidget {
    Q_OBJECT
public:
    explicit StimulationPage(QWidget *parent = nullptr, TreatmentManager *manager) ;

signals:
    void posAmpChanged(float val);

private slots:
    void onSliderMoved();
    void onBtnApplyClicked();
    void toggleStimulation();
    void timehandle(int secondsLeft);
    void onLogicTimer();
    void onStateChanged(Runstate state);

private:
    void setupUi();
    StimulationParam sti_param;
    
    TreatmentManager *m_manager;
    
    WaveformWidget *m_waveWidget;
    QLabel *m_lblStatus;
    QLabel *m_lblTime;
    
    QPushButton *m_btnApply;
    QPushButton *m_btnAction;
    
    QSlider *m_slFreq;
    QSlider *m_slPosAmp;
    QSlider *m_slNegAmp;
    QSlider *m_slPosW;
    QSlider *m_slDead;
    QSlider *m_slNegW;
    QSlider *m_slDur;

    QLabel *m_valFreq;
    QLabel *m_valPosAmp;
    QLabel *m_valNegAmp;
    QLabel *m_valPosW;
    QLabel *m_valDead;
    QLabel *m_valNegW;
    QLabel *m_valDur;
    
    QLabel *m_monV;
    QLabel *m_monI;
    QLabel *m_monR;
    QLabel *m_monQ;

    bool m_isRunning = false;
    int m_remainingTime = 60;
    float m_chargeAcc = 0.0f; // 【修改】电荷累积也建议用 float
    QTimer *m_logicTimer;

    // 【修改】生效参数改为 float 类型
    int m_activeFreq;
    float m_activePosAmp; // double -> float
    float m_activeNegAmp; // double -> float
    int m_activePosW;
    int m_activeDead;
    int m_activeNegW;
    int m_activeDur;
};

// ==========================================
// 页面 2: PID 设置页
// ==========================================
class PidPage : public QWidget {
    Q_OBJECT
public:
    explicit PidPage(QWidget *parent = nullptr, TreatmentManager *manager);

public slots:
    // 【修改】参数类型改为 float
    void updateTargetTip(float val);

private slots:
    void onSliderChanged();
    void resetParams();
    void onBtnApplyClicked();

private:
    void setupUi();
    TreatmentManager *m_manager;
    PIDParam pid_param;
    QSlider *m_slKp;
    QSlider *m_slKi;
    QSlider *m_slKd;
    
    QLabel *m_valKp;
    QLabel *m_valKi;
    QLabel *m_valKd;
    QLabel *m_lblInfo;
    QPushButton *m_btnApply;
    QPushButton *m_btnReset;
};

// BlankPage
class BlankPage : public QWidget {
    Q_OBJECT
public:
    explicit BlankPage(QWidget *parent = nullptr);
};



// main window
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
private slots:
    void onNavButtonClicked(int id);
private:
    void setupUi();
    void setupStyle();
    TreatmentManager *m_manager;
    QStackedWidget *m_stackedWidget;
    QButtonGroup *m_navGroup;
    StimulationPage *m_pageStim;
    PidPage *m_pagePid;
    BlankPage *m_pageBlank;
};

#endif // MAINWINDOW_H