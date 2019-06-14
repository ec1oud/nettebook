<? 
	include 'authenticate.inc';
	include 'find-name.inc';

	echo "<center>";
	echo "<form action=mailmerge-single-confirm.php3 method=post>\n";
	echo "<input type=hidden name=thing_name value=" . urlencode($thing_name) . ">\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "Mail-merge ";
	echo find_name($entity_id);
	echo " using the following template:<BR>\n";
	echo "<select name=template_id size=10 width=300>\n";
	$result = pg_Exec ($conn, "SELECT * FROM mailmerge_template");
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <option value=$row[template_id]>$row[name]</option>\n";
	}
	echo "</select><BR>\n";
	echo "When an address contains a newline,\n";
	echo "<select name=newline_handling size=1>\n";
	echo "   <option value=crlf selected>replace with &lt;CR&gt;&lt;LF&gt;</option>\n";
	echo "   <option value=lf>replace with &lt;LF&gt;</option>\n";
	echo "   <option value=slash-r-slash-n>replace with \"\\r\\n\"</option>\n";
	echo "   <option value=slash-n>replace with \"\\n\"</option>\n";
	echo "   <option value=leaveit>leave it as-is</option>\n";
	echo "   <option value=br>replace with &lt;BR&gt;</option>\n";
	echo "</select><BR>\n";
	echo "<input type=submit value=\"Go\"><P>\n";
?>

