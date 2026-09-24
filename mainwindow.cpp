#include "mainwindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QScrollArea>
#include <QStackedWidget>
#include <QMenu>
#include <QMenuBar>
#include <QActionGroup>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupUI();
    setupMqtt();

    // 桌面程序默认尺寸 + 最小尺寸，内容随窗口自动缩放
    resize(1100, 720);
    setMinimumSize(760, 560);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    setWindowTitle("功能测试台");
    applyStyle();

    // 连接状态胶囊：放在菜单栏右上角，默认隐藏，进入功能页才显示
    statusLabel = new QLabel("未连接");
    statusLabel->setStyleSheet("padding: 4px 14px; border-radius: 12px;"
                               "background: #f0f2f5; color: #5a6472; font-size: 12px;");
    menuBar()->setCornerWidget(statusLabel, Qt::TopRightCorner);
    statusLabel->setVisible(false);

    QWidget *central = new QWidget(this);
    QVBoxLayout *root = new QVBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // 菜单栏：功能项（新增功能就在这里加一项）
    m_menu = menuBar()->addMenu("mqtt测试");
    m_menuGroup = new QActionGroup(this);
    m_menuGroup->setExclusive(true);

    // 页面容器：索引 0 是默认空页，点菜单项才切换到对应页面
    m_stack = new QStackedWidget();
    m_stack->addWidget(createEmptyPage());
    addFunctionPage("控制", createControlPage());
    addFunctionPage("监控", createMonitorPage());
    addFunctionPage("设置", createSettingsPage());
    addFunctionPage("关于", createAboutPage());
    root->addWidget(m_stack);

    setCentralWidget(central);

    // 绑定信号
    connect(doorSwitch, &QCheckBox::toggled, this, &MainWindow::onSwitchChanged);
    connect(fireSwitch, &QCheckBox::toggled, this, &MainWindow::onSwitchChanged);
    connect(intensitySlider, &QSlider::valueChanged, this, &MainWindow::onSliderChanged);
}

void MainWindow::addFunctionPage(const QString &name, QWidget *page)
{
    m_stack->addWidget(page);

    QAction *act = m_menu->addAction(name);
    act->setCheckable(true);
    m_menuGroup->addAction(act);

    connect(act, &QAction::triggered, this, [this, page]() {
        m_stack->setCurrentWidget(page);
        statusLabel->setVisible(true);
    });
}

void MainWindow::applyStyle()
{
    setStyleSheet("QMainWindow { background: #f5f6fa; }"
                  "QWidget#page { background: #f5f6fa; }"
                  "QMenuBar { background: #ffffff; border-bottom: 1px solid #e6e8ef; }"
                  "QMenuBar::item { background: transparent; color: #5a6472; padding: 6px 14px; }"
                  "QMenuBar::item:selected { background: #f0f3fa; color: #3f51b5; border-radius: 4px; }"
                  "QMenu { background: #ffffff; border: 1px solid #e6e8ef; border-radius: 6px; padding: 4px; }"
                  "QMenu::item { padding: 8px 28px 8px 12px; border-radius: 4px; color: #1f2329; }"
                  "QMenu::item:selected { background: #f0f3fa; color: #3f51b5; }"
                  "QMenu::item:checked { color: #3f51b5; font-weight: 700; }"
                  "QFrame#card { background: #ffffff; border: 1px solid #eceff3; border-radius: 8px; }"
                  "QLabel#cardTitle { color: #1f2329; font-size: 15px; font-weight: 700; }"
                  "QLabel#hint { color: #9aa1ab; font-size: 13px; }"
                  "QScrollArea { background: transparent; border: none; }");
}

QWidget *MainWindow::createEmptyPage()
{
    QWidget *page = new QWidget();
    page->setObjectName("page");
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(20, 20, 20, 20);

    QLabel *hint = new QLabel("请从上方「功能」菜单选择功能模块");
    hint->setObjectName("hint");
    hint->setAlignment(Qt::AlignCenter);

    lay->addStretch();
    lay->addWidget(hint, 0, Qt::AlignHCenter);
    lay->addStretch();
    return page;
}

