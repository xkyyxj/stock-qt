#ifndef COMMONANARESULT_H
#define COMMONANARESULT_H

#include "anaresult.h"

class CommonAnaResult: public AnaResult
{
public:
    CommonAnaResult(QString);

    void initDataFromDB();
};

#endif // COMMONANARESULT_H
