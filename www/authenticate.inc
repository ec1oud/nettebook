<?
	if(!isset($PHP_AUTH_USER)) 
	{
    	Header("WWW-Authenticate: Basic realm=\"nettebook\"");
	    Header("HTTP/1.0 401 Unauthorized");
		echo "You must login in order to use the database\n";
  	} 
	else 
	{
		$conn=pg_connect("dbname=nettebook host=localhost port=5432 user=wwwdata");
		if (!$conn) 
		{
  	  		echo "Cannot connect to database\n";
			exit;
		}
		$result = pg_Exec ($conn, "SELECT * FROM \"user\" WHERE userid='$PHP_AUTH_USER' AND password='$PHP_AUTH_PW'");
		if (!$result) 
		{
			echo "login failed";
			unset($PHP_AUTH_USER);
			exit;
		}
		if (pg_numrows($result) == 0)
		{
			echo "login failed";
			unset($PHP_AUTH_USER);
			exit;
		}
		if (pg_numrows($result) > 1)
		{
			echo "<BR>This is odd: found " . pg_numrows($result) . " entries for $PHP_AUTH_USER<BR>";
			unset($PHP_AUTH_USER);
			exit;
		}
		$row = pg_fetch_array ($result, 0);
		$owner_id = $row["entity_id"];
		$result = pg_Exec ($conn, "SELECT * FROM person WHERE entity_id=$owner_id");
		$row = pg_fetch_array ($result, 0);
		$owner_lastname = $row[lastname];
		$owner_othernames = $row[othernames];
	}
?>
