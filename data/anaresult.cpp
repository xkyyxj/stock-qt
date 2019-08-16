#include "anaresult.h"
#include "data/datacenter.h"

AnaResult::AnaResult(QString _tableMeta)
{
    tableMeta = _tableMeta;
}

AnaResult::~AnaResult() {}

inline std::vector<std::vector<QVariant>>& AnaResult::getData() noexcept {
    return data;
};

inline std::vector<QString>& AnaResult::getDBColumns() noexcept {
    return dbColumns;
}

inline std::vector<QString>& AnaResult::getDisplayHead() noexcept {
    return displayHead;
}

inline bool AnaResult::getIsValid() noexcept {
    return isValid;
}
