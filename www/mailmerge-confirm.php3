<? 
	include "authenticate.inc";
	include "household.inc";
	if (!$template_id)
	{
		echo "You must chooose a template.";
		exit;
	}
	// If there was a submission from a previous instance of this form,
	// insert into list_addresses.
	if ($selected_address)
	{
		$query = "insert into list_addresses values(" . $member_id . ", " . 
			$entity_id . ", " . $selected_address . ")";
		pg_Exec($conn, $query);
	}
	$query = "select distinct lastname, othernames, suffixes, person.entity_id, location_id, " .
		"address from person, location, relationship_assoc where " .
		"relationship_assoc.type_id=" . $type . 
		"AND relationship_assoc.other_entity=" . $entity_id .
		"AND location.entity_id=relationship_assoc.one_entity " .
		"AND relationship_assoc.one_entity=person.entity_id ORDER BY person.entity_id";
	$result = pg_Exec ($conn, $query);
	$last_id = -1;
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		// verify that there is only one address, or if not, that
		// there is an entry in list_addresses to disambiguate;
		// if we are not sure which address to use, then ask.
		$row = pg_fetch_array ($result, $rc);
		if ($last_id == $row[entity_id])
		{
			$query = "select location_id from list_addresses where ".
				"member_id=" . $row[entity_id] .
				"AND thing_id=" . $entity_id;
			$result2 = pg_Exec($conn, $query);
			if (pg_numrows($result2) < 1)
			{
				echo "<center><form action=mailmerge-confirm.php3 method=post>\n";
				echo "<input type=hidden name=entity_id value=$entity_id>\n";
				echo "<input type=hidden name=entity_type value=$entity_type>\n";
				echo "<input type=hidden name=type value=$type>\n";
				echo "<input type=hidden name=template_id value=$template_id>\n";
				echo "<input type=hidden name=address_type value=$address_type>\n";
				echo "<input type=hidden name=newline_handling value=$newline_handling>\n";
				echo "<input type=hidden name=member_id value=$row[entity_id]>\n";
				echo "Please select which address to use for sending <B>";
				echo urldecode($thing_name) . "</b> mail to <font size=+1>";
				echo "$row[othernames] $row[lastname]</font>:<BR>\n";
				echo "<table>\n";
				$query = "select distinct lastname, othernames, suffixes, person.entity_id, " .
					"location_id, address from person, location, relationship_assoc where " .
					"relationship_assoc.type_id=" . $type . 
					"AND relationship_assoc.other_entity=" . $entity_id .
					"AND location.entity_id=relationship_assoc.one_entity " .
					"AND relationship_assoc.one_entity=person.entity_id " .
					"AND person.entity_id=" . $row[entity_id];
				$result3 = pg_Exec($conn, $query);
			    for ($rc3 = 0; $rc3 < pg_numrows($result3); ++$rc3)
			    {
			        $row3 = pg_fetch_array ($result3, $rc3);
					echo "  <tr valign=top><td><input type=radio name=selected_address value=$row3[location_id]></td>";
					echo "<td><pre>$row3[address]</pre></td></tr>\n";
			    }
				echo "</table>";
				echo "<input type=submit value=Enter>";
				echo "</form></center>";
			}
		}
		$last_id = $row[entity_id];
	}
	// If we get this far, there are no errors.  
	// Show entire list with link to final output.
	$last_id = -1;
	$query = "select distinct lastname, othernames, suffixes, person.entity_id, location_id, " .
		"address from person, location, relationship_assoc where " .
		"relationship_assoc.type_id=" . $type . 
		"AND relationship_assoc.other_entity=" . $entity_id .
		"AND location.entity_id=relationship_assoc.one_entity " .
		"AND relationship_assoc.one_entity=person.entity_id ORDER BY lastname, othernames";
	$result = pg_Exec ($conn, $query);
	echo "<form action=mailmerge.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=entity_type value=$entity_type>\n";
	echo "<input type=hidden name=relationship_type value=$type>\n";
	echo "<input type=hidden name=template_id value=$template_id>\n";
	echo "<input type=hidden name=address_type value=$address_type>\n";
	echo "<input type=hidden name=newline_handling value=$newline_handling>\n";
	echo "<center>If the list looks OK,<BR><input type=submit value=\"Do the Merge\"></center>\n";
	echo "<hr><table width=100%>\n";
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		$show = TRUE;
		if ($last_id == $row[entity_id])
		{
			$query = "select location_id from list_addresses where ".
				"member_id=" . $row[entity_id] .
				"AND thing_id=" . $entity_id;
			$result2 = pg_Exec($conn, $query);
			$row2 = pg_fetch_array($result2, 0);
			$show = ($row2[location_id] == $row[location_id]);
		}
		if ($show)
		{
			echo "<TR valign=top><TD>" . householdNames($row[entity_id], $row[location_id], 59) . "</TD>";
			echo "<TD><pre>$row[address]</pre></TD></TR>\n";
		}
		$last_id = $row[entity_id];
	}
	echo "</table>\n";
?>

