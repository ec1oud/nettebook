#include "codeblockhighlighter.h"
#include "settings.h"

CodeBlockHighlighter::CodeBlockHighlighter(QTextDocument *parent)
  : QSyntaxHighlighter(parent)
{
}

void CodeBlockHighlighter::highlightBlock(const QString &text)
{
    Q_UNUSED(text) // TODO reuse kate syntax highlighters?
    QTextBlockFormat fmt = currentBlock().blockFormat();
    if (fmt.hasProperty(QTextFormat::BlockCodeFence)) {
        fmt.setBackground(QColor(Settings::instance()->stringOrDefault(
                                     Settings::styleGroup, Settings::codeBlockBackground, "#EEE")));
        QTextCursor cur(currentBlock());
        cur.setBlockFormat(fmt);
    }
}
