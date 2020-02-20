//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define STRBUFSIZ 	10240

//
// Usage: ios-syslog-helper <"now"|iso8601-start-time> <process-name> [prefix]
//

int
main(int argc, char **argv)
{
	const char *procNameStr = NULL;
	const char *prefixStr = NULL;
	const char *startTimeStr = NULL;
	const char *startTimeFmt = "%Y-%m-%dT%H:%M:%S"; // ISO8601
	int printline = 0;
	int handleLevel = 0;

	if (argc < 4 || argc > 5)
	{
		fprintf(stderr, "Usage: %s <\"now\"|iso8601-start-time> <process-name> <prefix> [--level]\n", argv[0]);
		exit(1);
	}

	startTimeStr = argv[1];
	procNameStr = argv[2];
	prefixStr = argv[3];

	if (argc == 5)
	{
		handleLevel = 1;
	}

	if (prefixStr == NULL || *prefixStr == 0)
	{
		prefixStr = "";
	}

	time_t startTime = 0;

	if (strcmp(startTimeStr, "now") == 0)
	{
		// make "now" 30 seconds ago to allow things to settle
		startTime = time(NULL) - 30;
	}
	else
	{
		struct tm startTimeTM = { 0 };
		strptime(startTimeStr, startTimeFmt, &startTimeTM);

		if (startTimeTM.tm_year == 0)
		{
			fprintf(stderr, "%s: unable to parse date '%s' with format '%s'\n", argv[0], startTimeStr, startTimeFmt);
			exit(1);
		}

		startTime = mktime(&startTimeTM);
	}

	char buf[STRBUFSIZ+1];

	while (fgets(buf, STRBUFSIZ, stdin) != NULL)
	{
		// last character wont be a newline if the line was too long
		if (buf[strlen(buf)-1] != '\n')
		{
			// truncate very long lines
			buf[strlen(buf)-1] = '\n';

			// discard the rest of the line
			int c = 0;
			while ((c = fgetc(stdin)) != EOF)
			{
				if (c == '\n')
				{
					break;
				}
			}
		}

		char *bufPtr = buf;
		char hostnameBuf[STRBUFSIZ+1], processNameBuf[STRBUFSIZ+1], levelBuf[STRBUFSIZ+1], loglineBuf[STRBUFSIZ+1];

		if (buf[0] == '\t')
		{
			// continuation line: output if previous non-continuation line was output
			if (printline)
			{
				fputs(prefixStr, stdout);
				fputs(bufPtr, stdout);
			}
		}
		else
		{
			// not a continuation line
			//
			// parse the time stamp
			//  Dec  8 17:09:40 Perrys-iPhone-4S IDSRemoteURLConnectionAgent[123] <Warning>: WiFi is associated YES
			//

			printline = 0;

			const char *timestampFmt = "%b %e %T ";
			time_t now = time(NULL);
			struct tm tm = *gmtime(&now);  // initialize the tm structure to provide any fields missing in the timestamp (e.g. year)
			char *result = NULL;
			int numItems = 0;

			result = strptime(buf, timestampFmt, &tm);  // parse the timestamp
			bufPtr = result;

			if (bufPtr != NULL && *bufPtr != 0)
			{
				if (handleLevel)
				{
					// Expect a level like "<Warning>" on the log line (happens on device)
					numItems = sscanf(bufPtr, "%[^ ] %[^[][%*d] <%[^>]>: %[^\n]", hostnameBuf, processNameBuf, levelBuf, loglineBuf);
					if (numItems != 4)
					{
						// Can't parse line, just return all of it
						strncpy(loglineBuf, bufPtr, STRBUFSIZ);
					}
				}
				else
				{
					// Expect no level (happens on Xcode iOS Simulator)
					numItems = sscanf(bufPtr, "%[^ ] %[^[][%*d]: %[^\n]", hostnameBuf, processNameBuf, loglineBuf);
					if (numItems != 3)
					{
						// Can't parse line, just return all of it
						strncpy(loglineBuf, bufPtr, STRBUFSIZ);
					}
				}

				// output the line if its timestamp is later than startTime and it's for our process
				if (mktime(&tm) > startTime && strstr(processNameBuf, procNameStr) != NULL)
				{
					printline = 1;
				}
			}

			if (printline)
			{
				fputs(prefixStr, stdout);
				fputs(loglineBuf, stdout);
				fputc('\n', stdout);
			}
		}

		// we want line buffering
		fflush(stdout);
	}
	
	return 0;
}
