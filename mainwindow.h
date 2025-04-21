#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QImage>
#include <QLabel>
#include <QDebug>
#include <opencv2/opencv.hpp>
#include "ui_mainwindow.h"
#include <opencv2/objdetect.hpp>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startCamera();
    void stopCamera();
    void captureFrame();
    void saveCurrentFrame();
    void startRecording();
    void stopRecording();
private:
    Ui::MainWindow *ui;
    QTimer *timer;
    cv::VideoCapture cap;
    bool isGray = false;
    bool isDetecting = false;
    cv::CascadeClassifier face_cascade;
    bool isRecording = false;
    cv::Mat lastFrame;
    cv::VideoWriter writer;


};

#endif // MAINWINDOW_H
