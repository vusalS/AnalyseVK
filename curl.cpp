#include "curl.h"
#include "localdef.h"
/**
* Initialization Https connection.
*/
CURL* InitHttps()
{
	CURL *curl = curl_easy_init();
	curl_global_init(CURL_GLOBAL_SSL);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");
	curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);
	return curl;
}

/*
* Initialization Http connection.
*/
CURL* InitHttp()
{
	CURL *curl = curl_easy_init();
	curl_global_init(CURL_GLOBAL_ALL);
	return curl;
}

/**
* CURLOPT_WRITEFUNCTION for recieve a response from server.
*/
size_t write_callback_request(void* ptr, size_t size, size_t nmemb, longstring* s)
{
	s->append(ptr, size, nmemb);
	return size*nmemb;
}

/**
* CURLOPT_WRITEFUNCTION for Download File from server.
*/
size_t write_callback_file(void *ptr, size_t size, size_t nmemb, char *file)
{
	return fwrite(ptr, size, nmemb, (FILE*)file);
}

/**
* CURLOPT_READFUNCTION for Upload File.
*/
size_t read_callback(void *ptr, size_t size, size_t nmemb, const longstring *s)
{
	longstring *upload = (longstring*)s;
	size_t max = size*nmemb;
	size_t sizeleft = s->length;
	if (max < 1)
		return 0;

	if (sizeleft) {
		size_t copylen = max;
		if (copylen > sizeleft)
			copylen = sizeleft;
		memcpy(ptr, upload->ptr, copylen);
		upload->ptr += copylen;
		sizeleft -= copylen;
		return copylen;
	}

	return 0;
}

/**
* Send POST request (HTTP)
*/
CURLcode send_request(const char* url, const char* cookie, const char* data, longstring* header, longstring* body)
{
	CURLcode rv;
	CURL *curl = InitHttp();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Send POST request (HTTPS)
*/
CURLcode send_request_safety(const char* url, const char* cookie, const char* data, longstring* header, longstring* body)
{
	CURLcode rv;
	CURL *curl = InitHttps();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, header);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Send POST request (HTTP)
*/
CURLcode send_request(const char* url, const char* cookie, const char* data, longstring* body)
{
	CURLcode rv;
	CURL *curl = InitHttp();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Send POST request (HTTPS)
*/
CURLcode send_request_safety(const char* url, const char* cookie, const char* data, longstring* body)
{
	CURLcode rv;
	CURL *curl = InitHttps();

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, body);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Download Data: photo, video, document etc. (HTTP)
* Return status download
*/
CURLcode download_file(const char* url, const char* cookie, longstring* data)
{

	CURL* curl = InitHttp();
	CURLcode rv;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	return rv;
}

/**
* Download Data: photo, video, document etc. (HTTPS)
* Return status download
*/
CURLcode download_file_safety(const char* url, const char* cookie, longstring* data)
{
	CURL *curl = InitHttps();
	CURLcode rv;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	return rv;
}

/**
* Upload Data (photo, video, document etc) to server using SFTP protocol.
* Return status upload.
*/
CURLcode upload_file_sftp(const char* address, const char* username, const char* private_keyfile, const longstring* data)
{
	char url[MAX_LENGTH_URL] = { 0 };
	CURL* curl = curl_easy_init();
	CURLcode rv;

	_snprintf(url, MAX_LENGTH_URL, "sftp://%s/Documents/", address);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_USERNAME, username);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PUBLICKEY);
	curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE, private_keyfile);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, data);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Upload Data (photo, video, document etc) to server with FTP protocol.
* Return status upload.
*/
CURLcode upload_file_ftp(const char* address, const char* username, const char* userpwd, const longstring* data)
{
	char name_and_pwd[MAX_LENGTH_USERNAME + MAX_LENGTH_PWD + 2];
	char url[MAX_LENGTH_URL] = { 0 };
	CURL* curl = curl_easy_init();
	CURLcode rv;

	_snprintf(url, MAX_LENGTH_URL, "ftp://%s/Documents/", address);
	_snprintf(name_and_pwd, MAX_LENGTH_USERNAME + MAX_LENGTH_PWD + 2, "%s:%s", username, userpwd);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_USERPWD, name_and_pwd);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &data);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}