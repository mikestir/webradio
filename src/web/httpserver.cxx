/*
 * httpserver.cxx
 *
 *  Created on: 15 Jun 2013
 *      Author: mike
 */

#include <stdio.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>

#include "httpserver.h"
#include "debug.h"

#define DEFAULT_URL_SCHEME		"http://"

using namespace std;

int HttpRequestHandler::populate_args(void *self, enum MHD_ValueKind kind,
	const char *key, const char *value)
{
	switch (kind) {
	case MHD_HEADER_KIND:
		((HttpRequestHandler*)self)->_requestHeaders[key] = value;
		break;
	case MHD_COOKIE_KIND:
		((HttpRequestHandler*)self)->_requestCookies[key] = value;
		break;
	case MHD_GET_ARGUMENT_KIND:
		((HttpRequestHandler*)self)->_requestArgs[key] = value;
		break;
	default:
		return MHD_NO;
	}
	return MHD_YES;
}

/*******************************/

HttpRequestHandler* HttpErrorHandler::factory()
{
	return new HttpErrorHandler();
}

unsigned short HttpErrorHandler::handleRequest(const string &method, const string &path,
		const vector<char> &requestData, unsigned short status)
{
	stringstream ss;

	ss << "<!DOCTYPE html>\n" <<
		"<html>\n" <<
		"<head><title>Error " << status << "</title></head>\n" <<
		"<body>\n" <<
		"<h1>Error " << status << "</h1>\n" <<
		"<p>An error occurred.</p>\n" <<
		"</body>\n" <<
		"</html>\n";

	string response = ss.str();
	_data.insert(_data.end(), response.begin(), response.end());
	_contentType = "text/html";

	return status; // pass through
}

/*******************************/

int HttpServer::handlerCallback(void *arg,
		struct MHD_Connection *conn,
		const char *path,
		const char *method,
		const char *version,
		const char *upload_data,
		size_t *upload_data_size,
		void **ptr)
{
	HttpServer *self = (HttpServer*)arg;
	vector<char> *upload_buffer;
	HttpRequestHandlerFactory handler_factory;
	HttpRequestHandler *handler = NULL;
	struct MHD_Response *response;
	int rc;
	unsigned int status = MHD_HTTP_OK;
	const char *host;
	string handlerPath;

	/* Create vector for upload buffer if required */
	if (*ptr == NULL) {
		*ptr = new vector<char>;
		return MHD_YES;
	}
	upload_buffer = (vector<char>*)*ptr;

	/* Get POST data if present */
	if (*upload_data_size > 0) {
		LOG_DEBUG("adding %lu bytes\n", *upload_data_size);
		/* Append to upload_buffer */
		upload_buffer->insert(upload_buffer->end(),
			upload_data, upload_data + *upload_data_size);
		*upload_data_size = 0;
		return MHD_YES;
	}

	/* Sanity checks - we expect a Host header */
	host = MHD_lookup_connection_value(conn, MHD_HEADER_KIND, "Host");
	if (host == NULL) {
		LOG_ERROR("Missing HTTP Host header\n");
		status = MHD_HTTP_BAD_REQUEST;
		goto error_response;
	}

	/* FIXME: Check Accept header (for GET) - return 406 Not Acceptable,
	 * Check Content-type header (for POST) - return 415 Unsupported Media Type */

	LOG_DEBUG("%s %s\n", method, path);
//	LOG_DEBUG("Request data:\n%.*s\n", upload_buffer->size(), upload_buffer->data());

	handler_factory = self->findHandler(path, handlerPath);
	if (handler_factory) {
		handler = handler_factory();
	} else {
		status = MHD_HTTP_NOT_FOUND;
	}
error_response:
	if (handler == NULL) {
		/* Always use default handler to return error messages */
		handler = HttpErrorHandler::factory();
	}

	/* Populate query parameters for request handler */
	MHD_get_connection_values(conn, MHD_HEADER_KIND,
			HttpRequestHandler::populate_args, handler);
	MHD_get_connection_values(conn, MHD_COOKIE_KIND,
			HttpRequestHandler::populate_args, handler);
	MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND,
			HttpRequestHandler::populate_args, handler);

	/* Call the handler, after which we can get rid of the POST buffer */
	status = handler->handleRequest(string(method), handlerPath, *upload_buffer, status);
	if (upload_buffer)
		delete upload_buffer;
	*ptr = NULL;

	/* Build response */
	LOG_DEBUG("status = %u\n", status);
	if (handler->isPersistent()) {
		/* Arrange for callback */
		response = MHD_create_response_from_callback(
				MHD_SIZE_UNKNOWN, 512,
				&HttpRequestHandler::contentReaderCallback,
				(void*)handler,
				&HttpRequestHandler::freeContentReaderCallback);
	} else {
		/* Send static response */
		response = MHD_create_response_from_buffer(
				handler->response().size(),
				(void*)handler->response().data(),
				MHD_RESPMEM_MUST_COPY); /* FIXME: Can we keep the handler object for a while? */
	}

	/* Handle redirects */
	if (!handler->location().empty()) {
		string redirect_url;

		/* Only add Location: header if the handler returned something */
		LOG_DEBUG("Handler supplied location: %s\n", handler->location().c_str());

		/* Assemble the full URL */
		redirect_url = string(DEFAULT_URL_SCHEME) + string(host) + handler->location();
		LOG_DEBUG("Full URL for redirect: %s\n", redirect_url.c_str());
		MHD_add_response_header(response, MHD_HTTP_HEADER_LOCATION, redirect_url.c_str());
	}

	/* Add any other handler-specific headers */
	map<string, string> extra_headers = handler->responseHeaders();
	map<string, string>::iterator iter;
	for (iter = extra_headers.begin(); iter != extra_headers.end(); iter++) {
		LOG_DEBUG("Extra header %s: %s\n", iter->first.c_str(), iter->second.c_str());
		MHD_add_response_header(response, iter->first.c_str(), iter->second.c_str());
	}

	/* Add generic headers */
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, handler->contentType().c_str());
	MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
	{
		char keepalive[32];
		snprintf(keepalive, 32, "timeout=%d; max=%d", HTTP_CONNECTION_TIMEOUT, HTTP_CONNECTION_LIMIT);
		MHD_add_response_header(response, "Keep-Alive", keepalive);
	}
	MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION, "keep-alive");
	MHD_add_response_header(response, MHD_HTTP_HEADER_SERVER, HTTP_SERVER_NAME);

	/* FIXME: Handlers need to tell us what methods they support for a
	 * particular URL */
