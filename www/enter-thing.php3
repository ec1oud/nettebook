<? 
	include 'authenticate.inc';
	if (!$thing_owner) 
		$thing_owner = "null";
	if (!$entity_id) 
		echo "<form action=insert-thing.php3 method=post>\n";
	else
	{
		echo "<form action=update-thing.php3 method=post>\n";
		echo "<input type=hidden name=entity_id value=$entity_id>\n";
		$result = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$entity_id");
		$row = pg_fetch_array ($result, 0);
		if (!$result) 	
		{
		    echo "An error occured during SELECT\n";
	        exit;
		}
		if (pg_numrows($result) == 0)
		{
			echo "no thing found with entity_id $entity_id";
		    exit;
		}
		$name=$row["name"];
	}
	echo "<input type=hidden name=thing_owner value=$thing_owner>\n";

	echo "<center><table>\n";
	echo "<TR><td align=right>Name of thing</td><td><input type=text name=name size=60 value=\"$name\"></td></tr>\n";
	echo "<TR><td align=right>Type of thing</td>\n";
	echo "<td><select name=thing_type size=1>\n";
	$result = pg_Exec ($conn, "SELECT * FROM thing_type ORDER BY type_id");
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <option value=$row[type_id]>$row[name]\n";
	}
	echo "</select></td>\n";
	echo "</table>";
	echo "<input type=submit value=enter>\n";
	echo "</center>\n";
?>
