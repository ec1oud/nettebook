https://bugreports.qt.io/browse/QTCREATORBUG-29756 asks for the ability to sync
scrolling in a [two-pane editor](two-pane-markdown-editor.md). It seems to me
that we need to store source character ranges in QTextDocumentFragment, maybe
just add an enum value to QTextFormat::Property so you can get it from
QTextCharFormat::property() or something like that.

