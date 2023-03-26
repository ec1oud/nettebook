#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
    Q_PROPERTY(bool searchCaseSensitive READ searchCaseSensitive WRITE setSearchCaseSensitive NOTIFY searchCaseSensitiveChanged)

public:
    Highlighter(QTextDocument *parent);

    QString searchText() const;
    void setSearchText(const QString &newSearchText);

    bool searchCaseSensitive() const;
    void setSearchCaseSensitive(bool newSearchCaseSensitive);

signals:
    void searchTextChanged();
    void searchCaseSensitiveChanged();

protected:
    void highlightBlock(const QString &text) override;

private:
    QString m_searchText;
    bool m_searchCaseSensitive = false;
};

#endif // HIGHLIGHTER_H
