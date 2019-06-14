<?
	include 'authenticate.inc';
	if (!$name)
	{
		echo "You must enter the group name.";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	$owner_id = $row["entity_id"];
	$name = ucwords($name);

	$result = pg_Exec ($conn, "INSERT into group_entity VALUES ('$name', $group_type, $group_owner)");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	if ($group_owner == 'null')
    	Header("Location: view-all-public-groups.php3");
	else
    	Header("Location: view-my-groups.php3");
?>
