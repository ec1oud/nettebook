#ifndef CODEBLOCKHIGHLIGHTER_H
#define CODEBLOCKHIGHLIGHTER_H

#include <QSyntaxHighlighter>

class CodeBlockHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    CodeBlockHighlighter(QTextDocument *parent);

protected:
    void highlightBlock(const QString &text) override;
};

#endif // CODEBLOCKHIGHLIGHTER_H
