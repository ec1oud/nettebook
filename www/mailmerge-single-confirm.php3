<? 
	include "authenticate.inc";
	include "find-name.inc";
	if (!$template_id)
	{
		echo "You must chooose a template.";
		exit;
	}
	$query = "select location_id, address from location where entity_id=$entity_id";
	$result = pg_Exec ($conn, $query);
	echo "<center><form action=mailmerge-single.php3 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<input type=hidden name=template_id value=$template_id>\n";
	echo "<input type=hidden name=newline_handling value=$newline_handling>\n";
	$address_count = pg_numrows($result);
	if ($address_count == 1)
	{
		echo "The following address will be used for sending mail to <font size=+1>";
		echo find_name($entity_id) . "</font>:<BR>\n";
      	$row = pg_fetch_array ($result, 0);
		echo "<table><tr><td><pre>$row[address]</pre></td></tr></table>";
		echo "<input type=hidden name=selected_address value=$row[location_id]>\n";
	}
	else
	{
		echo "Please select which address to use for sending mail to <font size=+1>";
		echo find_name($entity_id) . "</font>:<BR>\n";
		echo "<table>\n";
	 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
  	      	$row = pg_fetch_array ($result, $rc);
			echo "  <tr valign=top><td><input type=radio name=selected_address value=$row[location_id]></td>";
			echo "<td><pre>$row[address]</pre></td></tr>\n";
  	  	}
		echo "</table>";
	}
	$query = "select * from output_device";
	$result = pg_Exec ($conn, $query);
	if ($address_count == 1)
		echo "Please select where to send the output:<BR>\n";
	else
		echo "and select where to send the output:<BR>\n";
	echo "<select name=selected_output size=1>\n";
 	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
        $row = pg_fetch_array ($result, $rc);
		echo "   <option value=$row[id]>$row[name]\n";
	}
	echo "</select>";
	echo "<BR><input type=submit value=\"Do the Merge\">\n";
	echo "</form></center>";
?>

