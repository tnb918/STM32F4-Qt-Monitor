#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include "blt.h"
#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();

    void FrameProc(QString &str);

    //设置qcustomplot画图属性，实时
    void setupRealtimeDataDemo(QCustomPlot *customPlot);
    void setupRealtimeDataDemo_1(QCustomPlot *customPlot);

private slots:
    void on_BtnLink_clicked();
    void on_BtnScan_clicked();

//    void blt_if_Discover();
    void bltDevDiscover(const QBluetoothDeviceInfo &info);
    void scanFinshed();
    void bltConnected();
    void bltReadMsg();
    void blt_Refresh();
//    void bltColsed();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_BtnSend_clicked();

    void on_BtnClearRecv_clicked();
    QByteArray HexStringToByteArray(QString HexString);

    void on_btn_readpara_clicked();

    void on_btn_setpara_clicked();

    void on_pushButton_clicked();

    void realtimeDataSlot();

private:
    Ui::Dialog *ui;
    BLT *blt = new BLT();
    bool blt_connected = false;
    QListWidgetItem *selItem = NULL;
    //定时器，周期调用realtimeDataSlot()槽，实现动态数据添加到曲线
    QTimer dataTimer;
    bool autoScroll;
};
#endif // DIALOG_H
