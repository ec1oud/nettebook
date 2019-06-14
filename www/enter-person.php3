<? 
	include 'authenticate.inc';
	if (!$entity_id) 
		echo "<form action=insert-person.php3 method=post>\n";
	else
	{
		echo "<form action=update-person.php3 method=post>\n";
		echo "<input type=hidden name=entity_id value=$entity_id>\n";
		$result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$entity_id");
		$row = pg_fetch_array ($result, 0);
		if (!$result) 	
		{
		    echo "An error occured during SELECT\n";
	        exit;
		}
		if (pg_numrows($result) == 0)
		{
			echo "noone found with entity_id $entity_id";
		    exit;
		}
		$lastname=$row["lastname"];
		$othernames=$row["othernames"];
		$suffixes=$row["suffixes"];
	}

	echo "<table>\n";
	echo "<tr>\n";
	echo "<td>Last name</td><td>First, middle names, etc.</td><td>Title or other suffix</td>\n";
	echo "</tr>\n";
	echo "<tr>\n";
	echo "<td>\n";
	echo "<input type=text name=lastname size=30 value=$lastname >\n";
	echo "</td>\n";
	echo "<td>\n";
	echo "<input type=text name=othernames size=40 value=$othernames >\n";
	echo "</td>\n";
	echo "<td>\n";
	echo "<input type=text name=suffixes size=8 value=$suffixes >\n";
	echo "</td>\n";
	echo "</tr>\n";
	echo "</table>\n";
	echo "<center>\n";
	echo "<input type=submit value=enter>\n";
	echo "</center>\n";
	echo "</form>\n";
?>
