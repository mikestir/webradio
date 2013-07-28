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

void HttpRequestHandler::doError(unsigned short status)
{
	stringstream ss;
	string errorstr;

	switch (status) {
	case 400: errorstr = "Bad request"; break;
	case 401: errorstr = "Unauthorized"; break;
	case 403: errorstr = "Forbidden"; break;
	case 404: errorstr = "Not found"; break;
	case 405: errorstr = "Method not allowed"; break;
	case 406: errorstr = "Not acceptable"; break;
	default:
		errorstr = "An error occurred";
	}

	ss << "<!DOCTYPE html>\n" <<
		"<html>\n" <<
		"<head><title>Error " << status << "</title></head>\n" <<
		"<body>\n" <<
		"<h1>Error " << status << "</h1>\n" <<
		"<p>" << errorstr << "</p>\n" <<
		"</body>\n" <<
		"</html>\n";

	string response = ss.str();
	_data.insert(_data.end(), response.begin(), response.end());
	_contentType = "text/html";
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
	HttpRequestHandler *handler = NULL;
	struct MHD_Response *response;
	int rc;
	unsigned short status = MHD_HTTP_OK;
	const char *host;
	vector<char> *upload_buffer;
	vector<string> url_wildcards;

	/* Create vector for upload buffer if required */
	if (*ptr == NULL) {
		*ptr = new vector<char>;
		return MHD_YES;
	}
	upload_buffer = (vector<char>*)*ptr;

	/* Get POST data if present */
	if (*upload_data_size > 0) {
		LOG_DEBUG("adding %u bytes\n", (unsigned int)*upload_data_size);
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
	}

	/* FIXME: Check Accept header (for GET) - return 406 Not Acceptable,
	 * Check Content-type header (for POST) - return 415 Unsupported Media Type */

	LOG_DEBUG("%s %s\n", method, path);
	if (status == MHD_HTTP_OK) {
		/* Look up handler for requested URL */
		handler = self->findHandler(path, url_wildcards);
		if (handler == NULL)
			status = MHD_HTTP_NOT_FOUND;
	}

	if (handler == NULL) {
		/* Use default handler */
		handler = HttpRequestHandler::factory();
	}

	/* Populate query parameters for request handler */
	MHD_get_connection_values(conn, MHD_HEADER_KIND,
			HttpRequestHandler::populate_args, handler);
	MHD_get_connection_values(conn, MHD_COOKIE_KIND,
			HttpRequestHandler::populate_args, handler);
	MHD_get_connection_values(conn, MHD_GET_ARGUMENT_KIND,
			HttpRequestHandler::populate_args, handler);

	/* Don't call the handler if a method already occurred */
	if (status == MHD_HTTP_OK) {
		string strmethod(method);
		if (strmethod == "GET")
			status = handler->doGet(url_wildcards, *upload_buffer);
		else if (strmethod == "PUT")
			status = handler->doPut(url_wildcards, *upload_buffer);
		else if (strmethod == "POST")
			status = handler->doPost(url_wildcards, *upload_buffer);
		else if (strmethod == "DELETE")
			status = handler->doDelete(url_wildcards, *upload_buffer);
		else
			status = MHD_HTTP_METHOD_NOT_ALLOWED;
	}

	if (status >= 300) {
		/* Generate error page instead */
		handler->doError(status);
	}

	/* Build response */
	LOG_DEBUG("status = %u\n", status);
	if (handler->isPersistent()) {
		/* The handler wants to be called back (streaming) */
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

	/* If we return 405 then we have to include an Allows: header */
	if (status == MHD_HTTP_METHOD_NOT_ALLOWED)
		MHD_add_response_header(response, MHD_HTTP_HEADER_ALLOW, handler->allows(url_wildcards).c_str());

	/* Clean up */
	if (!handler->isPersistent())
		delete handler; // Handler can ask to stick around (for streaming), else we delete it
	if (upload_buffer)
		delete upload_buffer;
	*ptr = NULL;

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

void HttpServer::registerHandler(const string &pattern, HttpRequestHandlerFactory factory, void *arg)
{
	istringstream iss(pattern);
	string part;
	HttpUrlTree *node = &handlerTree;

	/* Split path on slashes and build node tree */
	while (std::getline(iss, part, '/'))
		node = &node->next[part];
	node->setHandler(factory, arg);
	LOG_DEBUG("Registered handler for URL: %s\n", pattern.c_str());
}

HttpRequestHandler* HttpServer::findHandler(const string &path, vector<string> &wildcards)
{
	istringstream iss(path);
	string part;
	HttpUrlTree *node = &handlerTree;

	wildcards.clear();

	/* Split requested URL on slashes and walk tree for a match */
	while (std::getline(iss, part, '/')) {
		if (part.empty())
			continue; // skip null path components

		if (node->next.count(part) == 0) {
			if (node->next.count("*") == 0) {
				if (node->next.count("**") == 0) {
					return NULL; // No match
				} else {
					/* Double wildcard matches the rest of the URL.
					 * Stop scanning and return the remainder as the final
					 * wildcard component. */
					string remainder;
					getline(iss, remainder);
					if (remainder.empty())
						/* No further path components after this one - trailing
						 * slash is suppressed */
						wildcards.push_back(part);
					else
						wildcards.push_back(part + "/" + remainder);
					node = &node->next.at("**");
					break;
				}
			} else {
				// Matches on wildcard
				wildcards.push_back(part);
				node = &node->next.at("*");
			}
		} else {
			// Matches on specific pattern
			node = &node->next.at(part);
		}
	}

	HttpRequestHandler *handler = NULL;
	if (node->factory()) {
		/* Instantiate a handler using the factory and set its argument
		 * for those handlers that use it */
		handler = (node->factory())();
		handler->setArg(node->arg());
	}

	return handler;
}
