<?
	include 'authenticate.inc';
	if (!$creator_id)
	{
		echo "creator_id is a required field.";
		exit;
	}
	if (!$template_name)
	{
		echo "template_name is a required field.";
		exit;
	}
	if ($template_id)
	{
		$result = pg_Exec ($conn, "UPDATE mailmerge_template set template='$template', name='$template_name' WHERE template_id=$template_id");
	}
	else
	{
		$result = pg_Exec ($conn, "INSERT INTO mailmerge_template VALUES ('$template_name', '$template', $creator_id)");
	}
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
   	Header("Location: view-my-groups.php3?entity_id=$owner_id");
?>
