#ifndef SEMANTICLOGROUTE_H
#define SEMANTICLOGROUTE_H 

#include <string>
#include <exception>

class SemanticLogRoute 
{
	protected:
		long _requestTime;
		bool _autoLotate;
		std::string _filePath;

	public:
		void setRequestTime(const long time) { this->_requestTime = time; }
		void setLogFile(const char* path) { this->_filePath = path; }
		void setRotate(bool on) { this->_autoLotate = on; }
		virtual bool processLogs(const char* log) = 0;
};

#endif
