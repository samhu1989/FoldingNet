#ifndef CONFIGURE_H
#define CONFIGURE_H
#include "common_global.h"
#include <string>
#include <memory>
#include <QMap>
#include <vector>
class COMMONSHARED_EXPORT Config
{
public:
    typedef std::shared_ptr<Config> Ptr;
    Config(const std::string&);
    ~Config();

    void reload(const std::string&);
    void reload();
    void add(const std::string&,const std::string&);

    bool has(const std::string&);
    std::string getString(const std::string&);
    int getInt(const std::string&);
    float   getFloat(const std::string&);
    double  getDouble(const std::string&);
    void getFloatVec(
            const std::string&,
            std::vector<float>&
            );
private:
    QMap<std::string,std::string>_Config;
    std::string _SourcePath;
};

#endif // CONFIGURE_H
