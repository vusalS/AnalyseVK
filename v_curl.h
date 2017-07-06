#ifndef __VCURL__
#define __VCURL__

#include <openssl/ssl.h>
#include <curl/curl.h>
#include <stdio.h>

#pragma comment(lib, "curl/libcurl.a")
#pragma comment(lib, "curl/libcurldll.a")
#pragma comment(lib, "libeay32.a")
#pragma comment(lib, "ssleay32.a")

size_t write_callback_request(void*, size_t, size_t, struct str*);
size_t write_callback_file(void*, size_t, size_t, char*);
size_t read_sent_file(void*, size_t, size_t, void*);
CURL* curlInitHttps();
CURLcode send_request(char*, struct str*, char*, char*);
CURLcode download_file(const char*, const char*, const char*);
CURLcode upload_file_sftp(const char*, const char*, const char*, const char*);
CURLcode upload_file_ftp(const char*, const char*, const char*, const char*);

#endif