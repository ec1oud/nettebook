<?
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "No entity ID given, cannot uniquely identify record";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	$owner_id = $row["entity_id"];

	$result = pg_Exec ($conn, "UPDATE person SET lastname='$lastname', othernames='$othernames', suffixes='$suffixes' WHERE entity_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	Header("Location: view-person.php3?entity_id=$entity_id");
?>
