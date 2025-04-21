#include "mainwindow.h"
#include <QVBoxLayout>
#include <QPixmap>
#include <QDateTime>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      timer(new QTimer(this))
{
    ui->setupUi(this);
    QString btnStyle = R"(
        QPushButton {
            min-width: 110px;
            min-height: 36px;
            font-family:  'Arial', sans-serif;
            border-radius: 10px;
            padding: 6px 14px;
            font-size: 13px;
            font-weight: bold;
        }
    )";
    ui->detectButton->setStyleSheet("background-color: royalblue; color: white;" + btnStyle);
    ui->grayButton->setStyleSheet("background-color: gold; color: white;" + btnStyle);
    ui->startRecordButton->setStyleSheet("background-color: crimson; color: white;" + btnStyle);
    ui->stopRecordButton->setStyleSheet("background-color: red; color: white;" + btnStyle);
    ui->saveButton->setStyleSheet("background-color: deepskyblue; color: white;" + btnStyle);
    ui->stopButton->setStyleSheet("background-color: #D32F2F; color: white;" + btnStyle);
    ui->startButton->setStyleSheet("background-color: green; color: white;" + btnStyle);

    ui->grayButton->setVisible(false);
    ui->detectButton->setVisible(false);
    ui->saveButton->setVisible(false);
    ui->stopButton->setVisible(false);
    ui->stopRecordButton->setVisible(false);
    ui->startRecordButton->setVisible(false);


    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        qDebug() << "Face cascade cant work!";
    }
    // Camera opening
    cap.open(0);

    if (!cap.isOpened()){
        ui->label->setText("Camera cannot be opened.");
        return;
    }
    // SIGNAL-SLOTS
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::startCamera);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopCamera);
    connect(ui->detectButton, &QPushButton::clicked, this, [this]() {
        isDetecting = !isDetecting;
        ui->detectButton->setText(isDetecting ? "DETECT ON" : "DETECT FACE");
    });
    connect(timer, &QTimer::timeout, this, &MainWindow::captureFrame);
    connect(ui->grayButton, &QPushButton::clicked, this, [this]() {
        isGray = !isGray;
    });
    connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::saveCurrentFrame);
    connect(ui->startRecordButton, &QPushButton::clicked, this, &MainWindow::startRecording);
    connect(ui->stopRecordButton, &QPushButton::clicked, this, &MainWindow::stopRecording);

}

MainWindow::~MainWindow()
{
    if (cap.isOpened()){
        cap.release();
    }
}

void MainWindow::startCamera()
{
    ui->saveButton->setVisible(true);
    ui->startRecordButton->setVisible(true);
    ui->stopRecordButton->setVisible(false);
    ui->stopButton->setVisible(true);
    ui->grayButton->setVisible(true);
    ui->detectButton->setVisible(true);
    ui->label->setMinimumSize(800, 600);
    ui->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        cap.open(0);
        if (!cap.isOpened())  {
            ui->label->setText("Camera cannot be opened.");
            return;
        }

    timer->start(30);
    ui->startButton->hide();

}

void MainWindow::stopCamera()
{
    timer->stop();
    ui->stopButton->setVisible(false);
    cap.release();
    ui->startButton->show();
    ui->grayButton->setVisible(false);
    ui->detectButton->setVisible(false);
    ui->startRecordButton->setVisible(false);
    ui->stopRecordButton->setVisible(false);
    ui->saveButton->setVisible(false);
    ui->label->clear();
    ui->label->setText("Camera stopped");
    isGray = false;
}
void MainWindow::saveCurrentFrame()
{
    if (lastFrame.empty()) {
        qDebug() << "Warning: There is no image for recording.";
         return;
    }
    // Timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filename = QString("capture_%1.png").arg(timestamp);

    cv::Mat bgr;
    cv::cvtColor(lastFrame, bgr, cv::COLOR_RGB2BGR);


      // Record
      if (cv::imwrite(filename.toStdString(), bgr)){
          qDebug() << "Image saved:" << filename;
      } else {
          qDebug() << "Error: Image cannot be saved.";
      }
}


void MainWindow::captureFrame()
{
    cv::Mat frame;
    cap >> frame;
    if (frame.empty()) return;

    if (isGray) {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
        cv::cvtColor(frame, frame, cv::COLOR_GRAY2RGB);
    } else {
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
    }
    // Face Detection
    if (isDetecting) {
        std::vector<cv::Rect> faces;
        cv::Mat gray;
        cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
        face_cascade.detectMultiScale(gray, faces);

        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(255, 0, 0), 4);
            cv::putText(frame, "Face", cv::Point(face.x, face.y - 10),
                        cv::FONT_HERSHEY_SIMPLEX, 1.0,
                        cv::Scalar(255, 0, 0), 2);
        }
    }

    //  Timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    std::string timeText = timestamp.toStdString();
    int x = 20;
    int y = 30;

    cv::putText(frame, timeText, cv::Point(x + 2, y + 2),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 0), 3);
    cv::putText(frame, timeText, cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);

    //  Record last frame
    lastFrame = frame.clone();

    // Recording
    if (isRecording && writer.isOpened()) {
        cv::Mat bgr;
        cv::cvtColor(lastFrame, bgr, cv::COLOR_RGB2BGR);
        writer.write(bgr);
    }
    if (isRecording) {
        QString recText = "RECORDING";
        std::string text = recText.toUtf8().constData();

        int baseline = 0;
        cv::Size textSize = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.9, 2, &baseline);

        int x = frame.cols - textSize.width - 20;
        int y = textSize.height + 20;

        //Shadow
        cv::putText(frame, text, cv::Point(x + 2, y + 2),
                    cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 0, 0), 4);

        //Text
        cv::putText(frame, text, cv::Point(x, y),
                    cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(250, 0, 0), 2);
    }

    QImage qimg((const uchar*)frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);
    ui->label->setPixmap(QPixmap::fromImage(qimg).scaled(ui->label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


void MainWindow::startRecording()
{
    ui->startRecordButton->hide();
    layout()->update();
    isRecording = true;
    ui->stopRecordButton->setVisible(true);
    ui->stopRecordButton->setEnabled(true);

    int width = cap.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = 30.0;

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    if (timestamp.isEmpty()) {
        qDebug() << "Error: Timestamp cannot be generated!";
        return;
    }

    QString filename = QString("record_%1.avi").arg(timestamp);
    std::string outputFile = filename.toStdString();

    // Record
    writer.open(outputFile, cv::VideoWriter::fourcc('M','J','P','G'), fps, cv::Size(width, height));

    if (!writer.isOpened()) {
        isRecording = false;
        ui->startRecordButton->setVisible(true);
        ui->stopRecordButton->setVisible(false);
    }

}

 void MainWindow::stopRecording()
 {
     isRecording = false;
     writer.release();
     ui->startRecordButton->setVisible(true);
     ui->stopRecordButton->setVisible(false);
 }
