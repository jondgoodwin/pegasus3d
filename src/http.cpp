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

CURLM *multi_handle;	//!< Handle for all asynchronous transfers
int nbr_running;		//!< Number of currently in-process transfers
int max_handles;		//!< Max number of easy handles allocated to resourcebuffers
struct ResourceBuffer {
	CURL *easy;
	char *buffer;
	size_t bufsize;
	Value th;
	Value callback;
} *resourcebuffers;

/** Allocate initialized space in resource buffers */
void alloc_buffers(int newmax) {
	resourcebuffers = (struct ResourceBuffer *) realloc(resourcebuffers, newmax*sizeof(struct ResourceBuffer));
	struct ResourceBuffer *resbufp = &resourcebuffers[max_handles];
	for (int i = max_handles; i < newmax; i++) {
		resbufp->easy = NULL;
		resbufp->buffer = NULL;
		resbufp->bufsize = 0;
		resbufp++;
	}
	max_handles = newmax;
}

/** Set up Internet resource access via libcurl */
void resource_init(void) {
	curl_global_init(CURL_GLOBAL_WIN32);	// Use CURL_GLOBAL_DEFAULT when SSL is desired
	multi_handle = curl_multi_init();
	nbr_running = 0;
	resourcebuffers = NULL;
	max_handles = 0;
	alloc_buffers(256);
}

/** Clean up the Internet resource access capability */
// NB Todo: This should also stop all in-process transfers and then
// take easy's out of the multi and clean them up
void resource_close(void) {
	curl_multi_cleanup(multi_handle);
	curl_global_cleanup();
	free(resourcebuffers);
}

void resource_poll() {
	// Return if no active transfers taking place
	if (nbr_running == 0)
		return;

	// Perform any transfer work in process
	curl_multi_perform(multi_handle, &nbr_running);
		
	// Any status messages from running transfers?
	CURLMsg *msg;	// Holder for a transfer status message
	int msgs_left;	// how many messages are left
	while ((msg = curl_multi_info_read(multi_handle, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			// Find the resource buffer that has the easy handle
			struct ResourceBuffer *resbufp = &resourcebuffers[0];
			for (int i=0; i<max_handles && resbufp->easy!=msg->easy_handle; i++)
				resbufp++;

			// Handle failure
			long respcode;
			CURLcode res;
			if ((res=curl_easy_getinfo(resbufp->easy, CURLINFO_RESPONSE_CODE, &respcode)) != CURLE_OK || respcode!=200) {
				// Do callback passing it an error diagnostic
				pushValue(resbufp->th, resbufp->callback);
				pushValue(resbufp->th, aNull);
				pushValue(resbufp->th, aNull);
				if (res!=CURLE_OK)
					pushString(resbufp->th, aNull, curl_easy_strerror(res));
				else {
					char respcodestr[80];
					sprintf(respcodestr, "HTTP response Code %ld", respcode);
					pushString(resbufp->th, aNull, respcodestr);
				}
				getCall(resbufp->th, 3, 0);
			}

			// Call the success method, passing it the stream
			else {
				pushValue(resbufp->th, resbufp->callback);
				pushValue(resbufp->th, aNull);
				pushStringl(resbufp->th, aNull, (char*) resbufp->buffer, resbufp->bufsize);
				getCall(resbufp->th, 2, 0);
			}

			// Clean up
			curl_multi_remove_handle(multi_handle, msg->easy_handle);
			curl_easy_cleanup(msg->easy_handle);
			resbufp->easy = NULL;
		}
	}
}

/** Callback that assembles the retrieved resource into a growing allocated buffer */
size_t http_write_callback(char *ptr, size_t size, size_t nmemb, void *udata) {
	struct ResourceBuffer *resbuf = (struct ResourceBuffer *) udata;
	size_t newsize = size*nmemb;
	resbuf->buffer = (char*) realloc(resbuf->buffer, resbuf->bufsize+newsize+1);
	if (resbuf->buffer == NULL) {
		fprintf(stderr, "Ran out of buffer memory getting resource from the Internet");
		return 0;
	}
	memcpy(&(resbuf->buffer)[resbuf->bufsize], ptr, newsize);
	resbuf->bufsize += newsize;
	resbuf->buffer[resbuf->bufsize] = '\0';
	return newsize;
}

/** 'Get': Get contents for passed http:// url string */
// NB Todo: This captures the current thread (th). Should that thread be stopped,
// all its in-process transfers should be halted first or else bad things will happen.
int http_get(Value th) {
	int nparms = getTop(th);

	// Get string value of filename path
	Value fnval;
	if (getTop(th)<2 || (!isStr(fnval = getLocal(th,1)) && !isSym(fnval))) {
		pushValue(th, aNull);
		return 1;
	}
	const char *url = toStr(fnval);

	// Point user data to an unused resource buffer
	struct ResourceBuffer *udata = &resourcebuffers[0];
	int residx;
	for (residx=0; residx < max_handles && udata->easy!=NULL; residx++)
		udata++;
	if (residx>=max_handles) {
		// double size of buffer table if we ran out
		int oldmax = max_handles;
		alloc_buffers(2*max_handles);
		udata = &resourcebuffers[oldmax];
	}

	// Set up the GET request
 	CURL *easy = curl_easy_init();
    curl_easy_setopt(easy, CURLOPT_URL, url);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) udata);
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, http_write_callback);
#ifdef _DEBUG
    curl_easy_setopt(easy, CURLOPT_VERBOSE, 1L);
#endif

	// Initialize user data area
	udata->easy = easy;
	udata->buffer = NULL;
	udata->bufsize = 0;
	udata->th = th;
	udata->callback = nparms>2? getLocal(th,2) : aNull;

	// Enqueue GET request (poll will do the actual perform)
	curl_multi_add_handle(multi_handle, easy);
	nbr_running++; // Make sure poll knows we have new work to perform/check
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
