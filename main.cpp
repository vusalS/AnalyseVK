/**
* Author: Vusal Salmanov (vusal_centr@mail.ru) 
*/

#include "v_curl.h"
#include "v_parser.h"

char command[MAX_SIZE_COMMAND];
char userPath[MAX_PATH];
struct user* userCurrent;
vector<user*> allUsers;
vector<user*> allChats;
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main(void)
{
	setlocale(LC_ALL, "rus");

	printfc(10, "Read cookies ... ");
	read_cookies();
	printfc(10, "OK!\n\n");

	print_users(allUsers);

	do {
		printfc(12, ">> ");
		cin_command();
	} while (!isNumber(command));

	// Bad style :((
	int n = atoi(command);
	int cur = (n < allUsers.size()) ? n : allUsers.size() - 1;
	userCurrent = allUsers[cur];

	mmkdir("CookieFiles");
	_snprintf(userPath, MAX_PATH, "CookieFiles/%s - %s - %s",
		userCurrent->name, userCurrent->id, userCurrent->remixsid);
	mmkdir(userPath);

	if (DOWNLOAD_DOC)
	{
		printfc(10, "Downloading Docs ... ");
		download_doc(userCurrent->remixsid);
		printfc(10, "OK!\n\n");
	}

	if (DOWNLOAD_CHAT)
	{
		compute_content_chat(userCurrent->remixsid);
	}

	return 0;

}
