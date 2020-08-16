#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qclient.h"

class qclient : public QMainWindow
{
    Q_OBJECT

public:
    qclient(QWidget *parent = Q_NULLPTR);

private:
    Ui::qclientClass ui;
};
