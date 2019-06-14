<?
	include 'authenticate.inc';
	if (!$name)
	{
		echo "You must enter the company name.";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	$owner_id = $row["entity_id"];
	$name = ucwords($name);

	$result = pg_Exec ($conn, "INSERT INTO company VALUES ('$name')");
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
    Header("Location: view-companies.php3");
?>
