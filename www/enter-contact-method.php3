<center>
<? 
	include 'authenticate.inc';
	function echoTypeOption($value, $selected)
	{
		echo "   <option value=$value";
		if ($value == $selected)
			echo " SELECTED";
		echo ">$value\n";
	}
	if (!$entity_id)
	{
		"entity_id is a required parameter";
		exit;
	}
    $result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$entity_id");
    if (!$result)
    {
        echo "An error occured during SELECT\n";
        exit;
    }
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
        echo "Warning: more than one entity with ID $entity_id was found<BR>\n";
	echo "<B>New contact method for $fullname</b>\n";

	echo "<form action=update-contact-method.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=is_company value=$is_company>\n";
	if ($detail)
	{
		$detail = rawurldecode($detail);
		$result = pg_Exec ($conn, 
			"SELECT * FROM contact_method WHERE entity_id=$entity_id AND detail='$detail'");
		if ($result && pg_numrows($result) > 0)
		{
			$row = pg_fetch_array ($result, 0);
			$description=$row["description"];
			$location_id=$row["location_id"];
			$type=$row["type"];
		}
	}

	echo "<table vspace=0>\n";
	echo "<tr>\n";
	echo "<td>Number, address, URL etc.<BR>\n";
	echo "<input type=text name=detail size=40 value=\"$detail\" >\n";
	echo "</td></tr>\n";
	echo "<tr><td><table><tr><td>Type<BR>";
	echo "<select name=type size=1>\n";
	echoTypeOption("Other", $type);
	echoTypeOption("Work", $type);
	echoTypeOption("Home", $type);
	echoTypeOption("Fax", $type);
	echoTypeOption("E-mail", $type);
	echoTypeOption("Main", $type);
	echoTypeOption("Pager", $type);
	echoTypeOption("Mobile", $type);
	echoTypeOption("ICQ", $type);
	echoTypeOption("Homepage", $type);
	echoTypeOption("Callsign", $type);
	$result = pg_Exec ($conn, "SELECT DISTINCT type FROM contact_method");
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		if ($row[type] == "Other")
			continue;
		if ($row[type] == "Work")
			continue;
		if ($row[type] == "Home")
			continue;
		if ($row[type] == "Fax")
			continue;
		if ($row[type] == "E-mail")
			continue;
		if ($row[type] == "Main")
			continue;
		if ($row[type] == "Pager")
			continue;
		if ($row[type] == "Mobile")
			continue;
		if ($row[type] == "ICQ")
			continue;
		if ($row[type] == "Homepage")
			continue;
		if ($row[type] == "Callsign")
			continue;
		echoTypeOption($row[type], $type);
	}
	echo "</select>";
	echo "</td>\n";
	echo "<td>other:<BR><input type=text name=other size=10 length=10></td>\n";
	echo "</tr></table></td></tr><tr><td>Location<BR>\n";
	if (!$location_id)
		$location_id="UNKNOWN";
	echo "<select name=location_id size=1>\n";
	echo "<option value=NULL>UNKNOWN\n";
	if ($entity_id)
	{
		$result = pg_Exec ($conn, "SELECT * FROM location WHERE entity_id=$entity_id");
 		for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
			$row = pg_fetch_array ($result, $rc);
			echo "<option value=$row[location_id]";
			if ($row[location_id] == $location_id)
				echo " SELECTED";
			echo ">$row[name]\n";
		}
	}
	echo "</select>";
	echo "</td>";
	echo "</tr>\n";
	echo "<tr>\n";
	echo "<td>\n";
	echo "Description<BR><input type=text name=description size=40 value=\"$description\" >\n";
	echo "</td>\n";
	echo "</tr>\n";
	echo "</table>\n";
	echo "<input type=submit value=enter>\n";
	
	echo "</form>\n";
?>
</center>
