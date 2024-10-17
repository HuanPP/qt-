#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QInputDialog>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>

// Vehicle 实现
Vehicle::Vehicle(const QString &licensePlate)
    : licensePlate(licensePlate), entryTime(QDateTime::currentDateTime()) {}

QString Vehicle::getLicensePlate() const { return licensePlate; }
QDateTime Vehicle::getEntryTime() const { return entryTime; }

// ParkingSpotManager 实现
ParkingSpotManager::ParkingSpotManager(int totalSpots) : totalSpots(totalSpots) {}

void ParkingSpotManager::parkVehicle(const Vehicle &vehicle) {
    if (parkingSpots.size() < totalSpots) {
        parkingSpots.append(vehicle);
    }
}

void ParkingSpotManager::removeVehicle(const QString &licensePlate) {
    for (int i = 0; i < parkingSpots.size(); ++i) {
        if (parkingSpots[i].getLicensePlate() == licensePlate) {
            parkingSpots.remove(i);  // 移除车辆
            break;  // 找到后立即退出循环
        }
    }
}

bool ParkingSpotManager::isFull() const {
    return parkingSpots.size() >= totalSpots;
}

QVector<Vehicle> ParkingSpotManager::getParkedVehicles() const {
    return parkingSpots;
}

bool ParkingSpotManager::hasVehicle(const QString &licensePlate) const {
    for (const Vehicle &vehicle : parkingSpots) {
        if (vehicle.getLicensePlate() == licensePlate) {
            return true;
        }
    }
    return false;
}

// QueueManager 实现
QueueManager::QueueManager(int maxCapacity) : maxCapacity(maxCapacity) {}

void QueueManager::addVehicleToQueue(const Vehicle &vehicle) {
    if (waitingQueue.size() < maxCapacity) {
        waitingQueue.enqueue(vehicle);
    }
}

Vehicle QueueManager::dequeueVehicle() {
    return waitingQueue.dequeue();
}

bool QueueManager::isQueueEmpty() const {
    return waitingQueue.isEmpty();
}

bool QueueManager::isQueueFull() const {
    return waitingQueue.size() >= maxCapacity;
}

QQueue<Vehicle> QueueManager::getWaitingQueue() const {
    return waitingQueue;
}

bool QueueManager::hasVehicleInQueue(const QString &licensePlate) const {
    for (const Vehicle &vehicle : waitingQueue) {
        if (vehicle.getLicensePlate() == licensePlate) {
            return true;
        }
    }
    return false;
}

// MainWindow 实现
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    parkingSpotManager(10), queueManager(5), isAnimating(false) {
    ui->setupUi(this);
    logWindow = new LogWindow();
    logWindow->show();  // Show the log window on start (you can control when to show it)

    // 初始化窗口大小
    this->resize(800, 600);

    // 用户输入停车位和队列容量
    bool ok;
    int totalSpots = QInputDialog::getInt(this, "停车位数量", "请输入停车场的停车位数量:", 10, 1, 100, 1, &ok);
    if (!ok) totalSpots = 10;

    int maxQueueSize = QInputDialog::getInt(this, "等待队列容量", "请输入等待队列的最大容量:", 5, 1, 20, 1, &ok);
    if (!ok) maxQueueSize = 5;

    // 初始化停车场和队列管理器
    parkingSpotManager = ParkingSpotManager(totalSpots);
    queueManager = QueueManager(maxQueueSize);

    setupUI();
    updateParkingStatus();
    updateQueueStatus();
}

MainWindow::~MainWindow() {
    delete ui;  // 释放 UI 资源
}

