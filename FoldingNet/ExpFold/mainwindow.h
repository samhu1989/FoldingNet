#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "meshpairviewerwidget.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void setInputPath(const QString &path){input_path_.setPath(path);}
    void setOutputPath(const QString &path){output_path_.setPath(path);}
    void load(void){get_input_lst();}
public slots:
    void next_input();
    void last_input();
    void get_input_lst();
private:
    Ui::MainWindow *ui;
    MeshPairViewerWidget* geo_view_;
    QStringList input_list_;
    QStringList::Iterator input_current_;
    QDir input_path_;
    QDir output_path_;
};

#endif // MAINWINDOW_H
