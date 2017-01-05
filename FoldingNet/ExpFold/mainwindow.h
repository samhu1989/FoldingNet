#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "meshpairviewerwidget.h"
#include "plate.h"
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
    bool load(void){get_input_lst();return !input_list_.empty();}
public slots:
    void load_current();
    void save_current();
    void next_input();
    void last_input();
    void get_input_lst();
protected slots:
    void next_plane();
    void last_plane();
    void next_axis();
    void last_axis();
    void rotate_to(double angle);
    void recover_plates(const DefaultMesh& mesh);
private:
    Ui::MainWindow *ui;
    MeshPairViewerWidget* geo_view_;
    QStringList input_list_;
    QStringList::Iterator input_current_;
    QDir input_path_;
    QDir output_path_;
    std::vector<Plate::Ptr> current_plate_;
};

#endif // MAINWINDOW_H
