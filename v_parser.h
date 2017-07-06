#ifndef __VPARSER__
#define __VPARSER__

#include <stdio.h>
#include <fstream>
#include <string>
#include <unordered_set>
#include <direct.h>
#include <unordered_map>
#include <queue>

#pragma warning(disable:4996)

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define DOWNLOAD_DOC					0
#define DOWNLOAD_CHAT					1
#define DOWNLOAD_PHOTO_FROM_CHAT		0

#define UPLOAD_DOC						0
#define UPLOAD_CHAT						0
#define UPLOAD_PHOTO_FROM_CHAT			0

#define MAX_SIZE_COMMAND				128
#define MAX_LENGTH_URL					256
#define MAX_LENGTH_USERID				16
#define MAX_LENGTH_USERNAME				32
#define MAX_LENGTH_REMIXSID				53
#define MAX_LENGTH_FORMDATA				128
#define MAX_LENGTH_LAST_MESSAGE			16
#define MAX_LENGTH_COOKIE				128

#define REMIXSID_LENGTH					53
#define PATTERNDIALOG_FILE_PATH			"PatternDialog/Pattern.htm"
#define IN_FILE_PATH					"Cookie.cck"

#define SERVER_ADDRESS					"31.220.20.246" 
#define SERVER_USERNAME					"u772574054"
#define SERVER_USERPWD					"123321qweewq"
#define SERVER_PRIVATE_KEYFILE			"privkey.pem"

#define VK_RESPONSE_NOT_FOUND			"HTTP/1.1 302 Found"
#define VK_RESPONSE_OK					"HTTP/1.1 200 OK"

#ifdef _WIN32
#define clearscr() system("cls");
#define mmkdir(text) mkdir(text);
#endif

#ifdef LINUX
#define clearscr() system("clear");
#define mmkdir(text, mode) mkdir(text, mode);
#endif

using namespace std;

struct user
{
	char* remixsid;
	char* name;
	char* id;
	char* download;
	char* timeLastMessage;
};

struct str
{
	char *ptr;
	size_t len = 0;

	str()
	{
		ptr = (char*)malloc(1);
		len = 0;
		ptr[0] = '\0';
	}

	~str()
	{
			free(ptr);
			len = 0;
	}
};

void compute_content_chat(const char*);
char* download_chat(const char*, const char*);
int download_photo(const char*, const char*);
int download_doc(const char*);

char* get_id(const char*);
char* get_name(const char*, const char*);
int get_dialog_list(const char*);
char* get_pattern_dialog(void);

void read_cookies(void);

void join_chat(struct str*, struct str*);
void remove_comment(struct str*);
bool isNumber(const char*);
queue<int> split(const char*);

void cin_command();
void print_users(vector<user*>);
void printfc(int, const char*, ...);

#endif