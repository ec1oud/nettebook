<?
	include 'authenticate.inc';
	if (!$lastname && !$othernames)
	{
		echo "You must enter at least one name!";
		exit;
	}
	$lastname = ucwords($lastname);
	$othernames = ucwords($othernames);

	$result = pg_Exec ($conn, "INSERT INTO person VALUES ('$lastname', '$othernames', '$suffixes')");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$query = "select entity_id from person where ";
	$added = FALSE;
	if ($lastname)
	{
		$query = $query . "lastname='$lastname'";
		$added = TRUE;
	}
	if ($othernames)
	{
		if ($added)
			$query = $query . " AND ";
		$query = $query . "othernames='$othernames'";
		$added = TRUE;
	}
	if ($suffixes)
	{
		if ($added)
			$query = $query . " AND ";
		$query = $query . "suffixes='$suffixes'";
		$added = TRUE;
	}
	$result = pg_Exec ($conn, $query);
	$row = pg_fetch_array ($result, 0);
    Header("Location: view-person.php3?entity_id=" . $row[entity_id]);
?>
