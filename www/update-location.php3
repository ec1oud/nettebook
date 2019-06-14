<?
	include 'authenticate.inc';
	if ($entity_id == "")
	{
		echo "entity_id is a required field.";
		exit;
	}
	if ($location_id)
	{
		$result = pg_Exec ($conn, "UPDATE location set address='$address', description='$description', name='$name' WHERE entity_id=$entity_id and location_id=$location_id");
	}
	else
	{
		$result = pg_Exec ($conn, "INSERT INTO location VALUES ('$name', '$description', '$address', $entity_id)");
	}
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	if ($is_company)
    	Header("Location: view-company.php3?entity_id=$entity_id");
	else
    	Header("Location: view-person.php3?entity_id=$entity_id");
?>
