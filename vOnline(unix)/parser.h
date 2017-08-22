#ifndef __VPARSER_ONLINE__
#define __VPARSER_ONLINE__

#include <stdio.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <memory.h>
#include <ctime>
#include <iostream>
#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#define MAX_SIZE_COMMAND				128
#define MAX_LENGTH_URL					256
#define MAX_LENGTH_USERID				16
#define MAX_LENGTH_USERNAME				32
#define MAX_LENGTH_FORMDATA				128

#define PAUSE_MS					60000000

#define VK_RESPONSE_NOT_FOUND			"HTTP/1.1 404 Not Found"
#define VK_RESPONSE_OK					"HTTP/1.1 200 OK"
#define USER_PATH						"UsersID.txt"
using namespace std;

struct user
{
	struct activity
	{
		time_t rawtime;
		bool status;

		activity() {
			time(&rawtime);
			status = false;
		}
	};

	string id;
	string name;
	activity* act;
	int act_size;

	user(string _id, int _size_act)
	{ 
		id = string(_id);
		act = new activity[_size_act];
		act_size = _size_act;
	}

	user(int _size_act)
	{
		act = new activity[_size_act];
		act_size = _size_act;
	}

	void set_id(string _id) {
		id = string(_id);
	}

	void set_name(string _name) {
		name = string(_name);
	}
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

void read_user(user&);
bool get_status(const string&, time_t&);
string get_name(const char*);
int analyse(user&);
void print(user&);
#endif