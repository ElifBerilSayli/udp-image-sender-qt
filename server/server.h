#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_server.h"

class server : public QMainWindow
{
    Q_OBJECT

public:
    server(QWidget *parent = Q_NULLPTR);

private:
    Ui::serverClass ui;
};
