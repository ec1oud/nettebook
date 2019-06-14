<?
	include 'authenticate.inc';
	if (!$comment_id)
	{
		echo "No comment ID given, cannot uniquely identify record";
		exit;
	}

	$result = pg_Exec ($conn, "DELETE FROM entity_comment WHERE comment_id=$comment_id");
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
