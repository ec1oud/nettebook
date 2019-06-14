<?
	include 'authenticate.inc';
	$result = pg_Exec ($conn, "SELECT * from thing WHERE entity_id=$entity_id");
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
	if (pg_numrows($result) > 1)
		echo "Warning: more than one thing with ID $entity_id was found<BR>\n";
	$row = pg_fetch_array ($result, 0);
	echo "<table width=100%><tr><td><font size=+1>$row[name]";
	$thing_type = $row[type];
	$thing_name = $row[name];
	echo "</font></td>";
	echo "<td align=right><a href=enter-thing.php3?entity_id=$row[entity_id]&thing_type=$thing_type>edit </a>";
	echo "<a href=delete-thing.php3?entity_id=$row[entity_id]>delete</a></td></table>";
	echo "<table width=100%>\n<tr bgcolor=lightyellow><th align=left width=50%>Location</th>\n";
	echo "<th  width=50% align=left>Address</th><th align=right> <a href=enter-location.php3?entity_id=$entity_id&is_thing=true>new</a></th></tr>\n";
	$result = pg_Exec ($conn, "SELECT * from thing_type WHERE type_id=$thing_type");
	$row = pg_fetch_array ($result, 0);
	$default_relationship = $row[default_relationship];
	$result = pg_Exec ($conn, "SELECT * from relationship_type WHERE type_id=$default_relationship");
	$row = pg_fetch_array ($result, 0);
	$default_relationship_name = strtoupper(substr($row[name], 0, 1)) . strtolower(substr($row[name], 1));

	$result = pg_Exec ($conn, "SELECT * FROM location WHERE entity_id=$entity_id ORDER BY name");
	if (pg_numrows($result) > 0)
	{
		for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
			$row = pg_fetch_array ($result, $rc);
			echo "   <td valign=top><FONT size=+1>$row[name]</FONT><BR>$row[description]</td><td valign=top><pre>$row[address]</pre></td>\n";
			echo "   <td align=right valign=top><a href=enter-location.php3?location_id=$row[location_id]&entity_id=$entity_id&is_thing=true>edit</a> ";
			echo "<a href=delete-location.php3?location_id=$row[location_id]&entity_id=$entity_id&is_thing=true>delete</a></td></tr>\n";
		}
	}
	echo "</table>\n";
	$result = pg_Exec ($conn, "SELECT * FROM contact_method WHERE entity_id=$entity_id ORDER BY location_id");
	echo "<table width=100%>\n<tr bgcolor=lightyellow><th align=left width=40%>Contact Method </th><th align=left width=10%>Type</th><th align=left width=25%>Location</th>\n";
	echo "<th align=left width=25%>Description</th><th align=right><a href=enter-contact-method.php3?entity_id=$entity_id&is_thing=1>new</a></th></tr>\n";
	if (pg_numrows($result) > 0)
	{
		for ($rc = 0; $rc < pg_numrows($result); ++$rc)
		{
			$row = pg_fetch_array ($result, $rc);
			if (strpos($row[detail], "@"))
				echo "   <td valign=top><a href=mailto:$row[detail]>$row[detail]</a></td valign=top>\n";
			else
				echo "   <td valign=top>$row[detail]</td valign=top>\n";
			echo "   <td>$row[type]</td>\n";
			if ($row[location_id])
			{
				$result2 = pg_Exec ($conn, "SELECT * FROM location WHERE location_id=" . $row[location_id]);
				if (pg_numrows($result2) > 0)
				{
					$row2 = pg_fetch_array ($result2, 0);
					echo "   <td>$row2[name]</td>\n";
				}
				else
					echo "   <td></td>\n";
			}
			else
				echo "   <td></td>\n";
			echo "   <td>$row[description]</td>\n";
			$detail = rawurlencode($row[detail]);
			echo "   <td align=right valign=top><a href=enter-contact-method.php3?entity_id=$entity_id&detail=$detail&is_thing=1>edit</a> ";
			echo "<a href=delete-contact-method.php3?entity_id=$entity_id&detail=$detail&is_thing=1>delete</a></td></tr>\n";
		}
	}
	echo "</table>\n";
	echo "<table width=100%>\n";
	echo "<tr bgcolor=lightyellow><th width=50% align=left>$default_relationship_name Relationships</th>\n";
	echo "<th width=50% align=left>Comments</th>";
	echo "<th align=right>";
	echo "<a href=enter-relationship.php3?entity_id=$entity_id&is_thing=1&selected_type=$default_relationship&direction=rev>new</a></th></tr>\n";
	$is_thing = 1;
	$relationship_type = $default_relationship;
	$terse = true;
	echo "<tr><td colspan=2>";
	include 'search-relationships.inc';
	echo "</td></tr>";
	echo "<tr><td colspan=3 align=right><hr>";
	echo "<a href=view-relationshipees-with-contact-methods.php3?entity_id=$entity_id>contact list</a>&nbsp;&nbsp;&nbsp;&nbsp;";
	echo "<a href=mailmerge-setup.php3?entity_id=$entity_id&relationship_type=$relationship_type&thing_name=";
	echo urlencode($thing_name) . ">mail merge</a>";
	echo "</td></tr>";
	echo "</table>\n";
	echo "<table width=100%>\n";
	echo "<tr bgcolor=lightyellow><th width=100% align=left>Other Relationships</th>\n";
	echo "<th align=right><a href=enter-relationship.php3?entity_id=$entity_id&is_thing=1>new</a></th></tr>\n";
	echo "<tr><td colspan=2><form action=search-relationships.php3?is_thing=1 method=post>\n";
	echo "<input type=hidden name=entity_id value=$entity_id>\n";
	echo "<table width=100%>\n";
	echo "<tr><td valign=top>Search:</td>\n";
	echo "<td valign=top>Relationship type<BR><select name=relationship_type size=1>\n";
	echo "   <option value=ALL>ALL\n";
	$result = pg_Exec ($conn, "SELECT * FROM relationship_type ORDER BY type_id");
	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	{
		$row = pg_fetch_array ($result, $rc);
		echo "   <option value=$row[type_id]>$row[name]\n";
	}
	echo "</select></td>\n";
	echo "<td valign=top>Name to search for (optional)<BR><input type=text name=other_entity_name></td>\n";
	echo "<td valign=bottom align=right><input type=submit value=search></tr></form></table></td>\n";
	echo "</tr></table>\n";
    $result = pg_Exec ($conn, "SELECT * FROM entity_comment WHERE entity_id=$entity_id ORDER BY last_mod");
    echo "<table width=100%>\n<tr bgcolor=lightyellow><th width=99% align=left>Comments</th>\n";
    echo "<th align=right><a href=enter-entity-comment.php3?entity_id=$entity_id&is_thing=true>new</a></th></tr>\n";
    if (pg_numrows($result) > 0)
    {
        for ($rc = 0; $rc < pg_numrows($result); ++$rc)
        {
            $row = pg_fetch_array ($result, $rc);
           	echo "   <td><i>$row[last_mod]</i><BR>$row[comment]</td>\n";
            echo "   <td align=right valign=top><a href=enter-entity-comment.php3?comment_id=$row[comment_id]&is_thing=true&entity_id=$entity_id>edit</a> ";
			echo "<a href=delete-entity-comment.php3?comment_id=$row[comment_id]&is_thing=true&entity_id=$entity_id>delete</a></td></tr>\n";
        }
    }
    echo "</table>\n";
?>
