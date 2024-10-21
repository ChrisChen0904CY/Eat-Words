#include "loadingwindow.h"
#include "ui_loadingwindow.h"

LoadingWindow::LoadingWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoadingWindow)
{
    ui->setupUi(this);
    // Transparent Background Support
    this->setAttribute(Qt::WA_TranslucentBackground, true);
    // Set Full Screen
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->showFullScreen();
}

LoadingWindow::~LoadingWindow()
{
    delete ui;
}

void LoadingWindow::set_label_text(QString text) {
    this->ui->label_2->setText(text);
}
