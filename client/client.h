#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_client.h"

class client : public QMainWindow
{
    Q_OBJECT

public:
    client(QWidget *parent = Q_NULLPTR);

private:
    Ui::clientClass ui;
};
