#include "sys/printlog.h"

#include <fstream>

#include <conio.h>
#include "tools/stringhelper.h"

#include "graphics/model.h"

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		Model model;

		int len = (int)strlen(argv[i]);
		if ((len > 0) && (argv[i][0] == '-'))
		{
			plog::info("Found flag: ARG[", i, "] '", argv[i], "'\n");
		}
		else
		{
			plog::info("Found filename(?): ARG[", i, "] '", argv[i], "'\n");
			if (len > 5)
			{
				char* sub = argv[i] + (len - 4);
				lowercase(sub);
				if (strncmp(sub, ".xml", 4) == 0)
				{
					plog::info("XML file; will attempt to convert to BIN.\n");
				}
				else if (strncmp(sub, ".bin", 4) == 0)
				{
					plog::info("BIN file; will attempt to convert to XML.\n");
				}
				else
				{
					plog::info("Unknown file type (will attempt to load via FBX SDK).\n");
					model.LoadFBX(argv[i]);

					char outfilename[512];
					snprintf(outfilename, 512, "%s.out.xml", argv[i]);
					std::fstream file;
					file.open(outfilename, std::ios::out | std::ios::trunc);
					model.SaveXML(file);
					file.close();
				}
			}
		}
	}

	plog::info("\n Press any key to continue . . . \n");
	_getch();
}