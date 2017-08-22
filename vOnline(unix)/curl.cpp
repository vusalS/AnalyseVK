#include "curl.h"
#include "parser.h"

/**
* CURLOPT_WRITEFUNCTION for recieve a response from server.
*/
size_t write_callback_request(void *ptr, size_t size, size_t nmemb, struct str *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = (char*)realloc(s->ptr, new_len + 1);
	if (s->ptr == NULL) {
		fprintf(stderr, "realloc() failed\n");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr + s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}

/*
* Initialization Http connection.
*/
CURL* curlInitHttp()
{
	CURL *curl = curl_easy_init();
	curl_global_init(CURL_GLOBAL_ALL);
	return curl;
}

/**
* Send GET request
*/
CURLcode send_request(char* url, struct str* resp, char* cookie, char* data)
{
	CURLcode rv;
	CURL *curl = curlInitHttp();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}
