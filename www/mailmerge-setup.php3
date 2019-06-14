<? 
	include 'authenticate.inc';
	function echoTypeOption($value, $label, $selected)
	{
		echo "   <option value=$value";
		if ($value == $selected)
			echo " SELECTED";
		echo ">$label\n";
	}
	echo "<center><table height=100%><tr height=1%><td valign=top>";
	echo "<form action=mailmerge-confirm.php3 method=post>\n";
	echo "<input type=hidden name=thing_name value=" . urlencode($thing_name) . ">\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "Do a mail merge for every \n";
	echo "<select name=entity_type size=1>\n";
	echo "   <option value=person selected>person</option>\n";
	echo "   <option value=entity>entity</option>\n";
	echo "</select>*<BR>\n";
	echo "who \n";
	echo "<select name=type size=1>\n";
	$result = pg_Exec ($conn, "SELECT * FROM relationship_type");
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echoTypeOption($row[type_id], $row[sentence_usage], $relationship_type);
	}
	echo "</select>* \n";
	echo "$thing_name<BR>\nusing the following template:<BR>\n";
	echo "<select name=template_id size=10 width=300>\n";
	$result = pg_Exec ($conn, "SELECT * FROM mailmerge_template");
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <option value=$row[template_id]>$row[name]</option>\n";
	}
	echo "</select><BR>\n";
	echo "with\n";
	echo "<select name=address_type size=1>\n";
	echo "   <option value=smail selected>snail-mail addresses only</option>\n";
	echo "   <option value=email>email addresses only</option>\n";
	echo "   <option value=smail selected>snail-mail if possible, or fallback to email</option>\n";
	echo "   <option value=smail selected>email if possible, or fallback to snail-mail</option>\n";
	echo "</select>*<BR>\n";
	echo "When an address contains a newline,\n";
	echo "<select name=newline_handling size=1>\n";
	echo "   <option value=crlf selected>replace with &lt;CR&gt;&lt;LF&gt;</option>\n";
	echo "   <option value=lf>replace with &lt;LF&gt;</option>\n";
	echo "   <option value=slash-r-slash-n>replace with \"\\r\\n\"</option>\n";
	echo "   <option value=slash-n>replace with \"\\n\"</option>\n";
	echo "   <option value=leaveit>leave it as-is</option>\n";
	echo "   <option value=br>replace with &lt;BR&gt;</option>\n";
	echo "</select><BR>\n";
	echo "</td></tr>\n";
	echo "<tr height=1%><td valign = top align=center>\n";
	echo "<input type=submit value=\"Preview addresses\"><P>\n";
	echo "</td></tr>\n";
	echo "<tr height=50%><td valign=bottom align=center>\n";
	echo "<font size=-1>Options marked with * are ignored in this version of NetteBook.<BR>Right now you will always get people and snail-mail addresses.</font>\n";
	echo "</td></tr></table>\n";
?>

