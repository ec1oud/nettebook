<? 
	include 'authenticate.inc';
	if (!$entity_id) 
		echo "<form action=insert-company.php3 method=post>\n";
	else
	{
		echo "<form action=update-company.php3 method=post>\n";
		echo "<input type=hidden name=entity_id value=$entity_id>\n";
		$result = pg_Exec ($conn, "SELECT * FROM company WHERE entity_id=$entity_id");
		$row = pg_fetch_array ($result, 0);
		if (!$result) 	
		{
		    echo "An error occured during SELECT\n";
	        exit;
		}
		if (pg_numrows($result) == 0)
		{
			echo "no company found with entity_id $entity_id";
		    exit;
		}
		$name=$row["name"];
	}

	echo "<center>Name of company <input type=text name=name size=60 value=\"$name\"><BR><input type=submit value=enter></center>\n";
?>
