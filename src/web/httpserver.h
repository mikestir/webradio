/*
 * WebRadio web-based Software Defined Radio
 *
 * Copyright (C) 2013 Mike Stirling
 *
 * This file is part of WebRadio (http://www.mike-stirling.com/webradio)
 *
 * All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <vector>
#include <string>
#include <map>

#include "debug.h"

extern "C" {
	#include <microhttpd.h>
}

using namespace std;

#define HTTP_DEFAULT_PORT			8080
#define HTTP_DEFAULT_CONTENT_TYPE	"text/plain"
#define HTTP_SERVER_NAME			"WebRadio (Linux)"
#define HTTP_CONNECTION_LIMIT		100
#define HTTP_CONNECTION_TIMEOUT		10

class HttpRequestHandler;
class HttpRequestHandler {
public:
	HttpRequestHandler() :
		_arg(NULL),
		_contentType("text/plain"),
		_isPersistent(false) {}
	virtual ~HttpRequestHandler() {}

	static HttpRequestHandler *factory() {
		return new HttpRequestHandler();
	}

	/*!
	 * \brief List of allowable methods for this handler, returned in Allows: header
	 */
	virtual const string allows(const vector<string> &wildcards) { return ""; }

	/*!
	 * \brief HTTP GET handler
	 * \param wildcards		Vector containing URL parts matched by wildcards
	 * \param requestData	Any data sent with the request (e.g. for POST)
	 * \return				HTTP status code
	 */
	virtual unsigned short doGet(const vector<string> &wildcards, const vector<char> &requestData) { return MHD_HTTP_METHOD_NOT_ALLOWED; }

	/*!
	 * \brief HTTP PUT handler
	 * \param wildcards		Vector containing URL parts matched by wildcards
	 * \param requestData	Any data sent with the request (e.g. for POST)
	 * \return				HTTP status code
	 */
	virtual unsigned short doPut(const vector<string> &wildcards, const vector<char> &requestData) { return MHD_HTTP_METHOD_NOT_ALLOWED; }

	/*!
	 * \brief HTTP POST handler
	 * \param wildcards		Vector containing URL parts matched by wildcards
	 * \param requestData	Any data sent with the request (e.g. for POST)
	 * \return				HTTP status code
	 */
	virtual unsigned short doPost(const vector<string> &wildcards, const vector<char> &requestData) { return MHD_HTTP_METHOD_NOT_ALLOWED; }

	/*!
	 * \brief HTTP DELETE handler
	 * \param wildcards		Vector containing URL parts matched by wildcards
	 * \param requestData	Any data sent with the request (e.g. for POST)
	 * \return				HTTP status code
	 */
	virtual unsigned short doDelete(const vector<string> &wildcards, const vector<char> &requestData) { return MHD_HTTP_METHOD_NOT_ALLOWED; }

	/*!
	 * \brief Handler called to generate error page in the event of a failure status code (>=300)
	 */
	void doError(unsigned short status);

	void* arg() { return _arg; }
	void setArg(void *arg) { _arg = arg; }

	const map<string, string>& requestHeaders() const { return _requestHeaders; }
	const map<string, string>& requestArgs() const { return _requestArgs; }
	const map<string, string>& requestCookies() const { return _requestCookies; }
	const map<string, string>& responseHeaders() const { return _responseHeaders; }
	const vector<char>& response() const { return _data; }
	const string& contentType() const { return _contentType; }
	const string& location() const { return _location; }
	const bool isPersistent() const { return _isPersistent; }

	/* Callback for populating header and get argument maps prior to
	 * calling handleRequest */
	static int populate_args(void *self, enum MHD_ValueKind kind,
		const char *key, const char *value);
	/* Static callbacks for access to persistent (streaming) handlers */
	static ssize_t contentReaderCallback(void *self, uint64_t pos, char *buf, size_t max) {
		return ((HttpRequestHandler*)self)->contentReader(pos, buf, max);
	}
	static void freeContentReaderCallback(void *self) {
		delete (HttpRequestHandler*)self;
		LOG_DEBUG("persistent handler freed\n");
	}

protected:
	void *_arg;

	map<string, string> _requestHeaders;
	map<string, string> _requestArgs;
	map<string, string> _requestCookies;

	map<string, string> _responseHeaders;
	vector<char> _data;
	string _contentType;
	string _location;
	bool _isPersistent;

	/* For libmicrohttpd running in threaded mode this must block if
	 * no data is available */
	virtual ssize_t contentReader(uint64_t pos, char *buf, size_t max) {
		return -1; // error
	}
};
typedef HttpRequestHandler* (*HttpRequestHandlerFactory)(void);

class HttpUrlTree;
class HttpUrlTree {
public:
	HttpUrlTree() : _factory(NULL), _arg(NULL) {}
	~HttpUrlTree() {}

	HttpRequestHandlerFactory factory() { return _factory; }
	void* arg() { return _arg; }
	void setHandler(const HttpRequestHandlerFactory factory,
			void *arg = NULL) { _factory = factory; _arg = arg; }

	map<string, HttpUrlTree> next;
private:
	HttpRequestHandlerFactory _factory;
	void *_arg;
};

class HttpServer {
public:
	HttpServer(const int _port = HTTP_DEFAULT_PORT);
	~HttpServer();

	bool start();
	/* \param pattern	URL pattern to match for this handler
	 *					Leading slash must be omitted.
	 * 					Two wildcards are recognised:  A single asterisk will
	 * 					match one path component, and a double asterisk will
	 * 					match the remainder of the URL.
	 */
	void registerHandler(const string &pattern, HttpRequestHandlerFactory factory,
			void *arg = NULL);

private:
	HttpRequestHandler* findHandler(const string &path, vector<string> &wildcards);

	static int handlerCallback(void *arg,
		struct MHD_Connection *conn,
		const char *path,
		const char *method,
		const char *version,
		const char *upload_data,
		size_t *upload_data_size,
		void **ptr);

	int port;
	struct MHD_Daemon *d;
	HttpUrlTree handlerTree;
};

#endif /* HTTPSERVER_H_ */
