#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollArea>
#include <QFrame>
#include <QStyle>
#include <QDebug>

// ============================================================================
// Page 1: StimulationPage (刺激控制页)
// ============================================================================
StimulationPage::StimulationPage(QWidget *parent) : QWidget(parent) {
    setupUi();

    m_logicTimer = new QTimer(this);
    connect(m_logicTimer, &QTimer::timeout, this, &StimulationPage::onLogicTimer);

    // 初始化时，先执行一次移动更新文字，再执行一次应用更新参数
    onSliderMoved();
    onBtnApplyClicked();
}

void StimulationPage::setupUi() {
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // === 左侧：波形显示区 ===
    QWidget *leftPanel = new QWidget;
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0,0,0,0);

    QLabel *lblTitle = new QLabel("  实时输出");
    lblTitle->setFixedHeight(40);
    lblTitle->setStyleSheet("background: #fafafa; color: #666; font-weight: bold; border-bottom: 1px solid #e0e0e0;");

    m_waveWidget = new WaveformWidget;
    leftLayout->addWidget(lblTitle);
    leftLayout->addWidget(m_waveWidget);

    // === 右侧：控制台 ===
    QWidget *rightPanel = new QWidget;
    rightPanel->setFixedWidth(380);
    rightPanel->setStyleSheet("background: #ffffff; border-left: 1px solid #e0e0e0;");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0,0,0,0);
    rightLayout->setSpacing(0);

    // 1. 顶部状态栏
    QWidget *statusBar = new QWidget;
    statusBar->setFixedHeight(80);
    statusBar->setStyleSheet("background: #fafafa; border-bottom: 1px solid #e0e0e0;");
    QHBoxLayout *statusLayout = new QHBoxLayout(statusBar);

    QVBoxLayout *stTxt = new QVBoxLayout;
    QLabel *lblSt = new QLabel("设备状态");
    lblSt->setStyleSheet("color:#888; font-size:12px; border:none; background:transparent;");
    m_lblStatus = new QLabel("待机中");
    m_lblStatus->setStyleSheet("color:#333; font-size:18px; font-weight:bold; border:none; background:transparent;");
    stTxt->addWidget(lblSt);
    stTxt->addWidget(m_lblStatus);

    m_lblTime = new QLabel("01:00");
    m_lblTime->setStyleSheet("font-family: monospace; font-size:32px; font-weight:bold; color:#333; border:none; background:transparent;");

    statusLayout->addLayout(stTxt);
    statusLayout->addStretch();
    statusLayout->addWidget(m_lblTime);
    rightLayout->addWidget(statusBar);

    // 2. 中间滚动区域
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollContent = new QWidget;
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setSpacing(15);
    scrollLayout->setContentsMargins(15,10,15,10);

    // --- 监测区 ---
    QGroupBox *grpMon = new QGroupBox("实时监测");
    grpMon->setStyleSheet("QGroupBox { font-weight: bold; border: none; margin-top: 10px; font-size: 14px; color: #666; } QGroupBox::title { subcontrol-origin: margin; left: 0px; }");

    QGridLayout *gridMon = new QGridLayout;
    m_monV = new QLabel("0.0 V");
    m_monI = new QLabel("0.0 mA");
    m_monR = new QLabel("-- Ω");
    m_monQ = new QLabel("0 µC");

    auto createCard = [](QString t, QLabel* v) {
        QWidget *w = new QWidget;
        w->setStyleSheet("background: #fff; border: 1px solid #e0e0e0; border-radius: 6px;");
        QVBoxLayout *l = new QVBoxLayout(w);
        l->setContentsMargins(10,5,10,5);
        QLabel *tl = new QLabel(t);
        tl->setStyleSheet("color:#888; font-size:11px; border:none; background:transparent;");
        v->setStyleSheet("color:#333; font-size:18px; font-weight:bold; border:none; background:transparent;");
        l->addWidget(tl);
        l->addWidget(v);
        return w;
    };

    gridMon->addWidget(createCard("峰值电压", m_monV), 0, 0);
    gridMon->addWidget(createCard("平均电流", m_monI), 0, 1);
    gridMon->addWidget(createCard("负载阻抗", m_monR), 1, 0);
    gridMon->addWidget(createCard("累计电荷", m_monQ), 1, 1);
    grpMon->setLayout(gridMon);
    scrollLayout->addWidget(grpMon);

    // --- 参数设置区 ---
    QGroupBox *grpSet = new QGroupBox("刺激参数设置");
    grpSet->setStyleSheet("QGroupBox { font-weight: bold; border: none; margin-top: 10px; font-size: 14px; color: #666; } QGroupBox::title { subcontrol-origin: margin; left: 0px; }");
    QVBoxLayout *setLayout = new QVBoxLayout;
    setLayout->setSpacing(10);

    // 【核心修复】将 auto 改为具体的函数指针 void (StimulationPage::*slot)() 和 类指针 StimulationPage* receiver
    auto createSlider = [](QString name, int min, int max, int val, QSlider*& s, QLabel*& l, QString unit, void (StimulationPage::*slot)(), StimulationPage* receiver) {
        QWidget *w = new QWidget;
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0,0,0,0);
        vl->setSpacing(2);

        QHBoxLayout *hl = new QHBoxLayout;
        hl->addWidget(new QLabel(name));
        hl->addStretch();
        l = new QLabel(QString::number(val) + " " + unit);
        l->setStyleSheet("color: #0091ea; font-weight: bold; border:none; background:transparent;");
        hl->addWidget(l);

        s = new QSlider(Qt::Horizontal);
        s->setRange(min, max);
        s->setValue(val);
        s->setStyleSheet(
            "QSlider::groove:horizontal { border: 1px solid #bbb; background: white; height: 6px; border-radius: 3px; } "
            "QSlider::sub-page:horizontal { background: #0091ea; border-radius: 3px; } "
            "QSlider::handle:horizontal { background: white; border: 2px solid #0091ea; width: 18px; height: 18px; margin: -7px 0; border-radius: 9px; }"
        );
        QObject::connect(s, &QSlider::valueChanged, receiver, slot);

        vl->addLayout(hl);
        vl->addWidget(s);
        return w;
    };

    setLayout->addWidget(createSlider("刺激频率", 1, 100, 20, m_slFreq, m_valFreq, "Hz", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("正向幅值", 0, 250, 100, m_slPosAmp, m_valPosAmp, "V", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("负向幅值", 0, 250, 100, m_slNegAmp, m_valNegAmp, "V", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("正向脉宽", 1, 50, 10, m_slPosW, m_valPosW, "ms", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("脉间死区", 0, 20, 2, m_slDead, m_valDead, "ms", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("负向脉宽", 1, 50, 10, m_slNegW, m_valNegW, "ms", &StimulationPage::onSliderMoved, this));
    setLayout->addWidget(createSlider("治疗时长", 10, 300, 60, m_slDur, m_valDur, "s", &StimulationPage::onSliderMoved, this));

    grpSet->setLayout(setLayout);
    scrollLayout->addWidget(grpSet);
    scrollLayout->addStretch();

    scrollArea->setWidget(scrollContent);
    rightLayout->addWidget(scrollArea);

    // 3. 底部按钮区 (包含 应用设置 + 开始治疗)
    QWidget *bottomArea = new QWidget;
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomArea);
    bottomLayout->setContentsMargins(15,10,15,15);
    bottomLayout->setSpacing(10);

    // 【新增】应用参数设置按钮
    m_btnApply = new QPushButton("确认参数设置");
    m_btnApply->setFixedHeight(45);
    m_btnApply->setCursor(Qt::PointingHandCursor);
    m_btnApply->setStyleSheet(
        "QPushButton { background-color: #0091ea; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } "
        "QPushButton:hover { background-color: #00b0ff; } "
        "QPushButton:pressed { background-color: #0081cb; }"
    );
    connect(m_btnApply, &QPushButton::clicked, this, &StimulationPage::onBtnApplyClicked);
    bottomLayout->addWidget(m_btnApply);

    // 开始/停止治疗按钮
    m_btnAction = new QPushButton("开始治疗");
    m_btnAction->setFixedHeight(50);
    m_btnAction->setCursor(Qt::PointingHandCursor);
    m_btnAction->setStyleSheet(
        "QPushButton { background-color: #00c853; color: white; border-radius: 6px; font-size: 18px; font-weight: bold; border: none; } "
        "QPushButton:hover { background-color: #00e676; } "
        "QPushButton:pressed { background-color: #00a344; }"
    );
    connect(m_btnAction, &QPushButton::clicked, this, &StimulationPage::toggleStimulation);

    bottomLayout->addWidget(m_btnAction);
    rightLayout->addWidget(bottomArea);

    mainLayout->addWidget(leftPanel, 1);
    mainLayout->addWidget(rightPanel, 0);
}

// 槽函数：仅更新文字预览，不改变生效参数
void StimulationPage::onSliderMoved() {
    m_valFreq->setText(QString::number(m_slFreq->value()) + " Hz");
    m_valPosAmp->setText(QString::number(m_slPosAmp->value() / 10.0, 'f', 1) + " V");
    m_valNegAmp->setText(QString::number(m_slNegAmp->value() / 10.0, 'f', 1) + " V");
    m_valPosW->setText(QString::number(m_slPosW->value()) + " ms");
    m_valDead->setText(QString::number(m_slDead->value()) + " ms");
    m_valNegW->setText(QString::number(m_slNegW->value()) + " ms");
    m_valDur->setText(QString::number(m_slDur->value()) + " s");
}

// 槽函数：点击确认后，将参数“提交”到生效区
void StimulationPage::onBtnApplyClicked() {
    // 1. 将滑块值保存到 Active 变量
    m_activeFreq = m_slFreq->value();
    m_activePosAmp = m_slPosAmp->value() / 10.0;
    m_activeNegAmp = m_slNegAmp->value() / 10.0;
    m_activePosW = m_slPosW->value();
    m_activeDead = m_slDead->value();
    m_activeNegW = m_slNegW->value();
    m_activeDur = m_slDur->value();

    // 2. 更新波形图 (使用 Active 参数)
    m_waveWidget->updateParams(
        m_activeFreq, m_activePosAmp, m_activeNegAmp,
        m_activePosW, m_activeNegW, m_activeDead, m_activeDur
    );

    // 3. 联动 PID 页面
    emit posAmpChanged(m_activePosAmp);

    // 4. 重置剩余时间 (如果需要的话，或者只在开始时重置)
    if (!m_isRunning) {
        m_remainingTime = m_activeDur;
        int m = m_remainingTime / 60;
        int s = m_remainingTime % 60;
        m_lblTime->setText(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));
    }
}

void StimulationPage::toggleStimulation() {
    m_isRunning = !m_isRunning;
    if(m_isRunning) {
        m_btnAction->setText("停止治疗");
        m_btnAction->setStyleSheet("QPushButton { background-color: #d50000; color: white; border-radius: 6px; font-size: 18px; font-weight: bold; border: none; } QPushButton:hover { background-color: #ff1744; }");

        m_lblStatus->setText("运行中");
        m_lblStatus->setStyleSheet("color: #00c853; font-size: 18px; font-weight: bold; border:none; background: transparent;");

        // 确保使用最新的 Active 参数开始
        m_waveWidget->start();
        m_remainingTime = m_activeDur; // 使用已确认的时长
        m_chargeAcc = 0;
        m_logicTimer->start(1000);

        // 禁用参数修改 (可选优化：运行时禁用滑块和确认按钮)
        m_btnApply->setEnabled(false);
        m_btnApply->setStyleSheet("QPushButton { background-color: #ccc; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; }");

    } else {
        m_btnAction->setText("开始治疗");
        m_btnAction->setStyleSheet("QPushButton { background-color: #00c853; color: white; border-radius: 6px; font-size: 18px; font-weight: bold; border: none; } QPushButton:hover { background-color: #00e676; }");

        m_lblStatus->setText("待机中");
        m_lblStatus->setStyleSheet("color: #333; font-size: 18px; font-weight: bold; border:none; background: transparent;");

        m_waveWidget->stop();
        m_logicTimer->stop();
        m_monV->setText("0.0 V");
        m_monI->setText("0.0 mA");

        // 恢复参数修改
        m_btnApply->setEnabled(true);
        m_btnApply->setStyleSheet("QPushButton { background-color: #0091ea; color: white; border-radius: 6px; font-size: 16px; font-weight: bold; border: none; } QPushButton:hover { background-color: #00b0ff; } QPushButton:pressed { background-color: #0081cb; }");
    }
}

void StimulationPage::onLogicTimer() {
    if(m_remainingTime > 0) {
        m_remainingTime--;
        int m = m_remainingTime / 60;
        int s = m_remainingTime % 60;
        m_lblTime->setText(QString("%1:%2").arg(m, 2, 10, QChar('0')).arg(s, 2, 10, QChar('0')));

        // 【关键】使用 Active 参数来计算监测值，而不是滑块值
        // 这样即使拖动滑块，如果不点击应用，监测值也不会变
        double v = m_activePosAmp;

        m_monV->setText(QString::number(v, 'f', 1) + " V");
        m_monI->setText(QString::number(v * 2.0, 'f', 1) + " mA");
        m_monR->setText("500 Ω");
        m_chargeAcc += 0.5;
        m_monQ->setText(QString::number((int)m_chargeAcc) + " µC");
    } else {
        toggleStimulation();
    }
}

// ============================================================================
// Page 2: PidPage (PID 设置页)
// ============================================================================
PidPage::PidPage(QWidget *parent) : QWidget(parent) {
    setupUi();
    onSliderChanged(); // Init
}

void PidPage::setupUi() {
    // 居中布局容器
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);

    // 卡片容器
    QFrame *card = new QFrame;
    card->setFixedSize(500, 480);
    card->setStyleSheet("background: #ffffff; border-radius: 12px; border: 1px solid #e0e0e0;");

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(0,0,0,0);

    // Header
    QLabel *lblHeader = new QLabel("🎛️ 闭环控制参数");
    lblHeader->setFixedHeight(60);
    lblHeader->setAlignment(Qt::AlignCenter);
    lblHeader->setStyleSheet("font-size: 18px; font-weight: bold; color: #333; border-bottom: 1px solid #e0e0e0; border-top-left-radius: 12px; border-top-right-radius: 12px; border: none; border-bottom: 1px solid #eee;");
    cardLayout->addWidget(lblHeader);

    // Content
    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setContentsMargins(30, 30, 30, 30);
    contentLayout->setSpacing(20);

    // Tip
    m_lblInfo = new QLabel("ℹ️ 提示：PID 目标值已自动关联至正向幅值 (10.0V)。");
    m_lblInfo->setStyleSheet("background: #e0f7fa; color: #006064; border-radius: 6px; padding: 10px; border: 1px solid #b2ebf2; font-size: 12px;");
    m_lblInfo->setWordWrap(true);
    contentLayout->addWidget(m_lblInfo);

    // Sliders Helper
    // 【核心修复】同样修复 PidPage 中的 Lambda，将 auto 替换为 void (PidPage::*slot)(), PidPage* receiver
    auto createSlider = [](QString name, int min, int max, int val, QSlider*& s, QLabel*& l, void (PidPage::*slot)(), PidPage* receiver) {
        QWidget *w = new QWidget;
        QVBoxLayout *vl = new QVBoxLayout(w);
        vl->setContentsMargins(0,0,0,0);
        vl->setSpacing(5);

        QHBoxLayout *hl = new QHBoxLayout;
        hl->addWidget(new QLabel(name));
        hl->addStretch();
        l = new QLabel(QString::number(val));
        l->setStyleSheet("color: #00b8d4; font-weight: bold; border:none;");
        hl->addWidget(l);

        s = new QSlider(Qt::Horizontal);
        s->setRange(min, max);
        s->setValue(val);
        s->setStyleSheet(
            "QSlider::groove:horizontal { border: 1px solid #bbb; background: white; height: 6px; border-radius: 3px; } "
            "QSlider::sub-page:horizontal { background: #00b8d4; border-radius: 3px; } "
            "QSlider::handle:horizontal { background: white; border: 2px solid #00b8d4; width: 18px; height: 18px; margin: -7px 0; border-radius: 9px; }"
        );
        QObject::connect(s, &QSlider::valueChanged, receiver, slot);

        vl->addLayout(hl);
        vl->addWidget(s);
        return w;
    };

    contentLayout->addWidget(createSlider("比例系数 (Kp)", 0, 100, 15, m_slKp, m_valKp, &PidPage::onSliderChanged, this));
    contentLayout->addWidget(createSlider("积分系数 (Ki)", 0, 500, 20, m_slKi, m_valKi, &PidPage::onSliderChanged, this));
    contentLayout->addWidget(createSlider("微分系数 (Kd)", 0, 200, 5,  m_slKd, m_valKd, &PidPage::onSliderChanged, this));

    contentLayout->addStretch();

    // 恢复默认按钮
    QPushButton *btnReset = new QPushButton("恢复默认");
    btnReset->setFixedSize(140, 40);
    btnReset->setCursor(Qt::PointingHandCursor);
    btnReset->setStyleSheet("QPushButton { background: #fff; color: #333; border: 1px solid #ccc; border-radius: 6px; font-weight: bold; } QPushButton:hover { background: #f5f5f5; }");
    connect(btnReset, &QPushButton::clicked, this, &PidPage::resetParams);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(btnReset);
    btnLayout->addStretch();
    contentLayout->addLayout(btnLayout);

    cardLayout->addLayout(contentLayout);
    mainLayout->addWidget(card);
}

void PidPage::onSliderChanged() {
    double kp = m_slKp->value() / 10.0;
    double ki = m_slKi->value() / 100.0;
    double kd = m_slKd->value() / 100.0;

    m_valKp->setText(QString::number(kp, 'f', 1));
    m_valKi->setText(QString::number(ki, 'f', 2));
    m_valKd->setText(QString::number(kd, 'f', 2));
}

void PidPage::resetParams() {
    m_slKp->setValue(15);
    m_slKi->setValue(20);
    m_slKd->setValue(5);
}

void PidPage::updateTargetTip(float val) {
    m_lblInfo->setText(QString("ℹ️ 提示：PID 目标值已自动关联至正向幅值 (%1V)。").arg(val, 0, 'f', 1));
}

// ============================================================================
// Page 3: BlankPage (空白页)
// ============================================================================
BlankPage::BlankPage(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QLabel *lbl = new QLabel("更多系统设置\n(开发中...)");
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color: #ccc; font-size: 24px; font-weight: bold; border: none;");
    layout->addWidget(lbl);
}

// ============================================================================
// MainWindow (主窗口导航)
// ============================================================================
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupStyle();
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    centralWidget->setStyleSheet("background-color: #f0f2f5;"); // 全局背景色

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);

    // 1. 顶部导航栏
    QWidget *navBar = new QWidget;
    navBar->setFixedHeight(50);
    navBar->setStyleSheet("background: #ffffff; border-bottom: 1px solid #e0e0e0;");
    QHBoxLayout *navLayout = new QHBoxLayout(navBar);
    navLayout->setContentsMargins(0,0,0,0);
    navLayout->setSpacing(20);
    navLayout->addStretch(); // 左弹簧

    m_navGroup = new QButtonGroup(this);
    m_navGroup->setExclusive(true);

    auto createNavBtn = [this](int id, QString text) {
        QPushButton *btn = new QPushButton(text);
        btn->setCheckable(true);
        btn->setFixedSize(160, 48); // 高度略小于导航栏
        btn->setCursor(Qt::PointingHandCursor);
        btn->setProperty("navId", id); // 用于QSS选择器

        m_navGroup->addButton(btn, id);
        // 使用 Lambda 连接信号
        connect(btn, &QPushButton::clicked, [this, id](){ onNavButtonClicked(id); });
        return btn;
    };

    navLayout->addWidget(createNavBtn(0, "波形控制"));
    navLayout->addWidget(createNavBtn(1, "PID 参数整定"));
    navLayout->addWidget(createNavBtn(2, "系统设置"));

    navLayout->addStretch(); // 右弹簧

    mainLayout->addWidget(navBar);

    // 2. 页面容器
    m_stackedWidget = new QStackedWidget;
    m_pageStim = new StimulationPage;
    m_pagePid = new PidPage;
    m_pageBlank = new BlankPage;

    m_stackedWidget->addWidget(m_pageStim);
    m_stackedWidget->addWidget(m_pagePid);
    m_stackedWidget->addWidget(m_pageBlank);

    mainLayout->addWidget(m_stackedWidget);

    // 3. 联动信号连接
    // 当 StimPage 的幅值变化时，通知 PidPage 更新提示
    connect(m_pageStim, &StimulationPage::posAmpChanged, m_pagePid, &PidPage::updateTargetTip);

    // 默认选中第一页
    m_navGroup->button(0)->click();
}

void MainWindow::setupStyle() {
    // 导航按钮样式
    // 利用 border-bottom 实现 Tab 选中下划线效果
    this->setStyleSheet(R"(
        QPushButton[navId] {
            border: none;
            background: transparent;
            font-size: 14px;
            font-weight: bold;
            color: #666;
            border-bottom: 3px solid transparent;
            border-radius: 0px;
        }
        QPushButton[navId]:hover {
            background: #f5f5f5;
            color: #333;
        }
        QPushButton[navId]:checked {
            color: #0091ea; /* 选中蓝色文字 */
            border-bottom: 3px solid #0091ea; /* 选中蓝色底条 */
            background: #e3f2fd; /* 选中浅蓝背景 */
        }
    )");
}

void MainWindow::onNavButtonClicked(int id) {
    m_stackedWidget->setCurrentIndex(id);
}
