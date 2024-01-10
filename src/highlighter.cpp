#include "highlighter.h"
#include "settings.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcHgl, "org.nettebook.highlight")

Highlighter::Highlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
}

void Highlighter::highlightBlock(const QString &text)
{
    // TODO reuse kate syntax highlighters?
    QTextBlockFormat fmt = currentBlock().blockFormat();
    if (fmt.hasProperty(QTextFormat::BlockCodeFence)) {
        fmt.setBackground(QColor(Settings::instance()->stringOrDefault(
                                     Settings::styleGroup, Settings::codeBlockBackground, "#EEE")));
        QTextCursor cur(currentBlock());
        cur.setBlockFormat(fmt);
    } else if (fmt.hasProperty(QTextFormat::BlockQuoteLevel)) {
        qCDebug(lcHgl) << "BQ" << fmt.property(QTextFormat::BlockQuoteLevel) << text;
    }

    if (!m_searchText.isEmpty()) {
        QColor bg = QColor(Settings::instance()->stringOrDefault(
                               Settings::styleGroup, Settings::searchResultBackground, "#CFF"));
        auto slen = m_searchText.length();
        qsizetype i = 0;
        do {
            i = text.indexOf(m_searchText, i,
                             m_searchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
            if (i >= 0) {
//                qDebug() << "in" << text << "found" << m_searchText << "from" << i << "len" << slen;
                auto fmt = format(i);
                fmt.setBackground(bg);
                setFormat(i, slen, fmt);
                ++i;
            }
        } while (i >= 0);
    }
}

QString Highlighter::searchText() const
{
    return m_searchText;
}

void Highlighter::setSearchText(const QString &newSearchText)
{
    if (m_searchText == newSearchText)
        return;

    m_searchText = newSearchText;
    emit searchTextChanged();
    rehighlight();
}

bool Highlighter::searchCaseSensitive() const
{
    return m_searchCaseSensitive;
}

void Highlighter::setSearchCaseSensitive(bool newSearchCaseSensitive)
{
    if (m_searchCaseSensitive == newSearchCaseSensitive)
        return;

    m_searchCaseSensitive = newSearchCaseSensitive;
    emit searchCaseSensitiveChanged();
    rehighlight();
}
