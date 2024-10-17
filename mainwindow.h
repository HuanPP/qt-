#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDateTime>
#include <QVector>
#include <QQueue>
#include <QString>
#include <QGridLayout>
#include <QInputDialog>
#include <QResizeEvent>
#include <QLabel>
#include <QSequentialAnimationGroup>
#include "log.h"  // Include the log header

// Vehicle 类
class Vehicle {
public:
    Vehicle(const QString &licensePlate);
    QString getLicensePlate() const;
    QDateTime getEntryTime() const;

private:
    QString licensePlate;
    QDateTime entryTime;
};

// ParkingSpotManager 类
class ParkingSpotManager {
public:
    ParkingSpotManager(int totalSpots);
    void parkVehicle(const Vehicle &vehicle);
    void removeVehicle(const QString &licensePlate);
    bool isFull() const;
    QVector<Vehicle> getParkedVehicles() const;
    bool hasVehicle(const QString &licensePlate) const;
    int getTotalSpots() const { return totalSpots; }

private:
    int totalSpots;
    QVector<Vehicle> parkingSpots;
};

// QueueManager 类
class QueueManager {
public:
    QueueManager(int maxCapacity);
    void addVehicleToQueue(const Vehicle &vehicle);
    Vehicle dequeueVehicle();
    bool isQueueEmpty() const;
    bool isQueueFull() const;
    QQueue<Vehicle> getWaitingQueue() const;
    bool hasVehicleInQueue(const QString &licensePlate) const;
    int getMaxCapacity() const { return maxCapacity; }

private:
    QQueue<Vehicle> waitingQueue;
    int maxCapacity;
};

// MainWindow 类
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    LogWindow *logWindow;  // Add the log window as a member

    ParkingSpotManager parkingSpotManager;
    QueueManager queueManager;

    QVector<QPushButton*> parkingSpotButtons;
    QVector<QPushButton*> queueButtons;
    QLabel *movingCarLabel = nullptr;

    QPushButton *parkButton;
    QPushButton *releaseButton;
    QPushButton *queryButton;
    QPushButton *aboutButton;

    bool isAnimating; // 用于避免动画冲突

    void setupUI();
    void updateParkingStatus();
    void updateQueueStatus();
    double calculateParkingCost(const QDateTime &entryTime) const;
    void parkVehicleInSpot(const Vehicle &vehicle, int index);
    void playVehicleAnimation(QPushButton *targetButton, const QString &vehicleIconPath, bool isEntering);
    void rearrangeParkingSpots();
    void clearAnimations();

private slots:
    void onParkButtonClicked();
    void onReleaseButtonClicked();
    void onQueryButtonClicked();
    void onAboutButtonClicked();
};

#endif // MAINWINDOW_H
