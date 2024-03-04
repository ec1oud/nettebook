---
birth: 2024-01-09T19:53:12.835-07:00
position:
  x: 50456.701053967976
  y: 49529.308555939759
---
<https://bugreports.qt.io/browse/QTBUG-76105>

What's left is QMimeData::markdown() and setMarkdown()

<https://codereview.qt-project.org/c/qt/qtbase/+/417465> Volker raises
questions about standardization on 2022-07-15:

> Following up on the previous discussion: formats that QMimeData supports
> should at least be useful within a Qt application. For widgets, that seems to
> be the case since 
> [56f0ebfe860e440dcbba8997f44836debc901119](https://codereview.qt-project.org/q/commit:56f0ebfe860e440dcbba8997f44836debc901119)
> added support for text/markdown to our rich text editing widgets, although I
> don't see an equivalent commit in qtdeclarative (grepping for 'text/markdown'
> finds nothing).
> 
> And ideally they also improve interoperability with other applications. Do
> other applications produce or consume or produce markdown?
> 
> FWIW, <https://www.rfc-editor.org/rfc/rfc7763> states that
> 
> - charset is required (also see
> 
> 
> [https://www.rfc-editor.org/rfc/rfc6838#section-4.2.1)](https://www.rfc-editor.org/rfc/rfc6838#section-4.2.1)
> ) , so we should set it and parse it
> 
> - variant is optional
> 
> But perhaps 'variant' is something we should specify when we produce the
> payload, and interpret if we receive a payload with a variant (or a charset)?

my reply on 2022-07-23:

> OK it looks like QQuickTextControl::canInsertFromMimeData and
> QQuickTextControl::insertFromMimeData need a little attention. This could even
> be useful there.
> 
> > Do other applications produce or consume or produce markdown?
> 
> Could be that some Markdown editors do? But I haven't found one yet that
> does, as far as I can tell. It seems like they ought to use the right mime
> type(s) when they put markdown onto the clipboard. Typora has a "Copy as
> Markdown" menu item, but puts it on the clipboard as plain text AFAICT. Whereas
> Qt usually offers everything we are able to export (even
> application/vnd.oasis.opendocument.text).
> 
> > perhaps 'variant' is something we should specify when we produce the
> > payload, and interpret if we receive a payload with a variant (or a charset)?
> 
> Where would we do that?
> 
> But ok there are a couple of RFCs to read.