QWidget *MainWindow::createControlPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget();
    container->setObjectName("page");
    QVBoxLayout *page = new QVBoxLayout(container);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);

    // ---- 卡片1：温度监测 ----
    QVBoxLayout *tempBody = new QVBoxLayout();
    tempBody->setSpacing(12);

    auto addInfoRow = [&](QLayout *parent, const QString &title, QLabel *value) {
        QWidget *row = new QWidget();
        QHBoxLayout *lay = new QHBoxLayout(row);
        lay->setContentsMargins(0, 0, 0, 0);
        QLabel *t = new QLabel(title);
        t->setObjectName("hint");
        value->setStyleSheet("font-size: 20px; font-weight: 700; color: #1f2329;");
        lay->addWidget(t);
        lay->addStretch();
        lay->addWidget(value);
        parent->addWidget(row);
    };

    tempCelsiusLabel = new QLabel("34.1");
    tempFahrenheitLabel = new QLabel("83.4");
    addInfoRow(tempBody, "房间温度 (°C)", tempCelsiusLabel);
    addInfoRow(tempBody, "房间温度 (°F)", tempFahrenheitLabel);
    page->addWidget(makeCard("温度监测", tempBody));

    // ---- 卡片2：开关控制 ----
    QVBoxLayout *switchBody = new QVBoxLayout();
    switchBody->setSpacing(12);

    auto addSwitchRow = [&](QLayout *parent, const QString &title, QCheckBox *&cb) {
        QWidget *row = new QWidget();
        QHBoxLayout *lay = new QHBoxLayout(row);
        lay->setContentsMargins(0, 0, 0, 0);
        QLabel *t = new QLabel(title);
        t->setObjectName("hint");
        cb = new QCheckBox();
        cb->setCursor(Qt::PointingHandCursor);
        // 自定义 Switch 样式
        cb->setStyleSheet("QCheckBox::indicator { width: 45px; height: 24px; }"
                          "QCheckBox::indicator:unchecked { background: #ccd2db; border-radius: 12px; }"
                          "QCheckBox::indicator:checked { background: #3f51b5; border-radius: 12px; }");
        lay->addWidget(t);
        lay->addStretch();
        lay->addWidget(cb);
        parent->addWidget(row);
    };

    addSwitchRow(switchBody, "控制主门", doorSwitch);
    addSwitchRow(switchBody, "模拟火灾", fireSwitch);
    page->addWidget(makeCard("开关控制", switchBody));

    // ---- 卡片3：灯光调节 ----
    QVBoxLayout *lightBody = new QVBoxLayout();
    lightBody->setSpacing(12);

    QLabel *intensityTitle = new QLabel("灯光强度");
    intensityTitle->setObjectName("hint");
    lightBody->addWidget(intensityTitle);

    intensitySlider = new QSlider(Qt::Horizontal);
    intensitySlider->setRange(0, 100);
    intensitySlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 6px; background: #e8ecf1; border-radius: 3px; }"
        "QSlider::handle:horizontal { background: white; border: 2px solid #3f51b5; width: 16px; margin: -7px 0; border-radius: 10px; }"
        "QSlider::sub-page:horizontal { background: #3f51b5; border-radius: 3px; }");
    lightBody->addWidget(intensitySlider);

    QLabel *colorTitle = new QLabel("灯光颜色");
    colorTitle->setObjectName("hint");
    lightBody->addWidget(colorTitle);

    QLabel *colorPicker = new QLabel();
    colorPicker->setMinimumHeight(120);
    colorPicker->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #ffffff, stop:1 #66bb6a); border-radius: 6px;");
    lightBody->addWidget(colorPicker);

    QHBoxLayout *presetLayout = new QHBoxLayout();
    presetLayout->setSpacing(8);
    QStringList colors = { "#e8efff", "#d1e0ff", "#a3c2ff", "#75a4ff", "#4786ff", "#3f51b5", "#303f9f", "#283593", "#1a237e" };
    for (const QString &c : colors)
    {
        QLabel *box = new QLabel();
        box->setFixedSize(26, 26);
        box->setCursor(Qt::PointingHandCursor);
        box->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(c));
        presetLayout->addWidget(box);
    }
    presetLayout->addStretch();
    lightBody->addLayout(presetLayout);

    page->addWidget(makeCard("灯光调节", lightBody));
    page->addStretch();

    scroll->setWidget(container);
    return scroll;
}

