#ifndef SEMANTICLOGFILEROUTE_H
#define SEMANTICLOGFILEROUTE_H 

#include "SemanticLogRoute.h"

class SemanticLogFileRoute : public SemanticLogRoute
{
	public:
		virtual bool processLogs(const char* log);
};

#endif
