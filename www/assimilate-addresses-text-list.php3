<!-- modes - 
	0: error - ignore the rest of this block
	1: expecting to see a name; 
	2: expecting to see an address; when this mode is terminated by
		a blank line, the location record will be inserted.
-->
<?
	include 'authenticate.inc';
	function getEntity($fullname, $conn)
	{
		$firstname = substr($fullname, 0, strpos($fullname, " "));
		$lastname = trim(substr($fullname, strrpos($fullname, " "), 255));
		$othernames = trim(substr($fullname, 0, strrpos($fullname, " ")));
		$result = pg_Exec ($conn, "select entity_id, othernames, lastname from person where lastname ~* '.*$lastname' and othernames ~* '.*$firstname.*'");
        if (pg_numrows($result) < 1)
		{
			echo "There are no matches for <TT>$fullname</TT>; adding";
			$lastname = ucwords($lastname);
			$othernames = ucwords($othernames);
			$result = pg_Exec ($conn, "INSERT INTO person VALUES ('$lastname', '$othernames')");
			$query = "select entity_id from person where ";
			$added = FALSE;
			if ($lastname)
			{
				$query = $query . "lastname='$lastname'";
				$added = TRUE;
			}
			if ($othernames)
			{
				if ($added)
					$query = $query . " AND ";
				$query = $query . "othernames='$othernames'";
				$added = TRUE;
			}
			$result = pg_Exec ($conn, $query);
			$row = pg_fetch_array ($result, 0);
		    return $row[entity_id];
		}
        if (pg_numrows($result) > 1)
		{
			echo "There is more than one match for <TT>$fullname</TT>:<BR>\n<BLOCKQUOTE>\n";
	        for ($rc = 0; $rc < pg_numrows($result); ++$rc)
	        {
                $row = pg_fetch_array ($result, $rc);
				echo "   $row[othernames] $row[lastname]<BR>\n";
			}
			echo "</BLOCKQUOTE>\n";
			return -1;
		}
        $row = pg_fetch_array ($result, 0);
		echo "$row[othernames] $row[lastname]";
		return $row[entity_id];
	}
	echo "You successfully uploaded a file with $userfile_size bytes in it.\n";
	$file = fopen($userfile, "r");
	if ($file)
	{
		echo "I'm looking at it...<BR>";
		$mode = 1;
		$line = fgets($file, 256);
		$errorCount = 0;
		$successCount = 0;
		while ($line)
		{
			$trimmedLine = trim($line);
			switch ($mode)
			{
			case 0:
				if (!($trimmedLine))
				{
					$mode = 1;
					echo "therefore no location record is being inserted.<P>\n";
				}
				break;
			case 1:
				if ($trimmedLine)
				{
					$entity_id = getEntity($trimmedLine, $conn);
					if ($entity_id >= 0)
					{
						$mode = 2;
						$address = "";
					}
					else
					{
						$errorCount++;
						$mode = 0;
					}
				}
				break;
			case 2:
				if ($trimmedLine)
					$address = $address . $trimmedLine . "\n";
				else
				{
					$mode = 1;
					$result = pg_Exec($conn, "insert into location values(null, null, '$address', $entity_id)");
					if ($result)
					{
						echo ", OK.<BR>";
						$successCount++;
					}
					else
					{
						echo ", something went wrong.";
						$errorCount++;
					}
				}
				break;
			}
			$line = fgets($file, 256);
		}
		fclose($file);
		echo "<P>All done.\n";
		if ($errorCount)
			echo "There were $errorCount ambiguities.\n"; 
		else
			echo "There were no problems.\n";
		if ($successCount)
			echo "$successCount records were inserted.\n";
		else
			echo "No database changes were made.\n";
		echo "</P>\n";
	}
?>
