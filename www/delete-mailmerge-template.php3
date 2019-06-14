<?
	include 'authenticate.inc';
	if (!$template_id)
	{
		echo "No template ID given, cannot uniquely identify record";
		exit;
	}

	$result = pg_Exec ($conn, "DELETE FROM mailmerge_template WHERE template_id=$template_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	Header("Location: view-my-groups.php3?entity_id=$owner_id");
?>
