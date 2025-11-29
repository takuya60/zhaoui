#include "WaveformWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QRandomGenerator>
#include <QtMath>

WaveformWidget::WaveformWidget(QWidget *parent) : QWidget(parent)
{
    // 设置黑色/深色背景不太适合白色主题，这里我们后面用 paintEvent 画白底
    m_timer = new QTimer(this);
    m_timer->setInterval(33); // 30 FPS
    connect(m_timer, &QTimer::timeout, this, &WaveformWidget::onTimer);

    // 初始化 buffer
    m_buffer.resize(600);
    m_buffer.fill(0);
    m_timer->start(); // 始终运行画底噪
}

void WaveformWidget::updateParams(int freq, double posAmp, double negAmp, int posW, int negW, int dead, int duration) {
    m_freq = freq; m_posAmp = posAmp; m_negAmp = negAmp;
    m_posW = posW; m_negW = negW; m_dead = dead;
}

void WaveformWidget::start() { m_running = true; }
void WaveformWidget::stop() { m_running = false; }

void WaveformWidget::onTimer() {
    int pointsToAdd = 5;
    int period = 1000 / m_freq;

    for(int i=0; i<pointsToAdd; i++) {
        double val = 0;
        if(m_running) {
            int tCycle = (int)m_simTime % period;
            if (tCycle < m_posW) val = m_posAmp;
            else if (tCycle < m_posW + m_dead) val = 0;
            else if (tCycle < m_posW + m_dead + m_negW) val = -m_negAmp;
            else val = 0;

            val += (QRandomGenerator::global()->generateDouble() - 0.5) * 0.2;
        } else {
            val = (QRandomGenerator::global()->generateDouble() - 0.5) * 0.1;
        }

        m_buffer.pop_front();
        m_buffer.push_back(val);
        m_simTime += 2;
    }
    update(); // 触发重绘
}

void WaveformWidget::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // 1. 画背景 (白色)
    p.fillRect(rect(), Qt::white);

    // 2. 画网格
    int w = width();
    int h = height();
    double cy = h / 2.0;
    double scaleY = (h / 2.0) / 25.0; // ±25V 量程

    p.setPen(QPen(QColor("#e0e0e0"), 1));
    // 画横线 grid
    int levels[] = {0, 10, -10, 20, -20};
    for(int v : levels) {
        double y = cy - v * scaleY;
        p.drawLine(40, y, w, y);
        // 画文字
        p.setPen(QColor("#999999"));
        p.drawText(0, y - 5, 35, 10, Qt::AlignRight | Qt::AlignVCenter, QString::number(v) + "V");
        p.setPen(QPen(QColor("#e0e0e0"), 1)); // 还原画笔
    }

    // 3. 画波形
    QPainterPath path;
    double stepX = (double)(w - 40) / m_buffer.size();

    p.setClipRect(40, 0, w-40, h); // 设置剪裁区域，防止画到文字上

    for(int i=0; i<m_buffer.size(); i++) {
        double x = 40 + i * stepX;
        double y = cy - m_buffer[i] * scaleY;
        if(i==0) path.moveTo(x, y);
        else path.lineTo(x, y);
    }

    p.setPen(QPen(QColor("#00c853"), 2)); // 绿色波形
    p.drawPath(path);
}
