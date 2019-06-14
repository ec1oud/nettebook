<center>
<? 
	include 'authenticate.inc';
	echo "<form action=update-location.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=is_thing value=$is_thing>\n";
	if ($location_id)
	{
		echo "<input type=hidden name=location_id value=$location_id>\n";
		$result = pg_Exec ($conn, "SELECT * FROM location WHERE location_id='$location_id'");
		if ($result && pg_numrows($result) > 0)
		{
			$row = pg_fetch_array ($result, 0);
			$description=$row["description"];
			$name=$row["name"];
			$address=$row["address"];
		}
	}
	echo "<table>\n";
	echo "<tr><td>";
	echo "Name of Location<BR>\n";
	echo "<input type=text name=name size=62 value=\"$name\" ></td></tr>\n";
	echo "<tr><td>";
	echo "Description<BR>\n";
	echo "<TEXTAREA COLS=60 NAME=description ROWS=5 WRAP=SOFT>";
	echo "$description";
	echo "</TEXTAREA></td></tr>\n";
	echo "<tr><td>";
	echo "Address<BR>\n";
	echo "<TEXTAREA COLS=60 NAME=address ROWS=5 WRAP=SOFT>";
	echo "$address";
	echo "</TEXTAREA></td></tr>\n";
	echo "</table>\n";
	echo "<input type=submit value=enter>\n";
		
	echo "</form>\n";
?>
</center>
