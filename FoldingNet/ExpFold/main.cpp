#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include "FoldingNet.h"
#include "common.h"
int print_usage(int argc, char *argv[] )
{
    std::cerr << "Usage:"<<std::endl
              <<argv[0]<<" [-h] -m w " << std::endl;
    std::cerr << "Options" <<std::endl
              << "  -h\n"
              << "  Print this message\n"
              << "  -m :c Use console mode\n"
              << "      w Use ui mode.\n"
              << "  under console mode:\n"
              << "  -o : output path\n"
              << "  -f : input file name(overide the parameters)\n";

   return 0;
}

int uiMain(int argc, char *argv[])
{
    int ch;
    QString in_path,out_path;
    while( ( ch = getopt(argc,argv,"hm:i:n:c:o:d") ) != -1 )
    {
        switch(ch)
        {
        case 'i':
            in_path = QString::fromStdString(std::string(optarg));
            break;
        case 'o':
            out_path = QString::fromStdString(std::string(optarg));
            break;
        default:
            ;
        }
    }
    if(in_path.isEmpty()||out_path.isEmpty())
    {
        return print_usage(argc,argv);
    }
    QApplication::setColorSpec( QApplication::CustomColor );
    QApplication::addLibraryPath("./qtplugins/");
    QApplication::addLibraryPath("./bin/qtplugins/");
    init_resouce();
    QApplication a(argc, argv);
    MainWindow* w = new MainWindow();
    w->setInputPath(in_path);
    w->setOutputPath(out_path);
    w->show();
    //load have to be used after show so that the set_center_ won't be override  by the one in initializeGL()
    if(!w->load())return -1;
    return a.exec();
}

int cMain(int argc, char *argv[])
{
    int ch;
    std::string out_path;
    std::string f_name;
    while( ( ch = getopt(argc,argv,"hm:i:n:c:o:f:d") ) != -1 )
    {
        switch(ch)
        {
        case 'f':
            f_name = std::string(optarg);
            break;
        case 'o':
            out_path = std::string(optarg);
            break;
        default:
            ;
        }
    }
    if(out_path.empty())
    {
        return -1;
    }
    std::cerr<<"out_path:"<<out_path<<std::endl;
    FoldingNet foldingPaper;
    foldingPaper.LoadParameters();
    if(!f_name.empty())
    {
        g_parameters.InputFilePath = f_name;
    }
    foldingPaper.ReadLinesFromTxt();
    if(foldingPaper.FindPolygonByFloodFill())
    {
        foldingPaper.Modeling();
        foldingPaper.save_mesh(out_path);
    }else{
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
        glutInitWindowPosition(100, 100);
        glutInitWindowSize(800, 800);
        glutCreateWindow("What's Missing?");
        glutDisplayFunc(&foldingPaper.ShowPlane);
        glutMainLoop();
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int ch;
    opterr=0;
    bool worked(false);
    while( ( ch = getopt(argc,argv,"hm:") ) !=-1 )
    {
        switch(ch)
        {
        case 'h':
            worked = true;
            return print_usage(argc,argv);
        case 'm':
            if(optarg[0]=='c')
            {
                int r = cMain(argc,argv);
                std::cerr<<"cMain return:"<<r<<std::endl;
                if( 0 != r )print_usage(argc,argv);
                return r;
            }
            else if(optarg[0]=='w'){
                int r = uiMain(argc,argv);
                if(0!=r)print_usage(argc,argv);
                return r;
            }
            break;
        default:
            ;
        }
    }
    if(!worked)
    {
        return print_usage(argc,argv);
    }
    return 0;
}
