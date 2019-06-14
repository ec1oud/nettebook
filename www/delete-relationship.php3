<?
	include 'authenticate.inc';

	$result = pg_Exec ($conn, "DELETE FROM relationship_assoc WHERE one_entity=$one_entity AND other_entity=$other_entity");
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
