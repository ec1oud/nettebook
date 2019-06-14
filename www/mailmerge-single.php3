<? 
	include "authenticate.inc";
	include "mailmerge.inc";

	// Get the owner's address.
	// TODO - if the sender has more than one address, he should have been
	// forced to choose one as a return address at some point before this.
	// For now, just use the first one and hope it's right.
	$query = "select address from location where entity_id=$owner_id";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, 0);
	$owner_address = fixNewLines($row[address], "lf");

	$owner_address_fixed = fixNewlines($owner_address, $newline_handling);

	// Generate output from template.
	$query = "select template from mailmerge_template where template_id=$template_id";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, 0);
	$template = fixNewLines($row[template], "lf");
	$last_id = -1;
	$query = "select lastname, othernames, suffixes, person.entity_id, location_id, " .
		"address from person, location where location.location_id=$selected_address AND " .
		"person.entity_id=$entity_id";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, 0);
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

	// Decide what to do with the output
	$query = "select * from output_device where id=$selected_output";
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, 0);
	if ($row[type] == 1)
	{
		$fp = popen($row[commandline], "w");
		fputs($fp, $output);
		echo "<PRE>";
		system("/usr/bin/lpq");
		echo "</PRE>\n";
	}
	else
		echo $output;
?>

