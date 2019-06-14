<?
	header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");             // Date in the past
	header("Last-Modified: " . gmdate("D, d M Y H:i:s") . "GMT"); // always modified
	header("Cache-Control: no-cache, must-revalidate");           // HTTP/1.1
	header("Pragma: no-cache");                                   // HTTP/1.0

	include 'authenticate.inc';
	$result = pg_Exec ($conn, "SELECT * from thing WHERE owner=$owner_id ORDER BY name");
	if (!$result)
	{
	    echo "An error occured during SELECT\n";
        exit;
	}
	echo "<table width=100%>\n";
	echo "<tr bgcolor=lightyellow><th width=99% align = left>Named Groups</th><th align=right><a href = enter-thing.php3?thing_owner=$owner_id>New</a></th></tr>\n";
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <tr><td><a href=view-thing.php3?entity_id=$row[entity_id]>$row[name]";
		echo "</a></td>\n";
		echo "      <td><a href=delete-thing.php3?entity_id=$row[entity_id]>delete</a></td></tr>\n";
	}
	echo "<tr bgcolor=lightyellow><th width=99% align = left>Mailmerge Templates</th><th align=right><a href = enter-mailmerge-template.php3?creator_id=$owner_id>New</a></th></tr>\n";
	$result = pg_Exec ($conn, "SELECT name, template_id from mailmerge_template WHERE creator=$owner_id ORDER BY name");
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <tr><td><a href=enter-mailmerge-template.php3?template_id=$row[template_id]>$row[name]";
		echo "</a></td>\n";
		echo "      <td><a href=delete-mailmerge-template.php3?template_id=$row[template_id]>delete</a></td></tr>\n";
	}
	echo "</table>\n";
?>