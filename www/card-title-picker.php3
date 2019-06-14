<?
	include 'authenticate.inc';
	if ($category == "")
	{
		echo "category is required";
		exit;
	}
	$result = pg_Exec ($conn, "SELECT category, title FROM general_purpose WHERE owner_id = $owner_id AND category='$category' ORDER BY title");
	if (!$result) 	
	{
		echo "An error occured during SELECT\n";
	    exit;
	}
	$count = pg_numrows($result);
	echo "<TABLE width=100% bgcolor=lightyellow border=0 hspace=0 cellspacing=0><TR><TD><B>$category</B></TD>";
	echo "<TD align=right>$count cards</TD>";
	echo "<TD align=right><a href = entercard.php3?category=$category target=card>new card</a></TD>";
	echo "</TR></TABLE>";
	if (pg_numrows($result) == 0)
	{
		echo "none found";
		exit;
	}
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "<a href=card-picker.php3?category=";
		echo $row["category"];
		echo "&title=";
		echo urlencode($row["title"]);
		echo " target=card>";
		echo $row["title"];
		echo "</a><BR>\n";
	}
?>
