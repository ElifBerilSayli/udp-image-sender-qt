#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qqserver.h"

class qqserver : public QMainWindow
{
    Q_OBJECT

public:
    qqserver(QWidget *parent = Q_NULLPTR);

private:
    Ui::qqserverClass ui;
};
