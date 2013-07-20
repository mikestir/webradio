/*
 * receiverhandler.cxx
 *
 *  Created on: 16 Jul 2013
 *      Author: mike
 */

#include <vector>
#include <string>

#include <json/json.h>

#include "receiverhandler.h"
#include "debug.h"

ReceiverHandler::ReceiverHandler() : HttpRequestHandler()
{

}

ReceiverHandler::~ReceiverHandler()
{

}

const string ReceiverHandler::allows(const vector<string> &wildcards)
{
	if (wildcards.size() == 0)
		return "POST";
	else
		return "GET, PUT, DELETE";
}

unsigned short ReceiverHandler::doGet(const vector<string> &wildcards, const vector<char> &requestData)
{
	Json::Value root;

	if (wildcards.size() == 0) {
		/* Return data for all registered receivers */
		for (map<string, Receiver*>::const_iterator it = Radio::receivers().begin(); it != Radio::receivers().end(); ++it)
			root.append(buildReceiverInfo(it->second));
	} else {
		string uuid = wildcards[0];

		/* Return data for specific front-end */
		if (Radio::receivers().count(uuid) == 0)
			return MHD_HTTP_NOT_FOUND;
		root = buildReceiverInfo(Radio::receivers().at(uuid));
	}

	Json::StyledWriter writer;
	string output = writer.write(root);
	_contentType = "application/json";
	_data.insert(_data.end(), output.begin(), output.end());
	return MHD_HTTP_OK;
}

unsigned short ReceiverHandler::doPut(const vector<string> &wildcards, const vector<char> &requestData)
{
	if (wildcards.size() == 0)
		/* Only allowed for a specific rx */
		return MHD_HTTP_METHOD_NOT_ALLOWED;

	string uuid = wildcards[0];
	if (Radio::receivers().count(uuid) == 0)
		return MHD_HTTP_NOT_FOUND;

	Json::Reader reader;
	Json::Value root;
	bool ok = reader.parse(requestData.data(), root);
	if (!ok)
		return MHD_HTTP_BAD_REQUEST;

	unsigned short status = parseReceiverInfo(Radio::receivers().at(uuid), root);

	_contentType = "application/json";
	return status;
}

unsigned short ReceiverHandler::doPost(const vector<string> &wildcards, const vector<char> &requestData)
{
	/* FIXME: For adding a new receiver - not implemented yet */
	return MHD_HTTP_METHOD_NOT_ALLOWED;
}

unsigned short ReceiverHandler::doDelete(const vector<string> &wildcards, const vector<char> &requestData)
{
	/* FIXME: For deleting a receiver - not implemented yet */
	return MHD_HTTP_METHOD_NOT_ALLOWED;
}

Json::Value ReceiverHandler::buildReceiverInfo(Receiver *rx)
{
	Json::Value root;

	/* FIXME: af_gain, squelch */
	root["uri"] = "/receivers/" + rx->uuid();
	root["tuner"] = "/receivers/" + rx->frontEnd()->uuid();
	root["if_frequency"] = rx->downconverter()->IF();
	root["if_bandwidth"] = rx->channelFilter()->passband();
	root["af_bandwidth"] = rx->audioFilter()->passband();
	root["af_gain"] = 0;
	root["squelch_threshold"] = 0;
	root["demodulator"] = rx->demodulator()->modeString();

	return root;
}

unsigned short ReceiverHandler::parseReceiverInfo(Receiver *rx, Json::Value &root)
{
	/* FIXME: af_gain, squelch, tuner re-assignment */
	/* FIXME: some sort of validation - limits checking should go in the
	 * DspBlocks */
	if (root.isMember("if_frequency"))
		rx->downconverter()->setIF(root["if_frequency"].asInt());
	if (root.isMember("if_bandwidth"))
		rx->channelFilter()->setPassband(root["if_bandwidth"].asUInt());
	if (root.isMember("af_bandwidth"))
		rx->audioFilter()->setPassband(root["af_bandwidth"].asUInt());
	if (root.isMember("demodulator"))
		rx->demodulator()->setModeString(root["demodulator"].asString());

	return MHD_HTTP_NO_CONTENT;
}
