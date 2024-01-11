#include "utils.h"

#include <QLoggingCategory>
#include <QRegularExpression>

Q_LOGGING_CATEGORY(lcLnk, "org.nettebook.link")

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

QUrl Utils::rangedAnchorUrl(const QUrl &source, const QString &text)
{
    static QRegularExpression ws("\\s+");
    QString underscored(text);
    underscored.replace(ws, "_");
    QUrl ret(source);
    ret.setFragment(underscored);
    qCDebug(lcLnk) << source << text << "->" << underscored << "->" << ret;
    return ret;
}
