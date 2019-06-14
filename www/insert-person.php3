<?
	include 'authenticate.inc';
	if (!$lastname && !$othernames)
	{
		echo "You must enter at least one name!";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	$owner_id = $row["entity_id"];
	$lastname = ucwords($lastname);
	$othernames = ucwords($othernames);

	$result = pg_Exec ($conn, "INSERT INTO person VALUES ('$lastname', '$othernames', '$suffixes')");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
    Header("Location: view-people.php3");
?>
