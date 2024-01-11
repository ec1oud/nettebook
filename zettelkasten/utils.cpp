#include "utils.h"

#include <QLoggingCategory>
#include <QPalette>
#include <QRegularExpression>
#include <QTextCursor>

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

bool Utils::insertLink(QQuickTextDocument *doc1, const QUrl &u1,
                       QQuickTextDocument *doc2, const QUrl &u2)
{
    qCDebug(lcLnk) << "cross-link" << doc1->source() << u1 << doc2->source() << u2;

    // QTextDocument *qtd1 = ;
    // QTextDocument *qtd2 = doc2->textDocument();

    QTextCursor cursor1(doc1->textDocument());
    // cursor1.movePosition(...) // TODO we need start and end positions
    QTextCursor cursor2(doc2->textDocument());

    cursor1.beginEditBlock();
    QTextCharFormat fmt = cursor1.charFormat();
    fmt.setForeground(QPalette().link());
    fmt.setAnchor(true);
    fmt.setAnchorHref(u1.toString());
    cursor1.setCharFormat(fmt);
    cursor1.endEditBlock();
    return true;
}
