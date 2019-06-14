<center>
<?
	if(!isset($PHP_AUTH_USER)) 
	{
    	Header("WWW-Authenticate: Basic realm=\"nettebook\"");
	    Header("HTTP/1.0 401 Unauthorized");
		echo "You must login in order to use the database\n";
	    exit;
  	} 
	else 
	{
		$conn=pg_connect("dbname=nettebook user=wwwdata");
		if (!$conn) 
		{
  	  		echo "Cannot connect to database\n";
  	      	exit;
		}
		$result = pg_Exec ($conn, "SELECT * FROM \"user\" WHERE userid='$PHP_AUTH_USER' AND password='$PHP_AUTH_PW'");
		if (!$result) 
		{
			echo "login failed";
		    exit;
		}
		if (pg_numrows($result) == 0)
		{
			echo "login failed";
		    exit;
		}
		if (pg_numrows($result) > 1)
		{
			echo "<BR>This is odd: found " . pg_numrows($result) . " entries for $PHP_AUTH_USER<BR>";
			exit;
		}
		echo "<body bgcolor=white><P><B>$PHP_AUTH_USER</B> logged in</P>";
	}
?>
<P><a href=cardsframe.html target=main><img src=images/cards-icon.png alt=cards border=0><BR>Cardfile</a></P>
<P><a href=view-people.php3 target=main><img src=images/people-icon.png alt=people border=0><BR>People</a></P>
<P><a href=view-companies.php3 target=main><img src=images/company-icon.png alt=companies border=0><BR>Companies</a></P>
</center>
</body>
