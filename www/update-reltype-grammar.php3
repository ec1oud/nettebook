<?
	include 'authenticate.inc';
	if (!$reverse_sentence_usage || !$noun)
	{
		echo "You must fill in both blanks.";
		exit;
	}
	$result = pg_Exec ($conn, "UPDATE relationship_type set reverse_sentence_usage='$reverse_sentence_usage', name='$noun' WHERE sentence_usage='$sentence_usage'");
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
