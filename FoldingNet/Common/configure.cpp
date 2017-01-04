#include "configure.h"
#include <QString>
#include <QTextStream>
#include <QFile>
#include <iostream>
Config::Config(const std::string& path)
{
    reload(path);
}

void Config::reload()
{
    QFile inFile(QString::fromStdString(_SourcePath));
    inFile.open(inFile.ReadOnly);
    QTextStream stream(&inFile);
    while(!stream.atEnd())
    {
        QString line = stream.readLine();
        QTextStream line_stream(&line);
        QString str0;
        line_stream >> str0;
        if(str0.startsWith("#"))
        {
            continue;
        }
        QString str1 = line.remove(0,str0.size());
        str1 = str1.simplified();
        _Config.insert(str0.toStdString(),str1.toStdString());
    }
}

void Config::reload(const std::string& path)
{
    _SourcePath = path;
    reload();
}

void Config::add(const std::string& key,const std::string& value)
{
    _Config.insert(key,value);
}

bool Config::has(const std::string& key)
{
    if(_Config.contains(key))
    {
        return true;
    }else{
        std::cerr<<"Missing Config: "<<key;
    }
    return false;
}

std::string Config::getString(const std::string& key)
{
    return _Config.value(key);
}

int Config::getInt(const std::string& key)
{
    QString str = QString::fromStdString(_Config.value(key));
    bool ok;
    int result = str.toInt(&ok);
    if( !ok )std::cerr<<"Can't convert "+key+" to Int";
    return result;
}

float Config::getFloat(const std::string& key)
{
    QString str = QString::fromStdString(_Config.value(key));
    bool ok;
    float result = str.toFloat(&ok);
    if( !ok )std::cerr<<"Can't convert "+key+" to Float";
    return result;
}

void Config::getFloatVec(
        const std::string& key,
        std::vector<float>& vec
        )
{
    QString str = QString::fromStdString(_Config.value(key));
    QTextStream stream(&str);
    while(!stream.atEnd())
    {
        QString num;
        stream >> num;
        bool ok;
        vec.push_back(num.toFloat(&ok));
        if(!ok)std::cerr<<"Can't convert "+num.toStdString()+" to Float";
    }
}

double  Config::getDouble(const std::string& key)
{
    QString str = QString::fromStdString(_Config.value(key));
    bool ok;
    double result = str.toDouble(&ok);
    if( !ok )std::cerr<<"Can't convert "+key+" to Double";
    return result;
}

Config::~Config()
{

}

