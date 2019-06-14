<?
	include 'authenticate.inc';
	function displayContactMethod($cm)
	{
		if (strpos($cm, "@"))
		    return "<a href=mailto:$cm>$cm</a>";
		else if (strpos(" " . $cm, "http:"))
		    return "<a href=$cm>$cm</a>";
		else
			return $cm;
	}
	$result = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$entity_id");
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
		echo "Warning: more than one thing with ID $entity_id was found<BR>\n";
	$row = pg_fetch_array ($result, 0);
	echo "<B>All people with relationships to </B><font size=+1>$row[name]</font>";
	$thing_type = $row[type];
	$thing_name = $row[name];
	$result = pg_Exec ($conn, "SELECT * from thing_type WHERE type_id=$thing_type");
	$row = pg_fetch_array ($result, 0);
	$default_relationship = $row[default_relationship];
	$result = pg_Exec ($conn, "SELECT * from relationship_type WHERE type_id=$default_relationship");
	$row = pg_fetch_array ($result, 0);
	$default_relationship_name = strtoupper(substr($row[name], 0, 1)) . strtolower(substr($row[name], 1));

	echo "<table width=100%>\n";
	echo "<tr bgcolor=lightyellow><th width=50% align=left colspan=2>Person</th>\n";
	echo "<th width=50% align=left>Contact methods</th></tr>\n";
	$result = pg_Exec ($conn, "SELECT lastname, othernames, person.entity_id from person, relationship_assoc" . 
		" WHERE relationship_assoc.one_entity=person.entity_id AND relationship_assoc.other_entity=$entity_id" .
		" ORDER BY lastname, othernames");
    for ($rc = 0; $rc < pg_numrows($result); ++$rc)
    {
        $row = pg_fetch_array ($result, $rc);
		$res2 = pg_Exec ($conn, "SELECT name, filename, thumbnail from image WHERE entity_id=$row[entity_id] ORDER BY type, filename LIMIT 1");
		echo "   <tr><td width=1%>";
		if (pg_numrows($res2) > 0)
		{
  	    	$r2 = pg_fetch_array ($res2, 0);
			echo "<a href=images/entities/$r2[filename]><img src=images/entities/$r2[thumbnail] alt=\"$r2[name]\"></a>";
		}
		echo "</td><td><a href=view-person.php3?entity_id=$row[entity_id]>$row[lastname], $row[othernames]";
		echo "</a></td><td>";
		$res2 = pg_Exec ($conn, "SELECT * from contact_method WHERE entity_id=$row[entity_id] ORDER BY type");
    	for ($i = 0; $i < pg_numrows($res2); ++$i)
	    {
	  	    $r2 = pg_fetch_array ($res2, $i);
			echo "$r2[type]: " . displayContactMethod($r2[detail]) . "<BR>\n";
		}
		echo "</td></tr>\n";
	}

	echo "</table>\n";
?>
