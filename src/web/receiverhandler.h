/*
 * receiverhandler.h
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
 */

#ifndef RECEIVERHANDLER_H_
#define RECEIVERHANDLER_H_

#include <vector>
#include <string>
#include <json/json.h>

#include "httpserver.h"
#include "radio.h"

class ReceiverHandler : public HttpRequestHandler
{
public:
	ReceiverHandler();
	~ReceiverHandler();
	static HttpRequestHandler *factory() {
		return new ReceiverHandler();
	}

	const string allows(const vector<string> &wildcards);

	unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPut(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doPost(const vector<string> &wildcards, const vector<char> &requestData);
	unsigned short doDelete(const vector<string> &wildcards, const vector<char> &requestData);
private:
	Json::Value buildReceiverInfo(Receiver *rx);
	unsigned short parseReceiverInfo(Receiver *rx, Json::Value &root);
};

#endif /* RECEIVERHANDLER_H_ */
