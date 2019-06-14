<?
	include 'authenticate.inc';
	if ($category == "")
	{
		echo "category is required";
		exit;
	}
	if ($title == "")
	{
		echo "category is required";
		exit;
	}
	$result = pg_Exec ($conn, "DELETE FROM general_purpose WHERE owner_id = $owner_id AND category='$category' AND title='" . urldecode($title) . "'");
	if (!$result) 	
	{
		echo "An error occured during DELETE\n";
	    exit;
	}
	echo "Deleted.";
?>
