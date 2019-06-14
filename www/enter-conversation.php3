<? 
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "You must specify with whom you are having the conversation.";
		exit;
	}
	if ($conversation_id)
	{
	    $result = pg_Exec ($conn, "SELECT * FROM conversation WHERE conversation_id=$conversation_id");
		if (!$result)
			$conversation_id = 0;
		else
		{
    		if (pg_numrows($result) > 0)
			{
				$row = pg_fetch_array($result, 0);
				$comment = $row[comment];
			}
			else
				$conversation_id = 0;
		}
	}
	if ($is_thing)
	{
		$result = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$entity_id");
		$row = pg_fetch_array ($result, 0);
		echo "Record of conversation with <font size=+1>$row[name]</font>\n";
	}
	else
	{
		$result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$entity_id");
		$row = pg_fetch_array ($result, 0);
		echo "Record of conversation with <font size=+1>$row[othernames] $row[lastname]</font>\n";
	}
	echo "<form name=conv action=update-conversation.php3 method=post>\n";
	echo "<input type=hidden name=is_thing value=$is_thing>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	if ($conversation_id)
		echo "<input type=hidden name=conversation_id value=$conversation_id>\n";
	echo "<table>\n";
	if (!$beginning)
		$beginning = date("Y-n-j H:i:s");
#		$beginning = date("n/j/Y H:i:s");
	echo "<tr><td width=50%>beginning <input type=text size=19 name=beginning value=\"$beginning\"></td>";
	echo "<td width=50% align=right>ending <input type=text size=19 name=ending value=\"$beginning\"></td></tr>";
	echo "<td width colspan=2><textarea name=comment rows=20 cols=80>$comment</textarea></td></tr>";
	echo "<tr><td colspan=2 align=center><input type=submit value=enter></td></tr>\n";
	echo "</table>\n";
	
	echo "</form>\n";
?>
<script language="javascript">
speed=1000

function update()
{
	d = new Date();
  	document.conv.ending.value=d.getFullYear() + "-" + (d.getMonth() + 1) + "-" + d.getDate() + 
		" " + d.getHours() + ":" + d.getMinutes() + ":" + d.getSeconds();
  	window.setTimeout("update()", speed);
}
 
window.setTimeout("update()", speed);
</script>
