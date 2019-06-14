<?
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "entity_id is a required field.";
		exit;
	}
	if ($comment_id)
	{
		$result = pg_Exec ($conn, "UPDATE entity_comment set comment='$comment' WHERE comment_id=$comment_id");
	}
	else
	{
		$result = pg_Exec ($conn, "INSERT INTO entity_comment VALUES ($entity_id, '$comment')");
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
