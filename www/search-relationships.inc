<?
	$sqlstring = "SELECT * FROM relationship_assoc, relationship_type WHERE relationship_assoc.type_id=relationship_type.type_id AND (one_entity=$entity_id OR other_entity=$entity_id)";
	if ($relationship_type != "ALL")
		$sqlstring = $sqlstring . " AND relationship_assoc.type_id=$relationship_type";
	$result = pg_Exec ($conn, $sqlstring);
	if (!$result) 	
	{
		echo "An error occured during SELECT\n";
	    exit;
	}
	function find_name($ent_id)
	{
		$retval = "UNKNOWN";
		$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
		$res = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$retval = "<a href = view-person.php3?entity_id=$ent_id>";
			$r = pg_fetch_array ($res, 0);
			$retval = $retval . $r[othernames] . " " . $r[lastname];
			if ($suffixes)
				$retval = $retval . ", " . $r[suffixes];
			$retval = $retval . "</a>";
			return $retval;
		}
		$res = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$r = pg_fetch_array ($res, 0);
			if ($r[owner])
			{
				$res = pg_Exec ($conn, "SELECT * from person WHERE entity_id=$r[owner]");
				$r2 = pg_fetch_array ($res, 0);
				return "<a href = view-thing.php3?entity_id=$ent_id>$r2[othernames] $r2[lastname]'s $r[name]</a>";
			}
			return "<a href = view-thing.php3?entity_id=$ent_id>$r[name]</a>";
		}
	}
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		if ($terse)
			echo "<tr>\n   <td>" . find_name($row[one_entity]) . "</td>\n";
		else
			echo "<tr>\n   <td>" . find_name($row[one_entity]) . " $row[sentence_usage] " , find_name($row[other_entity]) . "</td>\n";
		echo "   <td>$row[comment]</td>\n\n";
		echo "   <td><a href=delete-relationship.php3?one_entity=$row[one_entity]&other_entity=$row[other_entity]&entity_id=$entity_id&is_thing=$is_thing>delete</a></td>\n</tr>\n";
	}
?>
