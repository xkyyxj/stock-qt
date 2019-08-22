#ifndef ANARESULT_H
#define ANARESULT_H

#include <string>
#include <vector>

#include <QString>
#include <QVariant>

class AnaResult
{
protected:
    std::vector<std::vector<QVariant>> data;
    std::vector<QString> dbColumns;
    std::vector<QString> displayHead;
    QString dbTableName, tableMeta;

    bool isValid;
public:
    AnaResult(QString);

    virtual ~AnaResult();

    virtual void initDataFromDB() = 0;

    void initDataFromDB(QString tableMeta, QString wherePart);

    inline std::vector<std::vector<QVariant>>& getData() noexcept {return data;}

    inline std::vector<QString>& getDBColumns() noexcept {return dbColumns;}

    inline std::vector<QString>& getDisplayHead() noexcept {return displayHead;}

    inline bool getIsValid() noexcept {return isValid;}
};

#endif // ANARESULT_H