void MainWindow::setupUI() {
    QWidget *mainWidget = new QWidget(this);
    QGridLayout *mainLayout = new QGridLayout(mainWidget);

    queueButtons.clear();
    parkingSpotButtons.clear();

    int totalSpots = parkingSpotManager.getTotalSpots();
    int cols = 5;
    int rows = (totalSpots + cols - 1) / cols;

    // 初始化等待队列按钮
    for (int i = 0; i < queueManager.getMaxCapacity(); ++i) {
        QPushButton *queueButton = new QPushButton(this);
        queueButton->setFixedSize(100, 50);
        queueButton->setIcon(QIcon(":/images/empty_spot.png"));

        // 设置队列按钮的背景为深灰色
        queueButton->setStyleSheet("background-color: #2E2E2E; color: white;");

        // 队列按钮点击事件，显示队列中车辆信息
        connect(queueButton, &QPushButton::clicked, this, [=]() {
            QQueue<Vehicle> waitingQueue = queueManager.getWaitingQueue();
            if (i < waitingQueue.size()) {
                Vehicle vehicle = waitingQueue.at(i);
                QString message = QString("车牌号: %1\n进入队列时间: %2")
                                      .arg(vehicle.getLicensePlate())
                                      .arg(vehicle.getEntryTime().toString("yyyy-MM-dd hh:mm:ss"));
                QMessageBox::information(this, "等待车辆信息", message);
            } else {
                QMessageBox::information(this, "等待车辆信息", "该位置无等待车辆");
            }
        });

        mainLayout->addWidget(queueButton, i, 0);
        queueButtons.append(queueButton);
    }

    // 初始化停车位按钮
    for (int i = 0; i < totalSpots; ++i) {
        QPushButton *parkSpotButton = new QPushButton(this);
        parkSpotButton->setIcon(QIcon(":/images/empty_spot.png"));

        // 停车位点击事件，显示车辆信息
        connect(parkSpotButton, &QPushButton::clicked, this, [=]() {
            QVector<Vehicle> parkedVehicles = parkingSpotManager.getParkedVehicles();
            if (i < parkedVehicles.size()) {
                Vehicle vehicle = parkedVehicles[i];
                QString message = QString("车牌号: %1\n停车时间: %2")
                                      .arg(vehicle.getLicensePlate())
                                      .arg(vehicle.getEntryTime().toString("yyyy-MM-dd hh:mm:ss"));
                QMessageBox::information(this, "停车位信息", message);
            } else {
                QMessageBox::information(this, "停车位信息", "该车位空置");
            }
        });

        mainLayout->addWidget(parkSpotButton, i / cols, (i % cols) + 1);
        parkingSpotButtons.append(parkSpotButton);
    }

    parkButton = new QPushButton("停车", this);
    releaseButton = new QPushButton("取车", this);
    queryButton = new QPushButton("查询", this);
    aboutButton = new QPushButton("关于", this);

    connect(parkButton, &QPushButton::clicked, this, &MainWindow::onParkButtonClicked);
    connect(releaseButton, &QPushButton::clicked, this, &MainWindow::onReleaseButtonClicked);
    connect(queryButton, &QPushButton::clicked, this, &MainWindow::onQueryButtonClicked);
    connect(aboutButton, &QPushButton::clicked, this, &MainWindow::onAboutButtonClicked);

    mainLayout->addWidget(parkButton, rows, 2);
    mainLayout->addWidget(releaseButton, rows + 1, 2);
    mainLayout->addWidget(queryButton, rows, 4);
    mainLayout->addWidget(aboutButton, rows + 1, 4);

    setCentralWidget(mainWidget);
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    // 动态调整按钮大小
    int availableWidth = centralWidget()->width();
    int availableHeight = centralWidget()->height();
    int cols = 5;
    int rows = (parkingSpotManager.getTotalSpots() + cols - 1) / cols;
    int buttonWidth = availableWidth / (cols + 1);
    int buttonHeight = availableHeight / (rows + 3);

    buttonWidth = qBound(80, buttonWidth, 150);
    buttonHeight = qBound(50, buttonHeight, 150);

    QSize buttonSize(buttonWidth, buttonHeight);
    QSize iconSize(buttonWidth * 0.8, buttonHeight * 0.8);

    for (QPushButton *button : parkingSpotButtons) {
        button->setFixedSize(buttonSize);
        button->setIconSize(iconSize);
    }

    for (QPushButton *button : queueButtons) {
        button->setFixedSize(buttonSize);
        button->setIconSize(iconSize);
    }

    QMainWindow::resizeEvent(event);
}

void MainWindow::updateParkingStatus() {
    QVector<Vehicle> parkedVehicles = parkingSpotManager.getParkedVehicles();
    for (int i = 0; i < parkingSpotButtons.size(); ++i) {
        parkingSpotButtons[i]->setIcon(QIcon(i < parkedVehicles.size() ? ":/images/car_icon.png" : ":/images/empty_spot.png"));
    }
}

void MainWindow::updateQueueStatus() {
    QQueue<Vehicle> waitingQueue = queueManager.getWaitingQueue();
    QSize parkingIconSize = parkingSpotButtons.isEmpty() ? QSize(80, 80) : parkingSpotButtons[0]->iconSize();
    for (int i = 0; i < queueButtons.size(); ++i) {
        queueButtons[i]->setIcon(QIcon(i < waitingQueue.size() ? ":/images/car_icon.png" : ":/images/empty_spot.png"));
        queueButtons[i]->setIconSize(parkingIconSize);
    }
}

