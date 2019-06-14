<form action="update-card.php3" method="post">
category: <input type=text name=category size=8 value=<? echo $category?> >
title: <input type=text name=title size=59 value=<? echo $title?> ><BR>
<textarea name=body rows=15 cols=80><? echo $body?></textarea><BR>
<center>
<input type=submit value=enter>
</center>
</form>
