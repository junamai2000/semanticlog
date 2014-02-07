#include "SemanticLogger.h"
#include "SemanticLogFileRoute.h"

int main()
{
	SemanticLogger log(1391178994, "123.123.123.123"); 
	SemanticLogRoute* r = new SemanticLogFileRoute();
	r->setLogFile("./output");
	r->setRotate(true);
	log.addRoute(r);
	log.applicationLog(1, "error\n", SemanticLogger::INFO, "aaa.aa.aaaaa", "test", "{\"ssss\":12344 }");
	log.flush();

	std::map<std::string, std::string> itms;  // 文字×文字のmapを指定
	itms.insert(std::pair<std::string, std::string>("01", "C++"));  // 値を挿入
	itms.insert(std::pair<std::string, std::string>("02", "C#"));
	itms.insert(std::pair<std::string, std::string>("03", "VB"));
	itms.insert(std::pair<std::string, std::string>("04", "Java"));
	itms.insert(std::pair<std::string, std::string>("05", "XML"));
	log.applicationLog(2, "trace\n", SemanticLogger::TRACE, "aaa.aa.aaaaa", itms);
	log.applicationLog(3, "info\n", SemanticLogger::INFO, "aaa.aa.aaaaa");
	return 1;
}
