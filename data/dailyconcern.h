#pragma execution_character_set("utf-8")
#ifndef DAILYCONCERN_H
#define DAILYCONCERN_H

#include "anaresult.h"

class DailyConcern: public AnaResult
{
public:
    DailyConcern(QString _tableMeta);

    void initDataFromDB() override;
};

#endif // DAILYCONCERN_H
