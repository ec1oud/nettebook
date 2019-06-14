<center>
<? 
	include 'authenticate.inc';
	function echoTypeOption($value, $label, $selected)
	{
		echo "   <option value=$value";
		if ($value == $selected)
			echo " SELECTED";
		echo ">$label\n";
	}
	function getAllEntitiesExcept($conn, $exclude)
	{
		$retval = "";
    	$result = pg_Exec ($conn, 
			"SELECT * FROM person WHERE entity_id!=$exclude ORDER BY lastname, othernames");
	    for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	    {
	        $row = pg_fetch_array ($result, $rc);
			$retval = $retval . "   <OPTION value=$row[entity_id]>";
			$retval = $retval . "$row[lastname], $row[othernames]\n";
		}
    	$result = pg_Exec ($conn, 
			"SELECT * FROM company WHERE entity_id!=$exclude ORDER BY name");
	    for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	    {
	        $row = pg_fetch_array ($result, $rc);
			$retval = $retval . "   <OPTION value=$row[entity_id]>$row[name]\n";
		}
		return $retval;
	}
    $result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$entity_id");
    if (!$result)
    {
        echo "An error occured during SELECT\n";
        exit;
    }
	$is_company = false;
    if (pg_numrows($result) == 0)
	{
		$result = pg_Exec ($conn, "SELECT * FROM company WHERE entity_id=$entity_id");
	    if (!$result)
	    {
	        echo "An error occured during SELECT\n";
	        exit;
	    }
	    if (pg_numrows($result) == 0)
	    {
	        echo "entity $entity_id not found";
	        exit;
	    }
	    else
	    {
	        $row = pg_fetch_array ($result, 0);
	        $fullname = $row[name];
			$is_company=true;
	    }
	}
    else
    {
        $row = pg_fetch_array ($result, 0);
        $lastname = $row[lastname];
        $othernames = $row[othernames];
        $fullname = $othernames . " " . $lastname;
    }
    if (pg_numrows($result) > 1)
    	echo "Warning: more than one person with ID $entity_id was found<BR>\n";

	if (!$entity_id)
	{
		"entity_id is a required parameter";
		exit;
	}
	if ($direction == "rev")
	{
		echo "<table width=100%><tr><td>";
		echo "<B>______ has relationship with $fullname</B>\n";
		echo "</td><td align=right>";
		echo "<a href=enter-relationship.php3?entity_id=$entity_id&direction=fwd>\n";
		echo "$fullname has relationship with ______\n";
		echo "</a></td></tr></table>\n";
	}
	else
	{
		echo "<table width=100%><tr><td>";
		echo "<B>$fullname has relationship with ______\n</B>";
		echo "</td><td align=right>";
		echo "<a href=enter-relationship.php3?entity_id=$entity_id&direction=rev>\n";
		echo "______ has relationship with $fullname\n";
		echo "</a></td></tr></table>\n";
	}
	echo "<form action=insert-relationship.php3 method=post>\n";
	echo "<input type=hidden name=is_company value=$is_company>\n";
	echo "<table vspace=0>\n";
	echo "<tr valign=top>\n";
	if ($direction == "rev")
	{
		echo "<td>\n";
		echo "<input type=hidden name=entity_id value=$entity_id>\n";
		echo "<input type=hidden name=other_entity value=$entity_id>\n";
		echo "<select name=one_entity size=20>\n";
		echo getAllEntitiesExcept($conn, $entity_id);
		echo "</select>";
		echo "</td>\n";
		echo "<td>";
		echo "<select name=type size=1>\n";
		$result = pg_Exec ($conn, "SELECT * FROM relationship_type");
 		for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
			$row = pg_fetch_array ($result, $rc);
			echoTypeOption($row[type_id], $row[sentence_usage], false);
		}
		echo "</select>";
		echo "<BR>other:<BR> <input type=text name=other size=10 length=10>\n";
		echo "</td>\n";
		echo "<td><BR>$fullname</td>\n";
	}
	else
	{
		echo "<td><BR>$fullname</td>\n";
		echo "<td>";
		echo "<input type=hidden name=entity_id value=$entity_id>\n";
		echo "<input type=hidden name=one_entity value=$entity_id>\n";
		echo "<select name=type size=1>\n";
		$result = pg_Exec ($conn, "SELECT * FROM relationship_type");
 		for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
			$row = pg_fetch_array ($result, $rc);
			echoTypeOption($row[type_id], $row[sentence_usage], false);
		}
		echo "</select>";
		echo "<BR>other:<BR> <input type=text name=other size=10 length=10>\n";
		echo "</td>\n";
		echo "<td align=right>\n";
		echo "<select name=other_entity size=20>\n";
		echo getAllEntitiesExcept($conn, $entity_id);
		echo "</select>";
		echo "</td>\n";
	}

	echo "</tr>\n";
	echo "<tr><td colspan=3>Comment<BR><textarea name=comment cols=60 rows=10></textarea></td></tr>\n";
	echo "<tr><td colspan=3 align=center><input type=submit value=enter></td></tr>\n";
	echo "</table>\n";
	echo "</form>\n";
?>
</center>
