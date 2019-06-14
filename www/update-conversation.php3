<?
	include 'authenticate.inc';
	if ($entity_id == "")
	{
		echo "entity_id is a required field.";
		exit;
	}
	if ($conversation_id)
	{
		$result = pg_Exec ($conn, "SELECT conversation_id FROM conversation WHERE conversation_id=$conversation_id");
		if (!$result) 
		{
			echo "An error occured during search for existing records";
	    	exit;
		}
		if (pg_numrows($result) == 1)
		{
			$result = pg_Exec ($conn, "UPDATE conversation set owner_id=$PHP_AUTH_USER, other_id=$entity_id, comment='$comment', beginning='$beginning', ending='$ending' WHERE conversation_id=$conversation_id");
		}
		if (!$result) 	
		{
			echo "An error occured.\n";
	    	exit;
		}
    	Header("Location: view-person.php3?entity_id=$entity_id");
	}
	$beginning = #beginning . " " . getenv ("TZ");
	$ending = $ending . " " . getenv ("TZ");
	$result = pg_Exec ($conn, "INSERT INTO conversation VALUES ($owner_id, $entity_id, '$comment', '$beginning', '$ending')");
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
