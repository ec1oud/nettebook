<?
function addToList($listSoFar, $next, $lookahead)
{
	$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
	if (!$lookahead)
	{
		if (!$next)
			return listSoFar;
		return $listSoFar . " and " . $next;
	}
	return $listSoFar . ", " . $next;
}

function findOtherNames($person_id)
{
	$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
	$res = pg_Exec ($conn, "SELECT othernames FROM person WHERE entity_id=$person_id");
	if (!$res)
		return "UNKNOWN";
	if (pg_numrows($res) < 1)
		return "UNKNOWN";
	$r = pg_fetch_array ($res, 0);
	return $r[othernames];
}

function hasLocation($person_id)
{
	$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
	$res = pg_Exec ($conn, "SELECT location_id FROM location WHERE entity_id=$person_id");
	if (!$res)
		return FALSE;
	if (pg_numrows($res) < 1)
		return FALSE;
	return TRUE;
}

function hasSameLocation($person_id, $location)
{
	$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
	$res = pg_Exec ($conn, "SELECT location_id FROM location WHERE entity_id=$person_id AND location_id=$location");
	if (!$res)
		return FALSE;
	if (pg_numrows($res) < 1)
		return FALSE;
	return TRUE;
}

function presumedToCohabitate($one, $ones_location, $other)
{
	if (hasSameLocation($other, $ones_location))
		return TRUE;
	if (!(hasLocation($other)))
		return TRUE;
	return FALSE;
}

function addIfNotDuplicate($array, $newElement)
{
	$count = count($array);
	$found = FALSE;
	for ($i = 0; $i < $count && !$found; ++$i)
		if ($array[$i] == $newElement)
			$found = TRUE;
	if (!$found)
		$array[] = $newElement;
	return $array;
}

// Make up a name sequence which includes all the members of
// the household of the given adult who live at that adult's home.
// Examples - given Jim Weller's entity_id in the $adult
// parameter, it would return "Jim and Dorothy Weller"; or
// given Dorothy Weller, it would return "Dorothy and Jim Weller"
// (because we do not enforce the convention of putting
// the man first here - it's up to the caller of the function).
// Polygamy is allowed - we will list all married persons who
// live at the same location.  For another household with
// children it might look like
// "Bob, Kim, Bobby, Jack and Jill Jones" or if a sequence
// of that form exceeds max_length charcters then simply 
// "The Galbraiths".
// The parents of $adult and spouse(s) are included, as are
// the children of all the above, only if these people do
// not have other locations listed in the database.
// Roommate situations are not handled yet (that should be another
// relationship type similar to spouse in that the roommate's
// SO or kids ought to be included if they cohabitate).
// Also, it will assume that the given $adult will define
// everyone's last name, which is not always true.  Should check 
// and keep a separate list of cohabitants with different last 
// names, and list them after the people with the same household name.
function householdNames($adult, $location, $max_length)
{
	$retval = "UNKNOWN";
	$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
	$res = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$adult");
	if (!$res)
		return $retval;
	if (pg_numrows($res) < 1)
		return $retval;
	$r = pg_fetch_array ($res, 0);
	$lastname = $r[lastname];
	$head_of_household = $r[othernames];
	$retval = $head_of_household;
	$next_member = "";

	// The given adult lives with himself (we hope).
	$cohabitant_ids[] = $adult;

	// Now get the spouse(s) (spice?) and other family members.
	// For any family member who has the same location, add him or her.
	// For any family member who has no location, if it's a child,
	// parent or spouse, assume the person lives with $adult.
	$query = "SELECT * FROM relationship_assoc WHERE one_entity=$adult OR other_entity=$adult";
	$res = pg_Exec ($conn, $query);
	// Run through once looking for spouses; list them first.
    for ($rc = 0; $rc < pg_numrows($res); ++$rc)
    {
		$r = pg_fetch_array ($res, $rc);
		if ($r[type_id] == 6)
		{
			$lookaheadID = $r[other_entity];
			if ($lookaheadID == $adult)
				$lookaheadID = $r[one_entity];
			if ( presumedToCohabitate($adult, $location, $lookaheadID) )
				$cohabitant_ids = addIfNotDuplicate($cohabitant_ids, $lookaheadID);
		}
	}

	// Now add the parents of the $adult and of the spouse(s), 
	// if they live with them.
	$count = count($cohabitant_ids);
	for ($i = 0; $i < $count; ++$i)
	{
		$query = "SELECT * FROM relationship_assoc WHERE one_entity=$cohabitant_ids[$i] AND (type_id=4 OR type_id=5)";
		$res = pg_Exec ($conn, $query);
    	for ($rc = 0; $rc < pg_numrows($res); ++$rc)
	    {
			$r = pg_fetch_array ($res, $rc);
			if ( presumedToCohabitate($adult, $location, $r[other_entity]) )
				$cohabitant_ids = addIfNotDuplicate($cohabitant_ids, $r[other_entity]);
		}
	}

	// Now look for kids of all of the above.
	$count = count($cohabitant_ids);
	for ($i = 0; $i < $count; ++$i)
	{
		$query = "SELECT * FROM relationship_assoc WHERE other_entity=$cohabitant_ids[$i] AND (type_id=4 OR type_id=5)";
		$res = pg_Exec ($conn, $query);
    	for ($rc = 0; $rc < pg_numrows($res); ++$rc)
	    {
			$r = pg_fetch_array ($res, $rc);

			// If the child doesn't have another location then
			// assume that he/she lives with $adult.
			if ( presumedToCohabitate($adult, $location, $r[other_entity]) )
				$cohabitant_ids = addIfNotDuplicate($cohabitant_ids, $r[other_entity]);
		}
	}

	// Finally, simply list all the cohabitants in order.
	$count = count($cohabitant_ids) - 1;
	for ($i = 1; $i < $count; ++$i)
		$retval = addToList($retval, findOtherNames($cohabitant_ids[$i]), $cohabitant_ids[$i + 1]);
	if ($count > 0)
		$retval = addToList($retval, findOtherNames($cohabitant_ids[$i]), "");

	// Add the last name
	$retval = $retval . " " . $lastname;

	// Return retval, or if too long, just return "The Lastnames".
	if (strlen($retval) > $max_length)
		return "The " . $lastname . "s";
	return $retval;
}
?>
