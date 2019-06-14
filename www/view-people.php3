<?
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");             // Date in the past
	header("Last-Modified: " . gmdate("D, d M Y H:i:s") . "GMT"); // always modified
	header("Cache-Control: no-cache, must-revalidate");           // HTTP/1.1
	header("Pragma: no-cache");                                   // HTTP/1.0

	include 'authenticate.inc';
	$result = pg_Exec ($conn, "SELECT * FROM person ORDER BY lastname, othernames");
	if (!$result) 	
	{
	    echo "An error occured during SELECT\n";
        exit;
	}
	echo "<table width=100%>\n";
	echo "<tr bgcolor=lightyellow><th width=99% align = left>People</th><th align=right><a href = enter-person.php3>New</a></th></tr>\n";
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <tr><td><a href=view-person.php3?entity_id=$row[entity_id]>$row[lastname]";
		if ($row[othernames])
			echo ", $row[othernames]";
		if ($row[suffixes])
			echo ", $row[suffixes]";
		echo "</a></td>\n";
		echo "   <td><a href=delete-person.php3?entity_id=$row[entity_id]>delete</a></td></tr>";
	}
	echo "</table>\n";
?>
