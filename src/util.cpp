#include "util.h"

#include <QDebug>

QString Util::toLocalFile(const QUrl &url)
{
    if (url.isLocalFile())
        return url.toLocalFile();
    if (url.scheme().isEmpty())
        return url.path();
    qWarning() << url << "is not a local file";
    return {};
}
