#include "v_curl.h"
#include "v_parser.h"

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

/**
* CURLOPT_WRITEFUNCTION for Download File from server.
*/
size_t write_callback_file(void *ptr, size_t size, size_t nmemb, char *file)
{
	size_t written = fwrite(ptr, size, nmemb, (FILE*)file);
	return written;
}

/**
* CURLOPT_READFUNCTION for Upload File from server.
*/
size_t read_sent_file(void *ptr, size_t size, size_t nmemb, void *stream)
{
	FILE *f = (FILE*)stream;
	size_t n;

	if (ferror(f))
		return CURL_READFUNC_ABORT;

	n = fread(ptr, size, nmemb, f) * size;

	return n;
}

/**
* Initialization Https connection.
*/
CURL* curlInitHttps()
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

/**
* Send POST request
*/
CURLcode send_request(char* url, struct str* resp, char* cookie, char* data)
{
	CURL *curl = curlInitHttps();
	CURLcode rv;

	if (data == NULL)
		data = "";

	curl_easy_setopt(curl, CURLOPT_HEADER, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 6.1; WOW64)");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
	if (cookie != NULL)
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_request);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, resp);
		rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();

	return rv;
}

/**
* Download Data: photo, video, document etc.
* Return status download
*/
CURLcode download_file(const char* url, const char* path, const char* cookie)
{
	FILE* file = fopen(path, "wb");
	CURL *curl = curlInitHttps();
	CURLcode rv;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
	if (cookie != NULL)
		curl_easy_setopt(curl, CURLOPT_COOKIE, cookie);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	fclose(file);

	return rv;
}

/**
* Upload Data (photo, video, document etc) to server with SFTP protocol.
* Return status upload.
*/
CURLcode upload_file_sftp(const char* address, const char* username, const char* private_keyfile, const char* path)
{
	char url[MAX_LENGTH_URL] = { 0 };
	CURL* curl = curl_easy_init();
	CURLcode rv;
	FILE *file;

	file = fopen(path, "rb");
	if (!file) {
		perror(NULL);
		return CURLE_FILE_COULDNT_READ_FILE;
	}

	_snprintf(url, MAX_LENGTH_URL, "sftp://%s/%s", address, path);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_USERNAME, username);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PUBLICKEY);
	curl_easy_setopt(curl, CURLOPT_SSH_PRIVATE_KEYFILE, private_keyfile);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_sent_file);
	curl_easy_setopt(curl, CURLOPT_READDATA, file);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	fclose(file);

	return rv;
}

/**
* Upload Data (photo, video, document etc) to server with FTP protocol.
* Return status upload.
*/
CURLcode upload_file_ftp(const char* address, const char* username, const char* userpwd, const char* path)
{
	char* user_namepwd;
	char url[MAX_LENGTH_URL] = { 0 };
	CURL* curl = curl_easy_init();
	CURLcode rv;
	FILE *file;

	file = fopen(path, "rb");
	if (!file) {
		perror(NULL);
		return CURLE_FILE_COULDNT_READ_FILE;
	}

	size_t length = strlen(username) + strlen(userpwd) + 2;
	user_namepwd = (char*)calloc(length, sizeof(char));

	_snprintf(url, MAX_LENGTH_URL, "ftp://%s/%s", address, path);
	_snprintf(user_namepwd, length, "%s:%s", username, userpwd);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	curl_easy_setopt(curl, CURLOPT_USERPWD, user_namepwd);
	curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);
	curl_easy_setopt(curl, CURLOPT_SSH_AUTH_TYPES, CURLSSH_AUTH_PASSWORD);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_sent_file);
	curl_easy_setopt(curl, CURLOPT_READDATA, file);
	rv = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	curl_global_cleanup();
	fclose(file);

	return rv;
}