void MainWindow::playVehicleAnimation(QPushButton *targetButton, const QString &vehicleIconPath, bool isEntering) {
    if (isAnimating) return; // 如果有动画在进行，跳过

    isAnimating = true; // 标记动画开始

    clearAnimations(); // 清理任何遗留的动画

    QSize iconSize = targetButton->iconSize();
    movingCarLabel = new QLabel(this);

    movingCarLabel->setPixmap(QPixmap(vehicleIconPath).scaled(iconSize, Qt::KeepAspectRatio));
    movingCarLabel->setGeometry(targetButton->x(), isEntering ? this->height() : targetButton->y(), iconSize.width(), iconSize.height());
    movingCarLabel->show();
    movingCarLabel->raise();

    QPropertyAnimation *animation = new QPropertyAnimation(movingCarLabel, "geometry", this);
    animation->setDuration(2000); // 动画时间可以根据需要进行调整

    if (isEntering) {
        animation->setStartValue(QRect(targetButton->x(), this->height(), iconSize.width(), iconSize.height())); // 从窗口底部进入
        animation->setEndValue(targetButton->geometry()); // 到达停车位
    } else {
        animation->setStartValue(targetButton->geometry()); // 从停车位开始
        animation->setEndValue(QRect(targetButton->x(), this->height() + 100, iconSize.width(), iconSize.height())); // 移出窗口之外（+100确保完全移出）
    }

    QSequentialAnimationGroup *animationGroup = new QSequentialAnimationGroup(this);
    animationGroup->addAnimation(animation);

    connect(animationGroup, &QSequentialAnimationGroup::finished, this, [=]() {
        targetButton->setIcon(QIcon(vehicleIconPath)); // 更新车位图标
        movingCarLabel->deleteLater();
        movingCarLabel = nullptr;
        isAnimating = false; // 动画结束，重置标志
        animationGroup->deleteLater();
    });

    animationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void MainWindow::clearAnimations() {
    if (movingCarLabel) {
        movingCarLabel->clear();
        movingCarLabel->deleteLater();
        movingCarLabel = nullptr;
    }
}

void MainWindow::onParkButtonClicked() {
    if (isAnimating) return; // 如果有动画在进行，跳过

    clearAnimations(); // 清理旧动画

    QString licensePlate = QInputDialog::getText(this, "车辆入库", "请输入车牌号:");
    if (licensePlate.isEmpty()) return;

    if (parkingSpotManager.hasVehicle(licensePlate) || queueManager.hasVehicleInQueue(licensePlate)) {
        QMessageBox::warning(this, "检查车牌号错误", "车牌号已存在！");
        return;
    }

    if (parkingSpotManager.isFull()) {
        if (queueManager.isQueueFull()) {
            QMessageBox::warning(this, "排队失败", "等待队列已满！");
        } else {
            queueManager.addVehicleToQueue(Vehicle(licensePlate));
            updateQueueStatus();
            logWindow->addLogMessage(QString("车号 %1 进入了等待队列").arg(licensePlate));
            QMessageBox::information(this, "排队成功", "车辆已进入等待队列.");
        }
    } else {
        int index = parkingSpotManager.getParkedVehicles().size();
        parkVehicleInSpot(Vehicle(licensePlate), index);
        logWindow->addLogMessage(QString("车号 %1 进入了停车场").arg(licensePlate));
        QMessageBox::information(this, "入库成功", "车辆已成功入库.");
    }
}

void MainWindow::onReleaseButtonClicked() {
    if (isAnimating) return; // 如果有动画在进行，跳过

    clearAnimations(); // 清理旧的动画

    QString licensePlate = QInputDialog::getText(this, "车辆出库", "请输入车牌号:");
    if (!licensePlate.isEmpty()) {
        QVector<Vehicle> parkedVehicles = parkingSpotManager.getParkedVehicles();
        for (int i = 0; i < parkedVehicles.size(); ++i) {
            if (parkedVehicles[i].getLicensePlate() == licensePlate) {
                double cost = calculateParkingCost(parkedVehicles[i].getEntryTime());

                // 获取停车位按钮
                QPushButton *parkingButton = parkingSpotButtons[i];
                QSize iconSize = parkingButton->iconSize();

                // **首先立即设置车位为空车位图标**
                parkingButton->setIcon(QIcon(":/images/empty_spot.png"));

                // 延迟播放动画，确保图标移除后再播放动画
                QTimer::singleShot(100, this, [=]() {
                    // 播放车辆出库动画
                    playVehicleAnimation(parkingButton, ":/images/car_icon.png", false);

                    // 动画结束后，重新排列停车位图标
                    QTimer::singleShot(2100, this, [=]() {
                        rearrangeParkingSpots(); // 更新停车位状态

                        // 检查是否有等待车辆可以进入
                        if (!queueManager.isQueueEmpty()) {
                            Vehicle waitingVehicle = queueManager.dequeueVehicle();
                            updateQueueStatus();  // 更新队列状态

                            // 获取下一个空闲车位
                            int nextFreeIndex = parkingSpotManager.getParkedVehicles().size();
                            if (nextFreeIndex < parkingSpotButtons.size()) {
                                // 将排队中的车辆移到下一个空车位，并播放动画
                                parkVehicleInSpot(waitingVehicle, nextFreeIndex);
                            }
                        }
                    });
                });

                // 从停车位中移除车辆
                parkingSpotManager.removeVehicle(licensePlate);

                // 弹出提示消息
                logWindow->addLogMessage(QString("车号 %1 被取出了车库").arg(licensePlate));
                QMessageBox::information(this, "出库成功", QString("车辆已出库，需支付费用：%1 元").arg(cost));

                return;
            }
        }

        // 如果未找到车牌号，给出提示
        QMessageBox::warning(this, "出库失败", "没有找到该车牌号的车辆！");
    }
}

void MainWindow::parkVehicleInSpot(const Vehicle &vehicle, int index) {
    clearAnimations(); // 清理旧动画，防止冲突

    parkingSpotManager.parkVehicle(vehicle); // 停车前的状态更新
    playVehicleAnimation(parkingSpotButtons[index], ":/images/car_icon.png", true); // 播放停车动画
}

void MainWindow::onQueryButtonClicked() {
    QString licensePlate = QInputDialog::getText(this, "查询车辆信息", "请输入车牌号:");
    if (licensePlate.isEmpty()) return;

    QVector<Vehicle> parkedVehicles = parkingSpotManager.getParkedVehicles();
    for (const Vehicle &vehicle : parkedVehicles) {
        if (vehicle.getLicensePlate() == licensePlate) {
            QDateTime entryTime = vehicle.getEntryTime();
            qint64 elapsedSeconds = entryTime.secsTo(QDateTime::currentDateTime());
            double cost = calculateParkingCost(entryTime);
            int hours = elapsedSeconds / 3600;
            int minutes = (elapsedSeconds % 3600) / 60;
            QString message = QString("车牌号: %1\n入库时间: %2\n停留时间: %3 小时 %4 分钟\n当前停车费用: %5 元")
                                  .arg(vehicle.getLicensePlate())
                                  .arg(entryTime.toString("yyyy-MM-dd hh:mm:ss"))
                                  .arg(hours)
                                  .arg(minutes)
                                  .arg(cost);
            QMessageBox::information(this, "车辆信息", message);
            return;
        }
    }
    QMessageBox::warning(this, "查询失败", "停车场中没有找到该车牌号的车辆！");
}

void MainWindow::onAboutButtonClicked() {
    int totalParkingSpots = parkingSpotManager.getTotalSpots();
    int availableParkingSpots = totalParkingSpots - parkingSpotManager.getParkedVehicles().size();
    int totalQueueSpots = queueManager.getMaxCapacity();
    int vehiclesInQueue = queueManager.getWaitingQueue().size();
    QString message = QString("停车场总车位: %1\n停车场空车位: %2\n等待队列总位置: %3\n当前等待车辆: %4")
                          .arg(totalParkingSpots)
                          .arg(availableParkingSpots)
                          .arg(totalQueueSpots)
                          .arg(vehiclesInQueue);
    QMessageBox::information(this, "关于停车场状态", message);
}

double MainWindow::calculateParkingCost(const QDateTime &entryTime) const {
    const double ratePerHour = 5.0;
    qint64 elapsedSeconds = entryTime.secsTo(QDateTime::currentDateTime());
    double hoursParked = static_cast<double>(elapsedSeconds) / 3600.0;
    return ratePerHour * hoursParked;
}

void MainWindow::rearrangeParkingSpots() {
    // 获取当前已停车辆
    QVector<Vehicle> parkedVehicles = parkingSpotManager.getParkedVehicles();

    // 重新按照顺序更新车位图标
    for (int i = 0; i < parkingSpotButtons.size(); ++i) {
        if (i < parkedVehicles.size()) {
            // 有车辆停放时，更新为车的图标
            parkingSpotButtons[i]->setIcon(QIcon(":/images/car_icon.png"));
        } else {
            // 没有车辆时，设置为空车位
            parkingSpotButtons[i]->setIcon(QIcon(":/images/empty_spot.png"));
        }
    }

    // 确保 UI 和内部状态保持一致
    updateParkingStatus();
}
