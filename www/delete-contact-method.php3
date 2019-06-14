<?
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "No entity ID given, cannot uniquely identify record";
		exit;
	}
	if (!$detail)
	{
		echo "No detail given, cannot uniquely identify record";
		exit;
	}
	$detail = rawurldecode($detail);
	$result = pg_Exec ($conn, "DELETE FROM contact_method WHERE entity_id=$entity_id AND detail='$detail'");
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
