<? 
	include 'authenticate.inc';
	function find_name($ent_id, $conn)
	{
		$retval = "UNKNOWN";
		$res = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$r = pg_fetch_array ($res, 0);
			$retval = $r[othernames] . " " . $r[lastname];
			if ($suffixes)
				$retval = $retval . ", " . $r[suffixes];
			return $retval;
		}
		$res = pg_Exec ($conn, "SELECT * FROM company WHERE entity_id=$ent_id");
		if (!$res)
			return $retval;
		if (pg_numrows($res) > 0)
		{
			$r = pg_fetch_array ($res, 0);
			return $r[name];
		}
	}

	$sentence_usage = urldecode($sentence_usage);
	$one_name = find_name($one_entity, $conn);
	$other_name = find_name($other_entity, $conn);
	echo "<P><B>New relationship - sentence usage</B></P>\n";
	echo "<P>Please fill in the blanks:</P>\n";
	echo "<form action=update-reltype-grammar.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=is_company value=$is_company>\n";
	echo "<input type=hidden name=sentence_usage value=\"$sentence_usage\">\n";
	echo "<P>Given that $one_name $sentence_usage $other_name,<BR>\n";
	echo "$other_name <input type=text name=reverse_sentence_usage length=10> $one_name.</P>\n";
	echo "<P>Please enter a unique noun to describe the type of the relationship, as in:<BR>\n";
	echo "$one_name has a/an <input type=text name=noun length=10> relationship with $other_name.</P>\n";

	echo "<center><input type=submit value=enter></center>\n";
	echo "</form>\n";
?>
