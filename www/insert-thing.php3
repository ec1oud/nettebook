<?
	include 'authenticate.inc';
	if (!$name)
	{
		echo "You must enter the name of the thing.";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	$owner_id = $row["entity_id"];
	$name = ucwords($name);

	$result = pg_Exec ($conn, "INSERT into thing VALUES ('$name', $thing_type, $thing_owner)");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	if ($thing_owner == 'null')
    	Header("Location: view-all-public-groups.php3");
	else
    	Header("Location: view-my-groups.php3");
?>
