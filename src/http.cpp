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

/** Callback structure for resource accumulation */
struct StringState {
	Value th;	//!< Preserve the thread pointer
	Value resource;	//!< The string holding the resource's contents
};

/** Callback from perform that assembles the retrieved resource into a string */
size_t http_write_callback(char *ptr, size_t size, size_t nmemb, void *udata) {
	struct StringState *ss = (struct StringState*)udata;
	if (ss->resource==aNull)
		ss->resource = pushStringl(ss->th, aNull, ptr, size*nmemb);
	else
		strAppend(ss->th, ss->resource, ptr, size*nmemb);
	return size*nmemb;
}

/** '()': Get contents for passed http:// url string */
int http_get(Value th) {
	// Get string value of filename path
	Value fnval;
	if (getTop(th)<2 || (!isStr(fnval = getLocal(th,1)) && !isSym(fnval))) {
		pushValue(th, aNull);
		return 1;
	}
	const char *url = toStr(fnval);

	// Set up the GET request
	struct StringState udata = {th, aNull};
 	CURL *curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) &udata);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_write_callback);
#ifdef _DEBUG
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
#endif

	// Do GET request and get response in callback
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
		vmLog("Failed to load %s. Curl error: %s", url, curl_easy_strerror(res));
 
    curl_easy_cleanup(curl);
	pushValue(th, udata.resource);
	return 1;
}

/** Initialize the Http type */
void http_init(Value th) {
	Value typ = pushType(th, aNull, 1);
		pushSym(th, "Http");
		popProperty(th, 0, "_name");
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
