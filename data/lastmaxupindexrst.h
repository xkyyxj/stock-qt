#ifndef LASTMAXUPINDEXRST_H
#define LASTMAXUPINDEXRST_H

#include "data/anaresult.h"

class LastMaxUpIndexRst: public AnaResult
{
public:
    static std::string REDIS_KEYNAME;
    LastMaxUpIndexRst(QString);

    void initDataFromDB();
};

#endif // LASTMAXUPINDEXRST_H
