/*
 * filehandler.h
 *
 *  Created on: 20 Jun 2013
 *      Author: mike
 */

#ifndef FILEHANDLER_H_
#define FILEHANDLER_H_

#include <string>
#include <vector>
#include <map>

#include "httpserver.h"

class FileHandler : public HttpRequestHandler
{
public:
	FileHandler();
	~FileHandler() {}
	static HttpRequestHandler *factory();

	unsigned short handleRequest(const string &method, const string &path,
			const vector<char> &requestData, unsigned short status);
private:
	map<string, string> mimeTypes;
};

#endif /* FILEHANDLER_H_ */
