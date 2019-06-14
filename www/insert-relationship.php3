<?
	include 'authenticate.inc';
	if (!$one_entity)
	{
		echo "Error: subject entity unknown";
		exit;
	}
	if (!$other_entity)
	{
		echo "<P>Error: object entity unknown</P>\n";
		echo "<P>You must choose an \"other\" entity</P>";
		exit;
	}
	if ($other)
	{
		$result = pg_Exec ($conn, "INSERT INTO relationship_type VALUES (null, '$other')");
		$result = pg_Exec ($conn, "SELECT * FROM relationship_type WHERE sentence_usage='$other'");
        if (pg_numrows($result) > 0)
        {
            $row = pg_fetch_array ($result, 0);
			$type = $row[type_id];
		}
		else
		{
			echo "Failed to insert new relationship type $other";
			exit;
		}
	}

	if ($comment)
		$result = pg_Exec ($conn, "INSERT INTO relationship_assoc VALUES ($one_entity, $other_entity, $type, '$comment')");
	else
		$result = pg_Exec ($conn, "INSERT INTO relationship_assoc VALUES ($one_entity, $other_entity, $type)");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	if ($other)
    	Header("Location: enter-reltype-grammar.php3?sentence_usage=" . urlencode($other) . "&one_entity=$one_entity&other_entity=$other_entity&entity_id=$entity_id&is_company=$is_company");
	else
	    if ($is_company)
	        Header("Location: view-company.php3?entity_id=$entity_id");
	    else
	        Header("Location: view-person.php3?entity_id=$entity_id");
?>
