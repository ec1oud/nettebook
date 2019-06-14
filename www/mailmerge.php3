<? 
	include "authenticate.inc";
	include "mailmerge.inc";

	// Get the owner's address.
	// TODO - if the sender has more than one address, he should have been
	// forced to choose one as a return address at some point before this.
	// For now, just use the first one and hope it's right.
	$query = "select address from location where entity_id=$owner_id";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, $rc);
	$owner_address = fixNewLines($row[address], "lf");

	$owner_address_fixed = fixNewlines($owner_address, $newline_handling);

	// Generate output from template.
	$query = "select template from mailmerge_template where template_id=$template_id";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, $rc);
	$template = fixNewLines($row[template], "lf");
	$last_id = -1;
	$query = "select distinct lastname, othernames, suffixes, person.entity_id, location_id, " .
		"address from person, location, relationship_assoc where " .
		"relationship_assoc.type_id=" . $relationship_type . 
		"AND relationship_assoc.other_entity=" . $entity_id .
		"AND location.entity_id=relationship_assoc.one_entity " .
		"AND relationship_assoc.one_entity=person.entity_id ORDER BY lastname, othernames";
	$result = pg_Exec ($conn, $query);
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
			$output = $template;
			$output = str_replace("<sender/>", "$owner_othernames $owner_lastname", $output);
			$output = str_replace("<senderAddress/>", $owner_address_fixed, $output);
			$firstname = trim($row[othernames]);
			$firstspace = strpos($row[othernames], " ");
			if ($firstspace > 0)
				$firstname = substr($firstname, 0, firstspace);
			$output = str_replace("<firstName/>", $firstname, $output);
			$output = str_replace("<otherNames/>", $row[othernames], $output);
			$output = str_replace("<lastName/>", $row[lastname], $output);
			$output = handleForEach("forEachSenderAddressLine", parseLines($owner_address), $output);
			$output = str_replace("<householdNames/>", householdNames($row[entity_id], $row[location_id], 59), $output);
			$recipient_address = fixNewLines($row[address], "lf");
			$recipient_address_fixed = fixNewlines($recipient_address, $newline_handling);
			$output = str_replace("<address/>", $recipient_address_fixed, $output);
			$output = handleForEach("forEachAddressLine", parseLines($recipient_address), $output);
			echo $output;
		}
		$last_id = $row[entity_id];
	}
?>

