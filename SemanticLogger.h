#ifndef SEMANTICLOGGER_H
#define SEMANTICLOGGER_H 

#include "picojson.h"

typedef struct request_rec request_rec;

class SemanticLogRoute;

class SemanticLogger
{
	public:
		enum SemanticLoggerLevel {TRACE, INFO, PROFILE, WARNING, ERROR, CRITICAL, DEBUG};

	private:
		SemanticLogger();

		// log englries
		long _requestTime;
		const char* _serverIp;
		const char* _clientIp;
		const char* _method;
		std::string _requestUri;

		// log saving object
		picojson::object _entry;
		picojson::object _access;
		picojson::array _applicationLog;

		// 初期化のルート保存
		enum LogType {WEB, TOOL};
		LogType _mode;

		bool _innerApplicationLog(picojson::object &log, int eventId, const char* message, SemanticLoggerLevel level, const char* category);
		const char* _getLogLevelAsString(SemanticLoggerLevel level);

		std::vector<SemanticLogRoute*> _routes;
	
	protected:

	public:
		// apacheから使う場合
		SemanticLogger(request_rec *r);
		// ツールから使う場合
		SemanticLogger(long requestTime, const char* serverIp);
		~SemanticLogger();
		bool flush();
		// アプリケーションログ記録
		bool applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category, std::map<std::string, std::string> items);
		bool applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category, const char* key, const char* json);
		bool applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category);

		// Router
		bool addRoute(SemanticLogRoute* route);

		// debug
		bool debug(int eventId, const char* message, SemanticLoggerLevel level, const char* category, std::map<std::string, std::string> items)
		{
			if(getenv("DEBUG")==NULL)
				return true;
			return this->applicationLog(eventId, message, DEBUG, category, items);
		}
		bool debug(int eventId, const char* message, SemanticLoggerLevel level, const char* category, const char* key, const char* json)
		{
			if(getenv("DEBUG")==NULL)
				return true;
			return this->applicationLog(eventId, message, DEBUG, category, key, json);
		}
		bool debug(int eventId, const char* message, SemanticLoggerLevel level, const char* category)
		{
			if(getenv("DEBUG")==NULL)
				return true;
			return this->applicationLog(eventId, message, DEBUG, category);
		}
};

#endif
