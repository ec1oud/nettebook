<?
	include "authenticate.inc";
	include "process-incoming.inc";
	include "household.inc";
	jpgsAndThumbnails();
	
	if (!$entity_id)
	{
		echo("You must choose an entity with which to associate incoming images");
		exit;
	}
	if ($dir = @opendir("/var/www/nettebook/incoming/thumbnail")) 
	{
	  echo "Choose an image to associate with <font size=\"+1\">" . getFullName($entity_id) . ":</font><BR>";
      $result = pg_Exec ($conn, "SELECT * FROM image_type ORDER BY type_id");
	  echo "<multicol cols=3>\n";
	  while($file = readdir($dir)) 
	  {
	  	if ($file[0] != ".")
		{
			$filestub = substr($file, 0, strrpos($file, "_t.jpg"));
			echo "\n<form method=post action=associate-image.php3>\n";
			echo "<input type=hidden name=entity_id value=$entity_id>\n";
			echo "<input type=hidden name=image value=\"incoming/jpg/$filestub.jpg\">\n";
			echo "<input type=hidden name=thumbnail value=\"incoming/thumbnail/$file\">\n";
			echo "<table><tr><td valign=top rowspan=5><a href=\"incoming/jpg/$filestub.jpg\"><img border=0 src=\"incoming/thumbnail/$file\"></a></td>";
			echo "<td valign=bottom bgcolor=lightyellow colspan=2>$filestub</td></tr>\n<tr>\n";
			echo "<td valign=bottom>Associate as</td><td><select name=type>\n";
				
        	for ($rc = 0; $rc < pg_numrows($result); ++$rc)
        	{
                $row = pg_fetch_array ($result, $rc);
				echo "<option value=" . $row[type_id] . ">" . $row[name] . "</option>\n";
			}
			echo "</select></td></tr><tr><td valign=bottom>with name</td><td valign=bottom><input type=text name=name size=17></td></tr>\n";
			echo "<tr><td valign=top colspan=2>and caption<BR><textarea name=caption cols=30 rows=2 wrap=soft></textarea></td></tr>\n";
			echo "<tr><td valign=bottom colspan=2><input type=submit></td></tr>";
			
			echo "</table></form>";
		}
	  }  
	  closedir($dir);
	}
	else
	{
		echo "no files in incoming directory";
	}
	
?>
</multicol>
