<?
	include 'authenticate.inc';
	if ($category == "")
	{
		echo "Category is a required field.";
		exit;
	}
	if ($title == "")
	{
		echo "Title is a required field.";
		exit;
	}
	$result = pg_Exec ($conn, "SELECT category FROM general_purpose WHERE owner_id=$owner_id and category='$category' and title='$title'");
	if (!$result) 
	{
		echo "An error occured during search for existing records";
	    exit;
	}
	if (pg_numrows($result) == 0)
	{
		echo "Inserting new row into database...<BR>";
		$result = pg_Exec ($conn, "INSERT INTO general_purpose VALUES ($owner_id, '$category', '$title', '$body')");
	}
	else
	{
		echo "Updating existing record in database...<BR>";
		$result = pg_Exec ($conn, "UPDATE general_purpose set body='$body' WHERE owner_id=$owner_id and category='$category' and title='$title'");
	}
	if (!$result) 	
	{
		echo "An error occured.\n";
	    exit;
	}
	echo "Done.";
?>
