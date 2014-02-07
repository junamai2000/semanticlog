/**
 * JSONを使ってアプリケーションログとアクセスログを一緒に出すようにするクラス.
 * <pre>
 * まだいろいろ改善が必要
 * </pre>
 * @author Junya Namai
 * @version 0.1
 */
#include <apache2/httpd.h>
#include <apr-1.0/apr_strings.h>

#include "SemanticLogger.h"
#include "SemanticLogRoute.h"

/**
 * ap pool cleanupからlogをフラッシュ出来るようにする
 * @param prt JsonLogクラスのポインター ap_poolのcleanupから呼ばれる 
 */
static apr_status_t CleanUpSemanticLogger(void* ptr) { delete (SemanticLogger*)ptr; }

/**
 * コンストラクタ request_recがある時
 * @param r apache request構造体
 * @param path ログの保存先
 */
SemanticLogger::SemanticLogger(request_rec *r)
{
	// class内に保存
	this->_requestTime = apr_time_sec(r->request_time);
	this->_serverIp = r->connection->local_ip;
	this->_clientIp = r->connection->remote_ip;
	this->_method = r->method;	
	this->_requestUri = r->uri;
	this->_requestUri += (r->args) ?
			apr_pstrcat(r->pool, "?", r->args, NULL) : "";
	
	// ログ用
	this->_access["requestTime"] = picojson::value((double)this->_requestTime);
	this->_access["serverIp"] = picojson::value(this->_serverIp);
	this->_access["clientIp"] = picojson::value(this->_clientIp);
	this->_access["method"] = picojson::value(this->_method);
	this->_access["uri"] = picojson::value(this->_requestUri);
	
	this->_mode = WEB;

	// request終了と当時に廃棄
	apr_pool_cleanup_register(r->pool, (void*)this, CleanUpSemanticLogger, apr_pool_cleanup_null);
}

/**
 * コンストラクタ request_recがない場合
 * @param requestTime ログの時刻 
 * @param serverIp ログを書き出したサーバー
 * @param clientIp アクセスしたクライアント(might be empty?)
 * @param path ログの保存先
 */
SemanticLogger::SemanticLogger(long requestTime, const char* serverIp)
{
	// class内に保存
	this->_requestUri = "";
	this->_method = NULL;
	this->_clientIp = NULL;
	this->_requestTime = requestTime;
	this->_serverIp = serverIp;
	
	// ログ request_recがない時はaccessは使わない
	//this->_applicationLog["requestTime"] = picojson::value((double)this->_requestTime);
	//this->_applicationLog["serverIp"] = picojson::value(this->_serverIp);
	
	this->_mode = TOOL;
}

/**
 * デストラクタでたまったデータを吐き出す
 */
SemanticLogger::~SemanticLogger()
{
	this->flush();
	std::vector<SemanticLogRoute*>::iterator it = _routes.begin();
	while  (it!=_routes.end())
	{
		delete *it;
		++it;
	}
}

/**
 * ログの書き出し
 */
bool SemanticLogger::flush()
{
	if (this->_mode==WEB)
	{
		this->_access["application"] = picojson::value(this->_applicationLog);
		this->_entry["access"] = picojson::value(this->_access);
	}
	else
		this->_entry["application"] = picojson::value(this->_applicationLog);

	std::vector<SemanticLogRoute*>::iterator it = _routes.begin();
	while  (it!=_routes.end())
	{
		(*it)->processLogs(picojson::value(this->_entry).serialize().c_str());
		++it;
	}

	// 初期化
	this->_applicationLog.clear();
	this->_access.clear();
	this->_entry.clear();
	return true;
}

/**
 * アプリケーションログに必須なデータの登録
 * 内部用、公開用の関数は絶対これを呼ぶ
 * @param log アプリケーションログエントリー（一つ分） 
 * @param eventId イベントコード
 * @param message ログメッセージ
 * @param level ログレベル
 * @param category カテゴリ文字列
 */
bool SemanticLogger::_innerApplicationLog(picojson::object &log, int eventId, const char* message, SemanticLoggerLevel level, const char* category)
{
	log["eventId"] = picojson::value((double)eventId);
	log["message"] = picojson::value(message);
	log["category"] = picojson::value(category);
	log["level"] = picojson::value(this->_getLogLevelAsString(level));
}

/**
 * アプリケーションログに必須なデータの登録
 * @param eventId イベントコード
 * @param message ログメッセージ
 * @param level ログレベル
 * @param category カテゴリ文字列
 */
bool SemanticLogger::applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category)
{
	picojson::object app;
	this->_innerApplicationLog(app, eventId, message, level, category);
	this->_applicationLog.push_back(picojson::value(app));
	return true;
}

/**
 * アプリケーションログに必須なデータの登録
 * @param eventId イベントコード
 * @param message ログメッセージ
 * @param level ログレベル
 * @param category カテゴリ文字列
 * @param items アプリケーションログに追加する情報
 */
bool SemanticLogger::applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category, std::map<std::string, std::string> items)
{
	picojson::object app;
	this->_innerApplicationLog(app, eventId, message, level, category);
	std::map<std::string, std::string>::iterator p;
	for(p=items.begin(); p!=items.end(); p++)
		app[p->first] = picojson::value(p->second);
	this->_applicationLog.push_back(picojson::value(app));
	return true;
}

/**
 * アプリケーションログに必須なデータの登録
 * @param eventId イベントコード
 * @param message ログメッセージ
 * @param level ログレベル
 * @param category カテゴリ文字列
 * @param key Jsonを追加するキーエントリ名
 * @param json Json文字列（シンタックスは正しくないとだめ）
 */
bool SemanticLogger::applicationLog(int eventId, const char* message, SemanticLoggerLevel level, const char* category, const char* key, const char* json)
{
	picojson::object app;
	this->_innerApplicationLog(app, eventId, message, level, category);

	std::string err;
	picojson::value v;
	picojson::parse(v, json, json+strlen(json), &err);
	if (err.empty())
	{
		app[key] = v; 
		this->_applicationLog.push_back(picojson::value(app));
		return true;
	}
	else
	{
		app[key] = picojson::value(err);
		this->_applicationLog.push_back(picojson::value(app));
		return false;
	}
}

/**
 * ログルート追加
 */
bool SemanticLogger::addRoute(SemanticLogRoute* route)
{
	if (!route)
		return false;

	route->setRequestTime(this->_requestTime);
	this->_routes.push_back(route);
	return true;
}

/**
 * Logleveのenumを文字列に変換する
 * @param level JsonLogLevel(enum)
 */
const char* SemanticLogger::_getLogLevelAsString(SemanticLoggerLevel level)
{
	switch(level)
	{
		case TRACE:
			return "trace";
		case INFO:
			return "info";
		case PROFILE:
			return "profile";
		case WARNING:
			return "warning";
		case ERROR:
			return "error";
		case CRITICAL:
			return "critical";
		case DEBUG:
			return "debug";
		default:
			return "unknown";
	}
}
