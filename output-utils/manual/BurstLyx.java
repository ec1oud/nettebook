import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileWriter;

public class BurstLyx
{
	public static void main(String[] args)
	{
		if (args.length < 1)
		{
			System.out.println("Usage: java BurstLyx file-to-burst.lyx");
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
				if (line.startsWith("#file:") && out == null)
				{
					System.out.println("Writing to " + line.substring(7));
					out = new FileWriter(line.substring(7));
					out.write(buffer);
					out.write(line + "\n");
					buffer = "";
				}
				else if (out != null)
					if (line.startsWith("#"))
					{
						out.close();
						out = null;
						buffer += line + "\n";
					}
					else
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
