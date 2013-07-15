/*
 * httpserver.h
 *
 *  Created on: 15 Jun 2013
 *      Author: mike
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
		_contentType("text/plain"),
		_isPersistent(false) {}
	virtual ~HttpRequestHandler() {}
	static HttpRequestHandler *factory();

	/*!
	 * \brief HTTP request handler
	 * \param method		HTTP method (verb) e.g. GET, POST, PUT, DELETE
	 * \param wildcards		Vector containing URL parts matched by wildcards
	 * \param requestData	Any data sent with the request (e.g. for POST)
	 * \param status		Current status, should be 200 but may have
	 * 						already been set to some error condition.  Really
	 * 						only of use to error handlers.
	 * \return				New status
	 */
	virtual unsigned short handleRequest(const string &method, const vector<string> &wildcards,
				const vector<char> &requestData, unsigned short status) {
		return MHD_HTTP_METHOD_NOT_ALLOWED;
	}

	const bool isPersistent() const { return _isPersistent; }
	const vector<char>& response() const { return _data; }
	const map<string, string>& responseHeaders() const { return _responseHeaders; }
	const string& contentType() const { return _contentType; }
	const string& location() const { return _location; }

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
	map<string, string> _requestHeaders;
	map<string, string> _requestArgs;
	map<string, string> _requestCookies;

	vector<char> _data;
	map<string, string> _responseHeaders;
	string _contentType;
	bool _isPersistent;
	string _location;

	/* For libmicrohttpd running in threaded mode this must block if
	 * no data is available */
	virtual ssize_t contentReader(uint64_t pos, char *buf, size_t max) {
		return -1; // error
	}
};
typedef HttpRequestHandler* (*HttpRequestHandlerFactory)(void);

class HttpErrorHandler : public HttpRequestHandler {
public:
	HttpErrorHandler() : HttpRequestHandler() {}
	static HttpRequestHandler *factory();

	unsigned short handleRequest(const string &method, const vector<string> &wildcards,
			const vector<char> &requestData, unsigned short status);
};

class HttpUrlTree;
class HttpUrlTree {
public:
	HttpUrlTree() : _handler(NULL) {}
	~HttpUrlTree() {}

	HttpRequestHandlerFactory handler() { return _handler; }
	void setHandler(const HttpRequestHandlerFactory handler) { _handler = handler; }

	map<string, HttpUrlTree> next;
private:
	HttpRequestHandlerFactory _handler;
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
	void registerHandler(const string &pattern, HttpRequestHandlerFactory factory);

private:
	HttpRequestHandlerFactory findHandler(const string &path, vector<string> &wildcards);

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
