#include <stdio.h>
#include <sys/file.h>

#include "SemanticLogFileRoute.h"

bool SemanticLogFileRoute::processLogs(const char* log)
{
	FILE* fp;
	char suffix[10];
	struct tm* timeinfo;
	timeinfo = localtime(&this->_requestTime);
	std::string path;
	
	if (this->_autoLotate)
	{
		strftime(suffix, 9, "%Y%m%d", timeinfo);
		path = std::string(this->_filePath) + "." + std::string(suffix);
	}
	else
		path = this->_filePath;

	if (fp = fopen(path.c_str(), "a+")) {
		if (flock(fileno(fp), LOCK_EX) == 0) {
			fprintf(fp, "%s\n", log);
			fclose(fp);
		}
	} else {
		std::string str = log;
		fprintf(stderr, "failed to open json log json = %s\n", str.c_str());
		return false;
	}

	return true;
}
