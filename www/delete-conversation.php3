<?
	include 'authenticate.inc';
	if (!$conversation_id)
	{
		echo "No conversation ID given, cannot uniquely identify record";
		exit;
	}
	if (!$entity_id)
	{
		echo "No entity ID given, cannot uniquely identify record";
		exit;
	}

	$result = pg_Exec ($conn, "DELETE FROM conversation WHERE conversation_id=$conversation_id AND other_id=$entity_id");
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
