<?
	include 'authenticate.inc';
	function new_relationship_link($ent_id)
	{
		$retval = "<A>";
		$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
		$res = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$retval = "<a href = enter-relationship.php3?entity_id=$ent_id&direction=fwd>";
			return $retval;
		}
		$res = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$r = pg_fetch_array ($res, 0);
			$res = pg_Exec ($conn, "SELECT * from thing_type WHERE type_id=$r[type]");
			$r = pg_fetch_array ($res, 0);
			$retval = "<a href = enter-relationship.php3?entity_id=$ent_id&selected_type=$r[default_relationship]&direction=rev>";
			return $retval;
		}
	}
	echo "<table width = 100%><tr bgcolor=lightyellow><th width = 50% align=left>Relationship</th><th width = 50% align=left>Comment</th><th>" . new_relationship_link($entity_id) . "New</a></th></tr>";
	include 'search-relationships.inc';
	echo "</table>\n";
?>
