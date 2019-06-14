<center>
<? 
	include 'authenticate.inc';
	echo "<form action=update-entity-comment.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=is_thing value=$is_thing>\n";
	if ($comment_id)
	{
		echo "<input type=hidden name=comment_id value=$comment_id>\n";
		$result = pg_Exec ($conn, "SELECT * FROM entity_comment WHERE comment_id='$comment_id'");
		if ($result && pg_numrows($result) > 0)
		{
			$row = pg_fetch_array ($result, 0);
			$comment=$row["comment"];
		}
	}
	echo "Comment<BR>\n";
	echo "<TEXTAREA COLS=60 NAME=comment ROWS=10 WRAP=SOFT>";
	echo "$comment";
	echo "</TEXTAREA><BR>\n";
	echo "<input type=submit value=enter>\n";
	echo "</form>\n";
?>
</center>
