#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
// #include <QtMqtt/QMqttClient>
#include "./qtmqtt/mqtt/qmqttclient.h"
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QStackedWidget>
#include <QMenu>
#include <QActionGroup>
#include <QLayout>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateMqttStatus();
    void onSwitchChanged(bool checked);
    void onSliderChanged(int value);
    void onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic);

private:
    void setupUI();
    void setupMqtt();
    void applyStyle();
    void addFunctionPage(const QString &name, QWidget *page);

    // 页面构建
    QWidget *createEmptyPage();
    QWidget *createControlPage();
    QWidget *createMonitorPage();
    QWidget *createSettingsPage();
    QWidget *createAboutPage();

    // 通用构建辅助（前端风格卡片）
    QWidget *makeCard(const QString &title, QLayout *body);
    QWidget *makeStatCard(const QString &title, const QString &value, const QString &accent);
    QWidget *makeEmptyCard(const QString &title, const QString &hint);
    QWidget *makeFormRow(const QString &title, const QString &value);

    QMqttClient *m_client;
    QStackedWidget *m_stack;
    QMenu *m_menu;
    QActionGroup *m_menuGroup;

    // 控制页 UI
    QLabel *tempCelsiusLabel;
    QLabel *tempFahrenheitLabel;
    QCheckBox *doorSwitch;
    QCheckBox *fireSwitch;
    QSlider *intensitySlider;

    // 全局状态
    QLabel *statusLabel;
};

#endif // MAINWINDOW_H
