#ifndef COMMONANARESULT_H
#define COMMONANARESULT_H

#include "anaresult.h"

class CommonAnaResult: public AnaResult
{
    QString filter;
public:
    CommonAnaResult(QString);

    void setFilter(QString filter) noexcept;

    void initDataFromDB();
};

#endif // COMMONANARESULT_H
