import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;

public class BurstTeX
{
	public static void main(String[] args)
	{
		if (args.length < 1)
		{
			System.out.println("Usage: java BurstTeX file-to-burst.tex");
			System.exit(1);
		}
		System.out.println("bursting " + args[0]);
		try
		{
			BufferedReader br = new BufferedReader(new FileReader(args[0]));
			FileWriter out = null;
			String line = br.readLine();
			String buffer = "";
			while (line != null)
			{
				if (line.startsWith("%% file:"))
				{
					if (out != null)
						out.close();
					System.out.println("Writing to " + line.substring(9));
					out = new FileWriter(line.substring(9));
					out.write(buffer);
					out.write(line + "\n");
					buffer = "";
				}
				else if (out != null)
				{
					out.write(line);
					out.write("\n");
				}
				else
					buffer += line + "\n";
				line = br.readLine();
			}
			out.close();
		}
		catch (Exception e)
		{
			System.out.println("Didn't succeed: " + e);
		}
	}
}
