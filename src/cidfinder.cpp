#include "cidfinder.h"

#include <QDebug>

using namespace Qt::StringLiterals;

static const auto base58HashPrefix = "Qm"_L1;
static const auto base32HashPrefix = "baf"_L1;

CidFinder::CidFinder()
{
}

CidFinder::Result CidFinder::findIn(const QString &s)
{
    Result ret;
    int i = s.indexOf(base58HashPrefix);
    if (i >= 0) {
        ret.start = i;
        ret.length = 46;
    } else {
        i = s.indexOf(base32HashPrefix);
        ret.start = i;
        ret.length = 60;
//        QByteArray b = s.toLatin1();
    }
//    qDebug() << s << ret.isValid() << ret.start << ret.length << ret.toString(s);
    return ret;
}

QString CidFinder::Result::toString(const QString &s)
{
    if (start < 0)
        return QString(); // invalid
    return s.mid(start, length);
}