QWidget *MainWindow::createMonitorPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget();
    container->setObjectName("page");
    QVBoxLayout *page = new QVBoxLayout(container);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);

    // 统计卡片（示例，后续接真实数据）
    QHBoxLayout *stats = new QHBoxLayout();
    stats->setSpacing(16);
    stats->addWidget(makeStatCard("在线设备", "--", "#3f51b5"));
    stats->addWidget(makeStatCard("今日告警", "--", "#f57c00"));
    stats->addWidget(makeStatCard("消息 / 分钟", "--", "#2e7d32"));
    stats->addWidget(makeStatCard("运行时长", "--", "#7b1fa2"));
    page->addLayout(stats);

    page->addWidget(makeEmptyCard("实时数据图表", "在这里接入图表组件（例如 QtCharts / QCustomPlot），展示实时曲线。"));
    page->addWidget(makeEmptyCard("设备列表", "在这里用 QTableView + 模型展示在线设备清单。"));
    page->addStretch();

    scroll->setWidget(container);
    return scroll;
}

QWidget *MainWindow::createSettingsPage()
{
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget();
    container->setObjectName("page");
    QVBoxLayout *page = new QVBoxLayout(container);
    page->setContentsMargins(20, 20, 20, 20);
    page->setSpacing(16);

    QVBoxLayout *mqttBody = new QVBoxLayout();
    mqttBody->setSpacing(10);
    mqttBody->addWidget(makeFormRow("服务器", "110.40.141.231"));
    mqttBody->addWidget(makeFormRow("端口", "1883"));
    mqttBody->addWidget(makeFormRow("用户名", "admin"));
    page->addWidget(makeCard("MQTT 连接配置", mqttBody));

    page->addWidget(makeEmptyCard("系统设置", "在这里添加语言、主题、开机自启等设置项。"));
    page->addStretch();

    scroll->setWidget(container);
    return scroll;
}

QWidget *MainWindow::createAboutPage()
{
    QWidget *container = new QWidget();
    container->setObjectName("page");
    QVBoxLayout *page = new QVBoxLayout(container);
    page->setContentsMargins(20, 20, 20, 20);

    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(24, 24, 24, 24);
    lay->setSpacing(10);

    QLabel *logo = new QLabel("智能控制台");
    logo->setStyleSheet("font-size: 22px; font-weight: 700; color: #3f51b5;");
    logo->setAlignment(Qt::AlignCenter);
    lay->addWidget(logo);

    QLabel *ver = new QLabel("版本 1.0.0");
    ver->setObjectName("hint");
    ver->setAlignment(Qt::AlignCenter);
    lay->addWidget(ver);

    QLabel *desc = new QLabel("基于 Qt Widgets + MQTT 的桌面控制程序。\n采用上方 Tab 页签组织不同功能模块，各模块独立开发。");
    desc->setObjectName("hint");
    desc->setAlignment(Qt::AlignCenter);
    lay->addWidget(desc);

    page->addWidget(card);
    page->addStretch();
    return container;
}

// ============ 通用构建辅助 ============

QWidget *MainWindow::makeCard(const QString &title, QLayout *body)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 14, 16, 16);
    lay->setSpacing(12);
    QLabel *t = new QLabel(title);
    t->setObjectName("cardTitle");
    lay->addWidget(t);
    lay->addLayout(body);
    return card;
}

QWidget *MainWindow::makeStatCard(const QString &title, const QString &value, const QString &accent)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(4);
    QLabel *v = new QLabel(value);
    v->setStyleSheet(QString("font-size: 26px; font-weight: 700; color: %1;").arg(accent));
    QLabel *t = new QLabel(title);
    t->setObjectName("hint");
    lay->addWidget(v);
    lay->addWidget(t);
    return card;
}

