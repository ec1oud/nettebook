---
birth: 2024-01-09T20:04:55.675-07:00
position:
  x: 49408.464957458869
  y: 49622.842785730696
---
<https://bugreports.qt.io/browse/QTCREATORBUG-29756> asks for the ability to
sync scrolling in a 
[two-pane editor](two-pane-markdown-editor.md#Markdown_syntax_and_a_preview_side-by-side)
. It seems to me that we need to store source character ranges in
QTextDocumentFragment, maybe just add an enum value to QTextFormat::Property so
you can get it from QTextCharFormat::property() or something like that.

