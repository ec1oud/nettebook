<?
	function find_name($ent_id)
	{
		$retval = "UNKNOWN";
		$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
		$res = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$retval = "<a href = view-person.php3?entity_id=$ent_id>";
			$r = pg_fetch_array ($res, 0);
			$retval = $retval . $r[othernames] . " " . $r[lastname];
			if ($suffixes)
				$retval = $retval . ", " . $r[suffixes];
			$retval = $retval . "</a>";
			return $retval;
		}
		$res = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$r = pg_fetch_array ($res, 0);
			return "<a href = view-thing.php3?entity_id=$ent_id>$r[name]</a>";
		}
	}
?>
