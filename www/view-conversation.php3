<?
	include 'authenticate.inc';
	if (!$conversation_id)
	{
		echo "No conversation ID given, cannot uniquely identify";
		exit;
	}
	$result = pg_Exec ($conn, "SELECT * FROM conversation WHERE owner_id=$owner_id AND conversation_id=$conversation_id");
	if (!$result) 	
	{
		echo "An error occured during SELECT\n";
	    exit;
	}
	if (pg_numrows($result) == 0)
	{
		echo "none found";
		exit;
	}
	if (pg_numrows($result) > 1)
		echo "Warning: more than one conversation with ID $conversation_id was found<BR>\n";
	$row = pg_fetch_array ($result, 0);
	$other_id = $row[other_id];
	if ($is_thing)
	{
		$result = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$other_id");
		$row2 = pg_fetch_array ($result, 0);
		echo "<B>Conversation between $owner_othernames $owner_lastname and $row2[name]<BR></B>\n";
	}
	else
	{
		$result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$other_id");
		$row2 = pg_fetch_array ($result, 0);
		echo "<B>Conversation between $owner_othernames $owner_lastname and $row2[othernames] $row2[lastname]<BR></B>\n";
	}
	echo "<table width = 100%><tr>";
	echo "<td>From $row[beginning] to $row[ending]</td>\n";
	echo "<td align=right><a href = delete-conversation.php3?conversation_id=$conversation_id&entity_id=$other_id&is_thing=$is_thing>delete</a></td>\n";
	echo "</tr></table><hr>\n";
	echo $row[comment];
?>
