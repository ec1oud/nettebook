#ifndef CIDFINDER_H
#define CIDFINDER_H

#include <QString>

class CidFinder
{
public:
    CidFinder();

    struct Result {
        int start = -1; // index where CID is found
        int length = 0; // in characters

        bool isValid() { return start >= 0; }
        QString toString(const QString &s);
    };

    static Result findIn(const QString &s);
};

#endif // CIDFINDER_H
