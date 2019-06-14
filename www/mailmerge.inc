<? 
	include "household.inc";

	function fixNewlines($input, $method)
	{
		if ($method == "leaveit")
			return $input;
		// Make usage of CR/LF consistent - replace with LFs
		$input = str_replace("\r\n", "\n", $input);
		$input = str_replace("\n\r", "\n", $input);
		$input = str_replace("\r", "\n", $input);

		// Now replace LFs with whatever was requested
		if ($method == "crlf")
			return str_replace("\n", "\r\n", $input);
		if ($method == "slash-n")
			return str_replace("\n", "\\n", $input);
		if ($method == "slash-r-slash-n")
			return str_replace("\n", "\\r\\n", $input);
		if ($method == "br")
			return str_replace("\n", "<BR>", $input);
		return $input;
	}

	// Separate the lines of the input and return an array.
	// Assumes there will be only newlines, no CRs.
	function parseLines($input)
	{
		$lastFound = strpos($input, "\n");
		while ($lastFound)
		{
			$retval[] = substr($input, 0, $lastFound);
			$input = substr($input, $lastFound + 1);
			$lastFound = strpos($input, "\n");
		}
		if ($input)
			$retval[] = $input;
		return $retval;
	}

	// Given some lines of text, apply each of them in turn to the given 
	// function in the input.  Recursive.
	function handleForEach($functionName, $lines, $input)
	{
		$lastFound = strpos($input, $functionName);
		if (!$lastFound)
			return $input;
		// Find the end of the function.
		$fnEnd = strpos($input, "\")", $lastFound) + 1;
		if (!$fnEnd)
			return $input;
		// Find the beginning of the function parameter.
		$fnParmBeg = strpos($input, "\"", $lastFound);
		// Extract the text to output for each line.  Remove it from input.
		$output = substr($input, $fnParmBeg + 1, $fnEnd - $fnParmBeg - 2);
		$trailer = substr($input, $fnEnd + 1);
		$retval = substr($input, 0, $lastFound);
		// For each value in lines, output the given text replacing the 
		// <line/> variable.
        $count = count($lines);
        for ($i = 0; $i < $count; ++$i)
		{
               $retval = $retval . str_replace("<line/>", $lines[$i], str_replace("<lineNumber/>", $i, $output));
		}
		$retval = $retval . $trailer;
		if (strpos($input, $functionName))
			$retval = handleForEach($functionName, $lines, $retval);
		return $retval;
	}
?>

