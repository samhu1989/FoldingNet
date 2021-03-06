#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include "common.h"
#include "designtomesh.h"
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
    if(!g_config)g_config.reset(new Config("./Default.config"));
    if(!g_config->has("Configure"))g_config->reload("../Default.config");
    if(!g_config->has("Configure"))
    {
        std::cerr<<"Can not find a Configuration File at local path ./Default.config"<<std::endl;
        std::cerr<<"Please create one and define the parameter 'Configure' in it"<<std::endl;
        return -1;
    }
    std::cerr<<"out_path:"<<out_path<<std::endl;
    DesignToMesh d2m;
    if(!d2m.configure(g_config))
    {
        std::cerr<<"Missing Critical Parameters in configure file please check it"<<std::endl;
        return -1;
    }
    if(!f_name.empty())
    {
        g_parameters.InputFilePath = f_name;
    }
    d2m.ReadLinesFromTxt();
    d2m.Rasterization();
    d2m.FindLayoutByFloodFill();
    d2m.save_flooding_map(out_path);
    d2m.generate_mesh();
    d2m.save_mesh(out_path);
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
