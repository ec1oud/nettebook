This is how I use the mailmerge feature to label envelopes.  I have an 
old daisy-wheel printer which I can feed them into one at a time.  So
I needed a shell script that will dump one address to the printer and
wait for me to press a key to continue.  To use it, simply create a new
mailmerge template using the browser, and into the big text field, paste
everything below the line of hyphens.  When you do the merge you will
get one big long shell script with commands to print all the addresses.
My printer takes "carriage return" and "line feed" very literally, so 
I had to send it both to get to the beginning of a new line.  If your
printer double-spaces when you use this, adjust accordingly.
------------------
read -p "Next..."
/bin/echo -e "<sender/>\r" > /dev/lp0
forEachSenderAddressLine("
/bin/echo -e "<line/>\r" > /dev/lp0")
/bin/echo -e "\r\n\n\n\n\n\n\n\n\n" > /dev/lp0
/bin/echo -e "                              <householdNames/>\r" > /dev/lp0
forEachAddressLine("
/bin/echo -e "                              <line/>\r" > /dev/lp0")
/bin/echo -e "\r\n\n\n\n\n\n\n\n\n\n\n" > /dev/lp0

