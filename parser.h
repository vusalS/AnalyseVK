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
	char* remixsid = NULL;
	char* name = NULL;
	char* id = NULL;
	char* status = NULL;
	char* timeLastMessage = NULL;
};

class longstring
{
private:

public:
	char*	ptr;
	size_t	length;

	longstring()
	{
		ptr = (char*)malloc(1);
		ptr[0] = '\0';
		length = 0;
	}

	void clear()
	{
		free(ptr);
		ptr = (char*)malloc(1);
		ptr[0] = '\0';
		length = 0;
	}

	~longstring()
	{
		free(ptr);
		length = 0;
	}

	char* append(void* str, size_t size, size_t count)
	{
		size_t new_len = length + count;
		ptr = (char*)realloc(ptr, new_len + 1);
		if (ptr == NULL) {
			fprintf(stderr, "realloc() failed\n");
			exit(EXIT_FAILURE);
		}
		memcpy(ptr + length, str, size*count);
		ptr[new_len] = '\0';
		length = new_len;

		return ptr;
	}

	char* append(string str, size_t size, size_t count)
	{
		size_t new_len = length + count;
		ptr = (char*)realloc(ptr, new_len + 1);
		if (ptr == NULL) {
			fprintf(stderr, "realloc() failed\n");
			exit(EXIT_FAILURE);
		}
		memcpy(ptr + length, str.c_str(), size*count);
		ptr[new_len] = '\0';
		length = new_len;

		return ptr;
	}

	char* append_begin(longstring* str, size_t count)
	{
		size_t new_size = length + count;
		char* buf = (char*)malloc(length);
		ptr = (char*)realloc(ptr, new_size + 1);
		if (ptr == NULL) {
			fprintf(stderr, "realloc() failed\n");
			exit(EXIT_FAILURE);
		}
		memcpy_s(buf, length, ptr, length);
		memcpy_s(ptr + count, new_size, buf, length);
		memcpy_s(ptr, new_size, str->ptr, count);
		ptr[new_size] = '\0';
		length = new_size;

		return ptr;
	}

};

void compute_content_chat();
int download_chat(const char*, longstring*);
int download_photo(const char*, const char*);
int download_doc();

char* get_id(const char*);
char* get_name(const char*, const char*);
int get_dialog_list();
char* get_pattern_dialog(void);

void remove_comment(longstring*);
bool isNumber(const char*);
queue<int> split_number(const char*);

void cin_command();
void print_users(vector<user>*);
void printfc(int, const char*, ...);

#endif