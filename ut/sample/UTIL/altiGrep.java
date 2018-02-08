import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


/**
 * Grep Utility for Altibase
 * 
 * Usage
 * 1. Grep from file(s): java altiGrep [OPTIONS] pattern [FILELIST]
 *    =============================================
 *    OPTIONS
 *    =============================================
 *    -from="yyyy/mm/dd hh:mm:ss": Start time for time-based search
 *    -to="yyyy/mm/dd hh:mm:ss": End time for time-based search
 *
 * 2. Grep from pipe: tail -f file | java altiGrep [OPTIONS] pattern
 *    =============================================
 *    OPTIONS
 *    =============================================
 *    -from="yyyy/mm/dd hh:mm:ss": Start time for time-based search
 *    -to="yyyy/mm/dd hh:mm:ss": End time for time-based search
 * 
 * Pattern
 * 1. Java regular expression or just string
 * 2. Example: "ERR-", "ERR-[\\d]0"
 * 
 * @author Choong Hun Kim (chkim@altibase.com)
 *
 */
public class altiGrep 
{
	private static boolean isFirstOutput = true;
	private static String[] files = null;

	private static MsgLog altiLog = null;
	
	private static boolean isMatched = false;
	private final static String findPattern= ".*?";
	private static String findString;

	// Regular expression for Header
	private final static Pattern pattern4Header 
			= Pattern.compile("^(\\[.*?\\]) (\\[.*?\\]) (\\[.*?\\])",
								Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
	private static Matcher matcher4Header;
	
	private final static Pattern pattern4Date 
	= Pattern.compile("((?:2|1)\\d{3}(?:-|\\/)(?:(?:0[1-9])|(?:1[0-2]))(?:-|\\/)(?:(?:0[1-9])|(?:[1-2][0-9])|(?:3[0-1]))(?:T|\\s)(?:(?:[0-1][0-9])|(?:2[0-3])):(?:[0-5][0-9]):(?:[0-5][0-9]))",
				Pattern.CASE_INSENSITIVE | Pattern.DOTALL);
	private static Matcher matcher4Date;
	
	// Time range
	private static boolean isTimeSearch = false;
	private final static SimpleDateFormat sdf
		= new SimpleDateFormat("yyyy/MM/dd hh:mm:ss");
	private static long fromTime = -1;
	private static long toTime = -1;

	private static void parseArgs(String option)
	{
		String[] pair = option.split("=");

		if (pair[0].equalsIgnoreCase("from"))
		{
			isTimeSearch =true;
			fromTime = getTime(pair[1]);
			if (toTime < 0)
				toTime = getTime("2999/12/30 12:00:00");
			
		} else if (pair[0].equalsIgnoreCase("to"))
		{
			isTimeSearch =true;
			toTime = getTime(pair[1]);
			if (fromTime < 0)
				fromTime = getTime("1970/01/01 00:00:00");
		}
		else
		{
			System.err.println("Unknown option: " + option);
			System.exit(-1);
		}
		
	}

	private static void init(String[] args)
	{
		int i = 0, count;
		boolean pattern = false;
		ArrayList fileList = new ArrayList();
		
		count = args.length;
		
		// [Option*] [pattern]
		while ((i < count) && (pattern == false))
		{
			if (args[i].charAt(0) == '-') // remove '-' for key value set
				parseArgs(args[i].substring(1));
			else
			{
				// set pattern
                // fix BUG-25887
				findString = findPattern + "(" + args[i] + ")" + findPattern;
				pattern = true;
			}
			i++;
		}
		
		// file list
		for(;i < count; i++)
		{
			fileList.add(args[i]);
		}
		
		files = (String[]) fileList.toArray(new String[fileList.size()]);
	}
	
	private static long getTime(String aTimeString)
	{
		long result = 0;
		try
		{
			result = sdf.parse(aTimeString).getTime();
		}
		catch (ParseException e)
		{
			System.err.println("Invalid format of time input: " + aTimeString);
			System.err.println(e.getLocalizedMessage());
			System.exit(-1);
		}
		
		return result;
	}
	
	private static boolean isInTimeRange(String aDateString)
	{
		long inputTime = getTime(aDateString);
		
		return (fromTime <= inputTime && inputTime <= toTime)? true:false;
	}
	
	private static void processLine(String aInput)
	{
		matcher4Header = pattern4Header.matcher(aInput);
		if(matcher4Header.find())
		{
			altiLog = null;
			
			if (isTimeSearch)
			{
				matcher4Date = pattern4Date.matcher(aInput);
				if (matcher4Date.find() && isInTimeRange(matcher4Date.group()))
				{
					altiLog = new MsgLog(matcher4Header.group());
					isMatched = false;
				}
				
			}else
			{
				altiLog = new MsgLog(matcher4Header.group());
				isMatched = false;
			}

		}
		else
		{
			if (altiLog == null)
				return;

			if (isMatched == true)
			{
				System.out.print(aInput);
			}
			else
			{
				altiLog.appendBody(aInput);
				if (hasPattern(aInput))
				{
					isFirstOutput = false;
					System.out.print(altiLog.toString());
					isMatched = true;
				}
			}
		}
	}
	
	private static boolean hasPattern(String aInput)
	{
		return Pattern.compile(findString,Pattern.CASE_INSENSITIVE | Pattern.DOTALL)
		.matcher(aInput).matches();
	}

	private static void getContents(BufferedReader aBufReader) 
	{
		try {
			String line = null;
			while (( line = aBufReader.readLine()) != null){
				if (line.length() > 0)
					processLine(line);// + System.getProperty("line.separator"));
			}
		}
		catch (IOException ex){
			ex.printStackTrace();
		}
	}

    private static void init4NewFile()
    {
        altiLog = null;
        isMatched = false;
    }

	public static void main(String[] args)
	{
		if (args.length < 1) 
		{
			System.err.println("Usage: ");
            System.err.println(" 1. Search pattern with files: java altiGrep [OPTTIONS] PATTERN [FILE...]");
            System.err.println(" 2. Search pattern with tail:  tail -f file | java altiGrep [OPTTIONS] PATTERN");
            System.err.println("where possible options include:");
            System.err.println(" -from=\"yyyy/mm/dd hh:mm:ss\" : Start time for time-based search");
            System.err.println(" -to=\"yyyy/mm/dd hh:mm:ss\"   : End time for time-based search");
            System.err.println("");
            return;
		}

		init(args);

		BufferedReader br = null;
		try {
			// File input
			if (files.length > 0)
			{
				for (int i = 0; i < files.length; i++) 
				{
					br = new BufferedReader(new FileReader(new File(files[i])));
					getContents(br);
                    init4NewFile();
					br.close();
				}
			}
			// Pipe input
			else
			{
				br = new BufferedReader(new InputStreamReader(System.in));
				getContents(br);
				br.close();
			}
		} catch (FileNotFoundException e) 
		{
			e.printStackTrace();
		} catch (IOException e)
		{
			e.printStackTrace();
		}

        if(isFirstOutput == false)
            System.out.println("");

	}

	static class MsgLog {
		private String mHeader = null;
		private StringBuffer mBody = null;

		public MsgLog(String aHeader)
		{
			if (isFirstOutput)
				mHeader = aHeader;
			else
				mHeader = System.getProperty("line.separator") + aHeader;
			mBody = new StringBuffer();
		}

		public void appendBody(String aBody)
		{
			mBody.append(' ');
			mBody.append(aBody);
		}

		public boolean hasPattern(String aRegEx)
		{
			//System.out.println(mBody);
			return Pattern.compile(aRegEx,Pattern.CASE_INSENSITIVE | Pattern.DOTALL)
			.matcher(mBody.toString()).matches();
		}

		public String toString()
		{
			return mHeader + mBody;
		}
	}
}
