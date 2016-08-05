/** Scene definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <stdio.h>
#include <string.h>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

/** '()': Get contents for passed filename path string */
int http_get(Value th) {
	// Get string value of filename path
	Value fnval;
	if (getTop(th)<2 || (!isStr(fnval = getLocal(th,1)) && !isSym(fnval))) {
		pushValue(th, aNull);
		return 1;
	}
	const char *fn = toStr(fnval);
	if (0==strncmp(fn, "http://", 7))
		fn+=7;

    boost::asio::io_service io_service;
    boost::system::error_code error;

	// Find host using boost
	tcp::resolver resolver(io_service);
    tcp::resolver::query query(tcp::v4(), "web3d.jondgoodwin.com", "80");
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
	tcp::resolver::iterator end;

	tcp::socket socket(io_service);
	error = boost::asio::error::host_not_found;
	
	// Connect socket to host
	while (error && endpoint_iterator != end) {
	   	socket.close();
	   	socket.connect(*endpoint_iterator++, error);
	}

	if (error) {
	    throw boost::system::system_error(error);
	}

	char request[] =
		"GET /world.acn HTTP/1.0 \r\n"
		"Host: web3d.jondgoodwin.com\r\n"
		"\r\n";

	// Make request
	boost::system::error_code ignored_error;
	boost::asio::write(socket, boost::asio::buffer(request), boost::asio::transfer_all(), ignored_error);

    // Read content
    std::string response = "";
    boost::asio::streambuf response_buf;
	std::stringstream response_stream;
    while(boost::asio::read(socket, response_buf, boost::asio::transfer_at_least(1), error)) {
      	response_stream << &response_buf;
    }

	response = response_stream.str();
	//remove_response_headers(response);
	response = response.substr(response.find("\r\n\r\n") + 4);

	AuintIdx bufsize = response.length();

	// Create the string buffer (which will be returned)
	Value strbuf = pushStringl(th, aNull, response.c_str(), bufsize);

	return 1;
}

/** Initialize the Http type */
void http_init(Value th) {
	Value typ = pushType(th, aNull, 1);
		pushCMethod(th, http_get);
		popProperty(th, 0, "()");
	popGloVar(th, "Http");

	// Register this type as Resource's 'http' scheme
	pushGloVar(th, "Resource");
		pushProperty(th, getTop(th) - 1, "schemes");
			pushValue(th, typ);
			popTblSet(th, getTop(th) - 2, "http");
		popValue(th);
	popValue(th);
}
