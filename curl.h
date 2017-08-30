#ifndef __VCURL__
#define __VCURL__

#include <curl/curl.h>
#include <stdio.h>
#include "parser.h"

#pragma comment(lib, "curl/libcurl.a")
#pragma comment(lib, "curl/libcurldll.a")
#pragma comment(lib, "libeay32.a")
#pragma comment(lib, "ssleay32.a")

CURL* InitHttps();
CURL* InitHttp();
size_t write_callback_request(void*, size_t, size_t, longstring*);
size_t write_callback_file(void*, size_t, size_t, char*);
size_t read_callback(void *ptr, size_t size, size_t nmemb, const longstring *s);
CURLcode send_request(const char*, const char*, const char*, longstring*, longstring*);
CURLcode send_request_safety(const char*, const char*, const char*, longstring*, longstring*);
CURLcode send_request(const char*, const char*, const char*, longstring*);
CURLcode send_request_safety(const char*, const char*, const char*, longstring*);
CURLcode download_file(const char*, const char*, longstring*);
CURLcode download_file_safety(const char*, const char*, longstring*);
CURLcode upload_file_sftp(const char*, const char*, const char*, const longstring*);
CURLcode upload_file_ftp(const char*, const char*, const char*, const longstring*);

#endif