QWidget *MainWindow::makeEmptyCard(const QString &title, const QString &hint)
{
    QFrame *card = new QFrame();
    card->setObjectName("card");
    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 14, 16, 16);
    lay->setSpacing(8);
    QLabel *t = new QLabel(title);
    t->setObjectName("cardTitle");
    lay->addWidget(t);
    QLabel *h = new QLabel(hint);
    h->setObjectName("hint");
    h->setWordWrap(true);
    lay->addWidget(h);
    QLabel *ph = new QLabel("待开发 · 占位区域");
    ph->setAlignment(Qt::AlignCenter);
    ph->setMinimumHeight(88);
    ph->setStyleSheet("color: #c0c4cc; border: 1px dashed #dcdfe6; border-radius: 6px;");
    lay->addWidget(ph);
    return card;
}

QWidget *MainWindow::makeFormRow(const QString &title, const QString &value)
{
    QWidget *row = new QWidget();
    QHBoxLayout *lay = new QHBoxLayout(row);
    lay->setContentsMargins(0, 0, 0, 0);
    QLabel *t = new QLabel(title);
    t->setObjectName("hint");
    QLabel *v = new QLabel(value);
    v->setStyleSheet("color: #1f2329; font-size: 14px;");
    lay->addWidget(t);
    lay->addStretch();
    lay->addWidget(v);
    return row;
}

// ============ MQTT ============

void MainWindow::setupMqtt()
{
    m_client = new QMqttClient(this);
    m_client->setHostname("110.40.141.231"); // 示例公共代理
    m_client->setPort(1883);
    m_client->setUsername("admin");
    m_client->setPassword("root123456");

    connect(m_client, &QMqttClient::stateChanged, this, &MainWindow::updateMqttStatus);
    connect(m_client, &QMqttClient::messageReceived, this, &MainWindow::onMqttMessageReceived);

    m_client->connectToHost();
}

void MainWindow::updateMqttStatus()
{
    switch (m_client->state())
    {
    case QMqttClient::Disconnected:
        statusLabel->setText("未连接");
        statusLabel->setStyleSheet("padding: 4px 14px; border-radius: 12px;"
                                   "background: #ffebee; color: #c62828; font-size: 12px;");
        qDebug() << "Disconnected from MQTT Broker";
        break;
    case QMqttClient::Connecting:
        statusLabel->setText("连接中…");
        statusLabel->setStyleSheet("padding: 4px 14px; border-radius: 12px;"
                                   "background: #fff3e0; color: #ef6c00; font-size: 12px;");
        qDebug() << "Connecting to MQTT Broker";
        break;
    case QMqttClient::Connected:
        statusLabel->setText("已连接");
        statusLabel->setStyleSheet("padding: 4px 14px; border-radius: 12px;"
                                   "background: #e8f5e8; color: #2e7d32; font-size: 12px;");
        qDebug() << "Connected to MQTT Broker";
        m_client->subscribe(QMqttTopicFilter("office/temp/data"));
        break;
    }
}

void MainWindow::onSwitchChanged(bool checked)
{
    if (m_client->state() == QMqttClient::Connected)
    {
        if (sender() == doorSwitch)
        {
            m_client->publish(QMqttTopicName("office/door/control"), checked ? "ON" : "OFF");
        }
        else if (sender() == fireSwitch)
        {
            m_client->publish(QMqttTopicName("office/fire/control"), checked ? "ON" : "OFF");
        }
    }
}

void MainWindow::onSliderChanged(int value)
{
    if (m_client->state() == QMqttClient::Connected)
    {
        m_client->publish(QMqttTopicName("office/light/intensity"), QByteArray::number(value));
    }
}

void MainWindow::onMqttMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    qDebug() << "Received:" << topic.name() << " -> " << message;

    if (topic.name() == "office/temp/data")
    {
        // 假设接收到的是温度数据，格式为"25.5"
        QString tempStr = QString(message);
        bool ok;
        double tempC = tempStr.toDouble(&ok);
        if (ok)
        {
            // 更新摄氏度显示
            tempCelsiusLabel->setText(QString::number(tempC, 'f', 1));
            // 转换为华氏度并更新显示
            double tempF = tempC * 9.0 / 5.0 + 32.0;
            tempFahrenheitLabel->setText(QString::number(tempF, 'f', 1));
        }
    }
}
