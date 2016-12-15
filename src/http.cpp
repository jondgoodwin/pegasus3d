/** Scene definition and rendering
 * @file
 *
 * This source file is part of the Pegasus3d browser.
 * See Copyright Notice in pegasus3d.h
*/

#include "pegasus3d.h"
#include <stdio.h>
#include <string.h>

#include "curl/curl.h"

CURLM *multi_handle;
struct ResourceBuffer {
	void *buffer;
	size_t bufsize;
};

// Set up Internet resource access via libcurl
void resource_init() {
	curl_global_init(CURL_GLOBAL_WIN32);	// Use CURL_GLOBAL_DEFAULT when SSL is desired
	multi_handle = curl_multi_init();
}

// Clean up the Internet resource access capability
void resource_close(void) {
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();
}

/** Callback that assembles the retrieved resource into a growing allocated buffer */
size_t http_write_callback(char *ptr, size_t size, size_t nmemb, void *udata) {
	struct ResourceBuffer *resbuf = (struct ResourceBuffer *) udata;
	size_t newsize = size*nmemb;
	if (resbuf->buffer==NULL) {
		resbuf->buffer = malloc(resbuf->bufsize = newsize);
		memcpy(resbuf->buffer, ptr, resbuf->bufsize);
	}
	else {
		resbuf->buffer = realloc(resbuf->buffer, resbuf->bufsize+newsize);
		memcpy(&((char*)resbuf->buffer)[resbuf->bufsize], ptr, newsize);
		resbuf->bufsize += newsize;
	}
	return newsize;
}

/** 'Get': Get contents for passed http:// url string */
int http_get(Value th) {
	int nparms = getTop(th);

	// Get string value of filename path
	Value fnval;
	if (getTop(th)<2 || (!isStr(fnval = getLocal(th,1)) && !isSym(fnval))) {
		pushValue(th, aNull);
		return 1;
	}
	const char *url = toStr(fnval);

	// Set up the GET request
	struct ResourceBuffer udata = {NULL, 0};
 	CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &udata);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_callback);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

	// Do GET request and get response in callback
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
		// Call the failure method, passing it an error diagnostic
		if (nparms>2 && isCallable(getLocal(th,2))) {
			pushLocal(th, 2);
			pushValue(th, aNull);
			pushValue(th, aNull);
			pushString(th, aNull, curl_easy_strerror(res));
			getCall(th, 3, 0);
		}
	    curl_easy_cleanup(curl);
		return 0;
	}
    curl_easy_cleanup(curl);

	// Call the success method, passing it the stream
	if (nparms>2 && isCallable(getLocal(th,2))) {
		pushLocal(th, 2);
		pushValue(th, aNull);
		pushStringl(th, aNull, (char*) udata.buffer, udata.bufsize);
		getCall(th, 2, 0);
	}
	return 0;
}

/** Initialize the Http type */
void http_init(Value th) {
	Value typ = pushType(th, aNull, 1);
		pushSym(th, "Http");
		popProperty(th, 0, "_name");
		pushCMethod(th, http_get);
		popProperty(th, 0, "Get");
	popGloVar(th, "Http");

	// Register this type as Resource's 'http' scheme
	pushGloVar(th, "Resource");
		pushProperty(th, getTop(th) - 1, "schemes");
			pushValue(th, typ);
			popTblSet(th, getTop(th) - 2, "http");
		popValue(th);
	popValue(th);
}
