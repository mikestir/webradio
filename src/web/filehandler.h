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
	~FileHandler();
	static HttpRequestHandler *factory() {
		return new FileHandler();
	}

	const string allows(const vector<string> &wildcards) { return "GET"; }

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
private:
	map<string, string> mimeTypes;
};

#endif /* FILEHANDLER_H_ */
