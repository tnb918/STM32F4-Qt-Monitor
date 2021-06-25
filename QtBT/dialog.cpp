#include "dialog.h"
#include "ui_dialog.h"

#include <QStringListModel>
#include <QMessageBox>
#include <QListWidgetItem>


float temp, fax, fay, faz;
int ax, ay, az, gx, gy, gz, w;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    this->showMaximized();

    ui->BtnLink->setEnabled(false);
    ui->tabWidget->setCurrentIndex(0);

    connect(blt,SIGNAL(emit_refresh()),this,SLOT(blt_Refresh()));
    connect(blt->discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(bltDevDiscover(QBluetoothDeviceInfo)));
    connect(blt->discoveryAgent,SIGNAL(finished()),this,SLOT(scanFinshed()));

    //////////////////////////////////////

    setupRealtimeDataDemo(ui->CustomPlot_faxyz);
    setupRealtimeDataDemo_1(ui->CustomPlot_temp);

    ui->CustomPlot_faxyz->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom | QCP::iMultiSelect);   //可拖拽+可滚轮缩放
    //ui->customPlot->replot();
    ui->CustomPlot_faxyz->yAxis->setVisible(true);
    ui->CustomPlot_faxyz->yAxis2->setVisible(true);
    connect(ui->CustomPlot_faxyz->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->CustomPlot_faxyz->yAxis2, SLOT(setRange(QCPRange)));//左右y轴同步放缩
    ui->CustomPlot_faxyz->yAxis->setRange(-90, 90);




    ui->CustomPlot_temp->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom | QCP::iMultiSelect);   //可拖拽+可滚轮缩放
    //ui->customPlot->replot();
    ui->CustomPlot_temp->yAxis->setVisible(true);
    ui->CustomPlot_temp->yAxis2->setVisible(true);
    connect(ui->CustomPlot_temp->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->CustomPlot_temp->yAxis2, SLOT(setRange(QCPRange)));//左右y轴同步放缩
    ui->CustomPlot_temp->yAxis->setRange(0, 40);


    ui->chk_fax->setChecked(true);
    ui->chk_fay->setChecked(true);
    ui->chk_faz->setChecked(true);
    ui->chk_temp->setChecked(true);

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::FrameProc(QString &str)
{
    //处理数据帧:T:28.5,A:  -166   -374  16468,G:     1      0     -1,F: -0.5   1.3 -118.4,W:0
    //处理参数帧:P1:30,P2:7,P3:30,P4:100
    if(str[0] == 'T' && str.length() >= 60)
    {

        QStringList list = str.split(QRegExp("[:, ]"), QString::SkipEmptyParts);

        for(int i = 0;i < list.size(); ++i)
        {
            if(list[i] == "T")
            {
                temp = list[i + 1].toFloat();
                ++i;
            }
            else if(list[i] == "A")
            {
                ax = list[i + 1].toInt();
                ay = list[i + 2].toInt();
                az = list[i + 3].toInt();
                i += 3;
            }
            else if(list[i] == "G")
            {
                gx = list[i + 1].toInt();
                gy = list[i + 2].toInt();
                gz = list[i + 3].toInt();
                i += 3;
            }
            else if(list[i] == "F")
            {
                fax = list[i + 1].toFloat();
                fay = list[i + 2].toFloat();
                faz = list[i + 3].toFloat();
                i += 3;
            }
            else if(list[i] == "W")
            {
                w = list[i + 1].toUInt();
                ++i;
            }
        }

        ui->edit_temp->setText(QString::number(temp) + "℃");
        ui->edit_ax->setText(QString::number(ax));
        ui->edit_ay->setText(QString::number(ay));
        ui->edit_az->setText(QString::number(az));
        ui->edit_gx->setText(QString::number(gx));
        ui->edit_gy->setText(QString::number(gy));
        ui->edit_gz->setText(QString::number(gz));
        ui->edit_pitch->setText(QString::number(fax) + "°");
        ui->edit_roll->setText(QString::number(fay) + "°");
        ui->edit_yaw->setText(QString::number(faz) + "°");
        switch(w)
        {
        default:
            ui->edit_warn->setText("无");
            break;
        case 1:
            ui->edit_warn->setText("温度报警");
            break;
        case 2:
            ui->edit_warn->setText("震动报警");
            break;
        case 3:
            ui->edit_warn->setText("温度、震动报警");
            break;
        }
    }
    else if(str.startsWith("P1:") && str.length() >= 20)//参数帧
    {
        int pos = str.indexOf("P1:");
        QString tst;
        int end;
        if(pos >= 0)
        {
            tst = str.mid(pos + 3);
            end = tst.indexOf(',');
            if(end > 0)
            {
                int templmt = tst.left(end).toInt();
                ui->spin_templmt->setValue(templmt);
            }
        }
        pos = str.indexOf("P2:");
        if(pos >= 0)
        {
            tst = str.mid(pos + 3);
            end = tst.indexOf(',');
            if(end > 0)
            {
                int mpustep = tst.left(end).toInt();
                ui->cmb_mpustep->setCurrentIndex(mpustep);
            }
        }
        pos = str.indexOf("P3:");
        if(pos >= 0)
        {
            tst = str.mid(pos + 3);
            end = tst.indexOf(',');
            if(end > 0)
            {
                int warntime = tst.left(end).toInt();
                ui->spin_warntime->setValue(warntime);
            }
        }
        pos = str.indexOf("P4:");
        if(pos >= 0)
        {
            tst = str.mid(pos + 3);
            int upstep = tst.toInt();
            ui->spin_upstep->setValue(upstep / 1000.0);
        }
    }
}

