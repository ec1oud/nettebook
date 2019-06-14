<?
	include 'authenticate.inc';
	$result = pg_Exec ($conn, "SELECT * FROM general_purpose WHERE owner_id = $owner_id ORDER BY category, title");
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
	echo "<table width=100% border=0 bgcolor=lightyellow>\n";
	echo "<tr bgcolor=white><th width=10%>Category</th><th width=80%>Title</th><th width=10%>Date</th></TR>";
	$even_row = false;
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "<tr";
		if ($even_row)
			echo " bgcolor=lightgreen";
		echo ">";
		echo "<td>" . $row["category"] . "</td>";
		echo "<td>" . $row["title"] . "</td>";
		echo "<td align = right>" . $row["last_mod"] . "</td>";
		echo "</tr>\n";
		echo "<tr";
		if ($even_row)
			echo " bgcolor=lightgreen";
		echo ">";
		echo "<td colspan=3>" . $row["body"] . "</td>";
		echo "</tr>\n";
		$even_row = !$even_row;
	}
	echo "</table>";
?>
