<?
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "No company ID given, cannot uniquely identify record";
		exit;
	}
	if (!$sure)
	{
		echo "<center>";
		echo "<P>Are you sure you want to delete ";
	    $result = pg_Exec ($conn, "SELECT * from group_entity WHERE entity_id=$entity_id");
		if (!$result) 	
		{
			echo "An error occured.\n";
		    exit;
		}
        $row = pg_fetch_array ($result, 0);
        echo "<font color=red size=+1>$row[name]</font> along with all associated records?</P>";
		echo "<a href = delete-group.php3?entity_id=$entity_id&sure=yes><img src = images/yes.png alt=YES></a>";
		echo "<a href = view-group.php3?entity_id=$entity_id><img src = images/no.png alt=NO></a>";
		echo "</center>";
		exit;
	}

	$result = pg_Exec ($conn, "DELETE from group_entity WHERE entity_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM conversation WHERE owner_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM conversation WHERE other_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM contact_method WHERE entity_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM location WHERE entity_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM entity_comment WHERE entity_id=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM relationship_assoc WHERE one_entity=$entity_id OR other_entity=$entity_id");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
    Header("Location: view-companies.php3");
?>