static const QLatin1String serviceUuid("00001101-0000-1000-8000-00805F9B34FB");

void Dialog::on_BtnLink_clicked()
{
    ui->BtnScan->setText("扫描蓝牙设备");

    if (selItem != NULL)
    {
        QString text = selItem->text();
        int index = text.indexOf(' ');
        if(index == -1){
            return;
        }

        ui->tabWidget->setCurrentIndex(1);

        QBluetoothAddress Address(text.left(index));
        QString name(text.mid(index + 1));
        QString temp = QString("%1").arg(Address.toString());
        ui->editRecv->append(QStringLiteral("你链接的蓝牙地址是：")+temp);
        ui->editRecv->append(QStringLiteral("你链接的蓝牙名字：")+name);
        blt->socket->connectToService(Address, QBluetoothUuid(serviceUuid) ,QIODevice::ReadWrite);
        connect(blt->socket,SIGNAL(connected()),this,SLOT(bltConnected()));
    }
}

void Dialog::on_BtnScan_clicked()
{
    ui->BtnScan->setText("开始扫描...");
    ui->listWidget->clear();
    ui->BtnLink->setEnabled(false);
    blt->bltScan();
}

void Dialog::bltDevDiscover(const QBluetoothDeviceInfo &info)
{
    QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());
    QList<QListWidgetItem *> items = ui->listWidget->findItems(label, Qt::MatchExactly);
    if (items.empty() && !(info.name().isEmpty())) {
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = blt->localDevice->pairingStatus(info.address());
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
            item->setForeground(QColor(Qt::blue));
        else
            item->setForeground(QColor(Qt::black));

        ui->listWidget->addItem(item);
    }
}

void Dialog::scanFinshed()
{
    ui->BtnScan->setText("扫描完成！");
}

void Dialog::bltConnected()
{
    ui->editRecv->append(QStringLiteral("成功链接！"));
    connect(blt->socket,SIGNAL(readyRead()),this,SLOT(bltReadMsg()));
    blt_connected  = true;

    ui->tabWidget->setCurrentIndex(1);
}

void Dialog::bltReadMsg()
{
    static QString oldString;//保存未处理完的数据
    QByteArray get = blt->socket->readAll();

    ////////////////////////////////////////////////
    /// 接收数据处理
    QString strRecv = QString(get);
    int pos = strRecv.indexOf('\n');
    if(pos >= 0)
    {
        oldString += strRecv.left(pos + 1);
        //完整帧数据处理
        FrameProc(oldString);
        //存储剩余数据
        strRecv = strRecv.right(strRecv.length() - pos - 1);
        while(true)
        {
            pos = strRecv.indexOf('\n');
            if(pos < 0)
            {
                //已经处理结束
                oldString = strRecv;
                break;
            }
            oldString = strRecv.left(pos + 1);
            //完整帧数据处理
            FrameProc(oldString);
            //存储剩余数据
            strRecv = strRecv.right(strRecv.length() - pos - 1);
        }
    }
    else
        oldString += strRecv;

    ////////////////////////////////////////////////

    QTextCursor cursor = ui->editRecv->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->editRecv->setTextCursor(cursor);
    if (ui->chkHexRecv->isChecked())
    {
        QString tstr, t;
        for (int i = 0; i < get.size(); ++i)
        {
            t.sprintf("%02X ", get.data()[i]);
            tstr += t;
        }
        ui->editRecv->insertPlainText(tstr);
    }
    else
        ui->editRecv->insertPlainText(QString(get));
}

void Dialog::blt_Refresh()
{

}

void Dialog::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->BtnLink->setEnabled(true);
    selItem = item;
}

void Dialog::on_BtnSend_clicked()
{
    QMessageBox *msg_box= new QMessageBox(QMessageBox::NoIcon,QStringLiteral("注意"),QStringLiteral("请链接BLT"));

    if(blt_connected ==true)
    {
        QString data = ui->editSend->toPlainText();
        QByteArray tba;
        if (ui->chkHexSend->isChecked())
            tba = HexStringToByteArray(data);
        else
            tba = data.toLatin1();
        blt->socket->write(tba);
    }
    else
        msg_box->exec();
}

void Dialog::on_BtnClearRecv_clicked()
{
    ui->editRecv->clear();
}

QByteArray Dialog::HexStringToByteArray(QString HexString)
{
    bool ok;
    QByteArray ret;
    HexString = HexString.trimmed();
    HexString = HexString.simplified();
    QStringList sl = HexString.split(" ");

    foreach (QString s, sl) {
        if(!s.isEmpty())
        {
            uint32_t td = s.toUInt(&ok, 16);
            int pos = ret.size();
            if (ok)
            {
                do {
                    ret.insert(pos, td & 0xFF);
//                    ret.append(td & 0xFF);
                    td >>= 8;
                }while(td > 0);
            }
        }
    }
    qDebug()<<ret;
    return ret;
}

