#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include "FoldingNet.h"
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
              << "  -g : generate scene\n"
              << "  -p : convert to ply\n"
              << "  -i : input path with -g\n"
              << "     : input vertex with -p"
              << "  -n : input vertex normal\n"
              << "  -c : input vertex color\n"
              << "  -o : output file\n";

   return 0;
}

int uiMain(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

int cMain(int argc, char *argv[])
{
    int ch;
    std::string out_name;
    while( ( ch = getopt(argc,argv,"hm:i:n:c:o:d") ) != -1 )
    {
        switch(ch)
        {
        case 'o':
            out_name = std::string(optarg);
            break;
        default:
            ;
        }
    }
    FoldingNet foldingPaper;
    foldingPaper.LoadParameters();
    foldingPaper.ReadLinesFromTxt();
    foldingPaper.FindPolygonByFloodFill();
    foldingPaper.Modeling();
    foldingPaper.save_mesh(out_name);
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
            if(optarg[0]=='c')return cMain(argc,argv);
            else if(optarg[0]=='w') return uiMain(argc,argv);
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
