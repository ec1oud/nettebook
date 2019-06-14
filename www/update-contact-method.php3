<?
	include 'authenticate.inc';
	if ($entity_id == "")
	{
		echo "entity_id is a required field.";
		exit;
	}
	if ($detail == "")
	{
		echo "You must enter contact method detail, such as phone number, email address, URL etc. as appropriate";
		exit;
	}
	if ($type == "Other")
		$type = $other;
	if (!$type)
	{
		echo "You must enter contact method type (one of Work, Home, Fax, etc.; if Other, then please fill in the 'other:' box)";
		exit;
	}
	"SELECT detail FROM contact_method WHERE entity_id=$entity_id and detail='$detail'";
	$result = pg_Exec ($conn, "SELECT detail FROM contact_method WHERE entity_id=$entity_id and detail='$detail'");
	if (!$result) 
	{
		echo "An error occured during search for existing records";
	    exit;
	}
	if (!$location_id)
		$location_id = "null";
	if (pg_numrows($result) == 0)
	{
		$result = pg_Exec ($conn, "INSERT INTO contact_method VALUES ($entity_id, $location_id, '$type', '$description', '$detail')");
	}
	else
	{
		$result = pg_Exec ($conn, "UPDATE contact_method set detail='$detail', type='$type', description='$description', location_id=$location_id WHERE entity_id=$entity_id and detail='$detail'");
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
