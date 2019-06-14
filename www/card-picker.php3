<?
	include 'authenticate.inc';
	if ($category == "")
	{
		echo "category is required";
		exit;
	}
	$result = pg_Exec ($conn, "SELECT * FROM general_purpose WHERE owner_id = $owner_id AND category='$category' AND title='" . urldecode($title) . "'");
	if (!$result) 	
	{
		echo "An error occured during SELECT\n";
	    exit;
	}
	if (pg_numrows($result) == 0)
	{
		echo "none found";
		exit;
	}
	$row = pg_fetch_array ($result, 0);
	echo "<P align=right>" . $row["last_mod"] . "</P>\n";
	$category = $row["category"];
	$body = $row["body"];
	echo "<form action=update-card.php3 method=POST>";
	echo "category: <input type=text name=category size=8 value=$category>\n";
	echo "title: <input type=text name=title size=59 value=\"" . urldecode($title) . "\"><BR><HR>\n";
	echo $body . "<BR>";
	echo "<textarea name=body rows=15 cols=80>$body</textarea><BR>\n";
	echo "<center>";
	echo "<input type=submit value=\"Submit Changes\">";
	echo "</center>";
	echo "</form>";
	echo "<P align=center><a href=delete-card.php3?category=$category&title=" . urlencode($title) . ">Delete this card</a></P>";
	echo "<P align=center><a href=entercard.php3?category=$category>Create a new card</a></P>";
?>
