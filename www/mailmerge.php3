<? 
	include "authenticate.inc";
	include "household.inc";

	function fixNewlines($input, $method)
	{
		if ($method == "leaveit")
			return $input;
		// Make usage of CR/LF consistent - replace with LFs
		$input = str_replace("\r\n", "\n", $input);
		$input = str_replace("\n\r", "\n", $input);
		$input = str_replace("\r", "\n", $input);

		// Now replace LFs with whatever was requested
		if ($method == "crlf")
			return str_replace("\n", "\r\n", $input);
		if ($method == "slash-n")
			return str_replace("\n", "\\n", $input);
		if ($method == "slash-r-slash-n")
			return str_replace("\n", "\\r\\n", $input);
		if ($method == "br")
			return str_replace("\n", "<BR>", $input);
		return $input;
	}

	// Separate the lines of the input and return an array.
	// Assumes there will be only newlines, no CRs.
	function parseLines($input)
	{
		$lastFound = strpos($input, "\n");
		while ($lastFound)
		{
			$retval[] = substr($input, 0, $lastFound);
			$input = substr($input, $lastFound + 1);
			$lastFound = strpos($input, "\n");
		}
		if ($input)
			$retval[] = $input;
		return $retval;
	}

	// Given some lines of text, apply each of them in turn to the given 
	// function in the input.  Recursive.
	function handleForEach($functionName, $lines, $input)
	{
		$lastFound = strpos($input, $functionName);
		if (!$lastFound)
			return $input;
		// Find the end of the function.
		$fnEnd = strpos($input, ")", $lastFound);
		if (!$fnEnd)
			return $input;
		// Find the beginning of the function parameter.
		$fnParmBeg = strpos($input, "\"", $lastFound);
		// Extract the text to output for each line.  Remove it from input.
		$output = substr($input, $fnParmBeg + 1, $fnEnd - $fnParmBeg - 2);
		$trailer = substr($input, $fnEnd + 1);
		$retval = substr($input, 0, $lastFound);
		// For each value in lines, output the given text replacing the 
		// <line/> variable.
        $count = count($lines);
        for ($i = 0; $i < $count; ++$i)
               $retval = $retval . str_replace("<line/>", $lines[$i], $output);
		$retval = $retval . $trailer;
		if (strpos($input, $functionName))
			$retval = handleForEach($functionName, $lines, $retval);
		return $retval;
	}

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

