#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include "meshpairviewerwidget.h"
#include "planegraph.h"
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
    void configure(void);
protected slots:
    void next_plane();
    void last_plane();
    void next_axis();
    void last_axis();
    void rotate_to(double angle);
    void recover_axis(void);
    void recover_angle(void);
protected slots://test
    void show_dash(bool);
    void show_dash_all(bool);
    void test_calc_connected(void);
protected:
    void show_axis(const arma::fvec&);
    void show_side(void);
private:
    Ui::MainWindow *ui;
    MeshPairViewerWidget* geo_view_;
    QStringList input_list_;
    QStringList::Iterator input_current_;
    QDir input_path_;
    QDir output_path_;
    arma::ivec is_dash_;
    std::shared_ptr<PlaneGraph> plane_graph_;
    std::vector<std::pair<int,arma::fvec>>::iterator axis_current_;
    int side_current_;
    arma::Mat<uint8_t> input_current_color_;
    float angle_current_;
};

#endif // MAINWINDOW_H
