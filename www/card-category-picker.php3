<?
	include 'authenticate.inc';
	$result = pg_Exec ($conn, "SELECT DISTINCT category FROM general_purpose WHERE owner_id = $owner_id ORDER BY category");
	if (!$result) 	
	{
		echo "An error occured during SELECT\n";
	    exit;
	}
	echo "<table width=100% bgcolor=lightyellow border=0 hspace=0 cellspacing=0><tr align=left><th>Categories</th></tr></table>";
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "<a href=card-title-picker.php3?category=";
		echo $row["category"];
		echo " target=titles>";
		echo $row["category"];
		echo "</a><BR>\n";
	}
?>
