<center>
<? 
	include 'authenticate.inc';
    // Get the owner's address.
    // TODO - if the sender has more than one address, he should have been
    // forced to choose one as a return address at some point before this.
    // For now, just use the first one and hope it's right.
    $query = "select address from location where entity_id=$owner_id";
    $result = pg_Exec ($conn, $query);
    $row = pg_fetch_array ($result, $rc);
    $owner_address = $row[address];

	echo "<form action=update-mailmerge-template.php3 method=post>\n";
	if (!($creator_id))
		$creator_id = $owner_id;
	echo "<input type=hidden name=creator_id value=$creator_id>\n";
	if ($template_id)
	{
		echo "<input type=hidden name=template_id value=$template_id>\n";
		$result = pg_Exec ($conn, "SELECT * FROM mailmerge_template WHERE template_id='$template_id'");
		if ($result && pg_numrows($result) > 0)
		{
			$row = pg_fetch_array ($result, 0);
			$template=$row[template];
			$template_name=$row[name];
		}
	}
	echo "Template<BR>\n";
	echo "Name: <input type=text value=\"$template_name\" size=50 name=template_name>\n";
	echo "<TEXTAREA COLS=80 NAME=template ROWS=20 WRAP=OFF>";
	echo "$template";
	echo "</TEXTAREA><BR>\n";
	echo "<input type=submit value=enter>\n";
	echo "</form>\n";
	echo "<table border=1>\n";
	echo "  <tr><th align=left colspan=2>Available variables:</th></tr>\n";
	echo "	<tr valign=top><td>&lt;sender/&gt;</td><td>Full name of sender, \"<tt>$owner_othernames $owner_lastname</tt>\"</td></tr>\n";
	echo "	<tr valign=top><td>&lt;senderAddress/&gt;</td><td>Full address of sender, like this:\n";
	echo "    <blockquote><pre>$owner_address</pre></blockquote></td></tr>\n";
	echo "	<tr valign=top><td>&lt;householdNames/&gt;</td><td>Full name of recipient household, e.g. \"<tt>John, Mary and Johnny Doe</tt>\"</td></tr>\n";
	echo "	<tr valign=top><td>&lt;address/&gt;</td><td>Recipient address</td></tr>\n";
	echo "	<tr valign=top><td>&lt;firstName/&gt;</td><td>first name of primary recipient, e.g. \"<tt>John</tt>\"</td></tr>\n";
	echo "	<tr valign=top><td>&lt;otherNames/&gt;</td><td>non-last names of primary recipient, e.g. \"<tt>John J.</tt>\"</td></tr>\n";
	echo "	<tr valign=top><td>&lt;lastName/&gt;</td><td>last name of primary recipient, e.g. \"<tt>Doe</tt>\"</td></tr>\n";
	echo "	<tr valign=top><td>&lt;line/&gt;</td><td>The current line</td></tr>\n";
	echo "	<tr valign=top><td>&lt;lineNumber/&gt;</td><td>The current line number, counting up from zero</td></tr>\n";
	echo "  <tr><th align=left colspan=2>Available functions:</th></tr>\n";
	echo "	<tr valign=top><td>forEachSenderAddressLine(\"format of &lt;line/&gt;\")</td><td>output to be repeated for each line in the sender's address; within your \"format\" string, &lt;line\&gt; will be substituted with that line of the address.</td></tr>\n";
	echo "	<tr valign=top><td>forEachAddressLine(\"format of &lt;line/&gt;\")</td><td>output to be repeated for each line in the recipient's address; within your \"format\" string, &lt;line\&gt; will be substituted with that line of the address.</td></tr>\n";
	echo "</table>\n";
?>
</center>
<P>
Your output can be in any form you like (XML, HTML, TeX, postscript, shell script, plain text, etc).
For each recipient, the text above will
be output as-is with the exception of having the variable names replaced with their content.</P>
<P>Because this template will be inserted into the database, you may not use single-quote characters within it.</P>