#if 0
	/* If a bad method was requested than add the Allow header based on the handlers
	 * defined for the entity */
	if (status == MHD_HTTP_METHOD_NOT_ALLOWED) {
		char allow[32] = {0};
		char *allowptr = allow;

		if (ent->get_handler)
			allowptr += sprintf(allowptr, "GET, ");
		if (ent->put_handler)
			allowptr += sprintf(allowptr, "PUT, ");
		if (ent->post_handler)
			allowptr += sprintf(allowptr, "POST, ");
		if (ent->delete_handler)
			allowptr += sprintf(allowptr, "DELETE, ");
		if (allow[0] == '\0') {
			/* This entity doesn't support anything! */
			CRITICAL("Entity %s has no methods!\n", ent->name);
			status = MHD_HTTP_NOT_FOUND;
		} else {
			/* Remove last ", " */
			allowptr[-2] = '\0';

			DEBUG("Supported methods: %s\n", allow);
			MHD_add_response_header(response, MHD_HTTP_HEADER_ALLOW, allow);
		}
	}
#endif

	if (!handler->isPersistent())
		delete handler;

	/* Send the response */
	rc = MHD_queue_response(conn, status, response);
	MHD_destroy_response(response);
	return rc;
}

HttpServer::HttpServer(const int _port) : port(_port), d(NULL)
{

}

HttpServer::~HttpServer()
{
	if (d) {
		LOG_INFO("Stopping HTTP server\n");
		MHD_stop_daemon(d);
	}
}

bool HttpServer::start()
{
	struct MHD_OptionItem opts[] = {
		{ MHD_OPTION_CONNECTION_LIMIT,		HTTP_CONNECTION_LIMIT,		NULL },
		{ MHD_OPTION_CONNECTION_TIMEOUT,	HTTP_CONNECTION_TIMEOUT,	NULL },
		{ MHD_OPTION_END, 0, NULL }
	};

	d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
		port,
		NULL, /* access control callback */
		NULL, /* argument to above */
 		&handlerCallback, /* default handler */
		this, /* argument to above */
		MHD_OPTION_ARRAY, opts,
		MHD_OPTION_END);
	if (d == NULL) {
		LOG_ERROR("Couldn't start http daemon\n");
		return false;
	}
	LOG_INFO("HTTP interface started on port %hu\n", port);
	return true;
}

void HttpServer::registerHandler(const string &path, HttpRequestHandlerFactory factory)
{
	LOG_DEBUG("Registered handler for path: %s\n", path.c_str());
	handlerMap[path] = factory;
}

HttpRequestHandlerFactory HttpServer::findHandler(const string &path, string &handlerPath)
{
	HttpRequestHandlerFactory factory;
	string pathcopy(path);
	size_t slashpos = path.length();

	/* Attempts to match a handler by stripping layers off the URL.
	 * The most specific handler will match first */
	do {
		pathcopy.resize(slashpos);
		handlerPath = path.substr(slashpos);

		LOG_DEBUG("Trying %s\n", pathcopy.c_str());
		factory = handlerMap[pathcopy];
		if (factory) {
			LOG_DEBUG("Found.  Path = %s\n", handlerPath.c_str());
			break;
		}
		slashpos = pathcopy.find_last_of('/');
	} while (slashpos != string::npos);

	return factory;
}
