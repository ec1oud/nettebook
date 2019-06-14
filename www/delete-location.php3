<?
	include 'authenticate.inc';
	if (!$location_id)
	{
		echo "No location ID given, cannot uniquely identify record";
		exit;
	}

	$result = pg_Exec ($conn, "DELETE FROM location WHERE location_id=$location_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
    if ($is_thing)
		Header("Location: view-thing.php3?entity_id=$entity_id");
    else
        Header("Location: view-person.php3?entity_id=$entity_id");
?>
