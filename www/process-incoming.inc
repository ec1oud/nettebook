<?
function jpgsAndThumbnails()
{
	if ($dir = @opendir("/var/www/nettebook/incoming")) 
	{
	  while($file = readdir($dir)) 
	  {
	  	if ($file[0] != "." && $file != "thumbnail" && $file != "jpg")
		{
			$filestub = $file;
			$dotpos = strrpos($file, ".");
			if ($dotpos)
			{
				$filestub = substr($file, 0, $dotpos);
			}
			exec(escapeshellcmd("/usr/bin/convert /var/www/nettebook/incoming/$file /var/www/nettebook/incoming/jpg/$filestub.jpg"));
	    	exec(escapeshellcmd("/usr/bin/convert -size 80x80 /var/www/nettebook/incoming/$file -resize 80x80 /var/www/nettebook/incoming/thumbnail/$filestub" . "_t.jpg"));
			unlink("/var/www/nettebook/incoming/$file");
		}
	  }  
	  closedir($dir);
	}
}
?>
