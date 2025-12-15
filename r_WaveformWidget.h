#ifndef R_WAVEFORMWIDGET_H
#define R_WAVEFORMWIDGET_H

#include <QWidget>
#include <QVector>
#include <QTimer>

class WaveformWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaveformWidget(QWidget *parent = nullptr);
    void updateParams(int freq, double posAmp, double negAmp, int posW, int negW, int dead, int duration);
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTimer();

private:
    QVector<double> m_buffer;
    QTimer *m_timer;
    bool m_running = false;
    double m_simTime = 0;

    // 参数缓存
    int m_freq = 20;
    double m_posAmp = 10.0;
    double m_negAmp = 10.0;
    int m_posW = 10;
    int m_negW = 10;
    int m_dead = 2;
};

#endif // WAVEFORMWIDGET_H
