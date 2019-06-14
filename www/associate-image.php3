<?
	include 'authenticate.inc';
	if (!$entity_id)
	{
		echo "you must specify entity_id";
		exit;
	}
	if (!$image)
	{
		echo "you must specify image";
		exit;
	}
	if (!$thumbnail)
	{
		echo "you must specify thumbnail";
		exit;
	}
	
	$dest = $DOCUMENT_ROOT . "/nettebook/images/entities/" . $entity_id . "-" . substr($image, strrpos($image, "/") + 1);
	if (!copy($image, $dest))
	{
		echo "failed to copy file";
		exit;
	}
	
	$thumbdest = $DOCUMENT_ROOT . "/nettebook/images/entities/" . $entity_id . "-" . substr($thumbnail, strrpos($thumbnail, "/") + 1);
	if (!copy($thumbnail, $thumbdest))
	{
		echo "failed to copy file";
		exit;
	}
	
	unlink($image);
	unlink($thumbnail);
	
	$insertquery = "INSERT into image VALUES ($entity_id, '$name', '$caption', $type, '" . 
		substr($dest, strrpos($dest, "/") + 1) . "', '" .
		substr($thumbdest, strrpos($thumbdest, "/") + 1) .
		"')";
	$result = pg_Exec ($conn, $insertquery);
	if (!$result) 	
	{
		echo "Database error occured; files saved but not associated\n";
	    exit;
	}
/*
	echo $insertquery. "<BR>";
	echo "copying to $dest<BR>";
	echo "copying to $thumbdest<BR>";
	echo $insertquery. "<BR>";
	chmod($dest, "a+r-x");
	chmod($thumbdest, "a+r-x");
*/	
	Header("Location: view-person.php3?entity_id=$entity_id");
?>