void Dialog::on_btn_readpara_clicked()
{
    if(blt_connected)
        blt->socket->write(QString("QPAR\n").toLatin1());
}

void Dialog::on_btn_setpara_clicked()
{
    if(blt_connected)
    {
        QString tstr = QString("P1:%1,P2:%2,P3:%3,P4:%4\n")
                .arg(ui->spin_templmt->value())
                .arg(ui->cmb_mpustep->currentIndex())
                .arg(ui->spin_warntime->value())
                .arg((int)(ui->spin_upstep->value()*1000));
        blt->socket->write(tstr.toLatin1());
    }
}

void Dialog::on_pushButton_clicked()
{
    if(blt_connected)
        blt->socket->write(QString("IFUP\n").toLatin1());
}

//< 画图初始化
void Dialog::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::blue));
    customPlot->graph(0)->setName("俯仰角");

    customPlot->addGraph();
    customPlot->graph(1)->setPen(QPen(Qt::red));
    customPlot->graph(1)->setName("横滚角");

    customPlot->addGraph();
    customPlot->graph(2)->setPen(QPen(Qt::green));
    customPlot->graph(2)->setName("航向角");


    QSharedPointer<QCPAxisTickerDateTime> dateTick(new QCPAxisTickerDateTime);
    dateTick->setDateTimeFormat("HH:mm:ss");
    ui->CustomPlot_faxyz->xAxis->setTicker(dateTick);

    //   customPlot->axisRect()->setupFullAxesBox();

    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(50); // Interval 0 means to refresh as fast as possible
    customPlot->legend->setVisible(true);
}

void Dialog::setupRealtimeDataDemo_1(QCustomPlot *customPlot)
{
    customPlot->addGraph();
    customPlot->graph(0)->setPen(QPen(Qt::red));
    customPlot->graph(0)->setName("温度");

    QSharedPointer<QCPAxisTickerDateTime> dateTick(new QCPAxisTickerDateTime);
    dateTick->setDateTimeFormat("HH:mm:ss");
    ui->CustomPlot_temp->xAxis->setTicker(dateTick);

    //   customPlot->axisRect()->setupFullAxesBox();

    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(50); // Interval 0 means to refresh as fast as possible
    customPlot->legend->setVisible(true);
}

void Dialog::realtimeDataSlot()
{
    //key的单位是秒
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
    qsrand(QTime::currentTime().msec() + QTime::currentTime().second() * 10000);
    //使用随机数产生两条曲线
    double value0 = fax;
    double value1 = fay;
    double value2 = faz;
    double value3 = temp;
    if (ui->chk_fax->isChecked())
    {
        ui->CustomPlot_faxyz->graph(0)->addData(key, value0);//添加数据1到曲线1
        ui->CustomPlot_faxyz->graph(0)->setPen(QPen(Qt::blue));
    }
    else{
        ui->CustomPlot_faxyz->graph(0)->setPen(QPen(Qt::transparent));
    }

    if (ui->chk_fay->isChecked())
    {
        ui->CustomPlot_faxyz->graph(1)->addData(key, value1);//添加数据1到曲线1
        ui->CustomPlot_faxyz->graph(1)->setPen(QPen(Qt::red));
    }
    else{
        ui->CustomPlot_faxyz->graph(1)->setPen(QPen(Qt::transparent));
    }
    if (ui->chk_faz->isChecked())
    {
        ui->CustomPlot_faxyz->graph(2)->addData(key, value2);//添加数据1到曲线1
        ui->CustomPlot_faxyz->graph(2)->setPen(QPen(Qt::green));
    }
    else{
        ui->CustomPlot_faxyz->graph(2)->setPen(QPen(Qt::transparent));
    }

    if (ui->chk_temp->isChecked())
    {
        ui->CustomPlot_temp->graph(0)->addData(key, value3);//添加数据1到曲线1
        ui->CustomPlot_temp->graph(0)->setPen(QPen(Qt::red));
    }
    else{
        ui->CustomPlot_temp->graph(0)->setPen(QPen(Qt::transparent));
    }

    //自动设定graph(1)曲线y轴的范围，如果不设定，有可能看不到图像
    //也可以用ui->customPlot->yAxis->setRange(up,low)手动设定y轴范围
    //ui->customPlot->graph(0)->rescaleValueAxis();
    //ui->customPlot->graph(1)->rescaleValueAxis(true);

    //这里的8，是指横坐标时间宽度为8秒，如果想要横坐标显示更多的时间
    //就把8调整为比较大到值，比如要显示60秒，那就改成60。
      ui->CustomPlot_faxyz->xAxis->setRange(key+0.25, 20, Qt::AlignRight);//设定x轴的范围

      ui->CustomPlot_temp->xAxis->setRange(key+0.25, 20, Qt::AlignRight);//设定x轴的范围

    ui->CustomPlot_faxyz->replot();
    ui->CustomPlot_temp->replot();
}
