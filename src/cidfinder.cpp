#include "cidfinder.h"

#include <QDebug>

static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafy");

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
        QByteArray b = s.toLatin1();
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
