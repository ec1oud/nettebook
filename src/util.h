#ifndef UTIL_H
#define UTIL_H

#include <QUrl>

struct Util
{
    static QString toLocalFile(const QUrl &url);
};

#endif // UTIL_H
