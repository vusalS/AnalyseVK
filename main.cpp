/**
* Author: Vusal Salmanov (vusal_centr@mail.ru)
*/

#include "curl.h"
#include "parser.h"
#include "localdef.h"

using namespace std;

char* server_address = NULL;
char* server_username = NULL;
char* server_userpwd = NULL;
char* server_private_keyfile = NULL;
char* path_to_save = NULL;

bool download_chats = false;
bool download_all_chats = false;
bool download_documents = false;
bool download_photos_from_chat = false;
bool save_chats_to_disk = false;
bool save_document_to_disk = false;
bool save_photos_to_disk = false;
bool upload_documents_to_server = false;
bool upload_chats_to_server = false;
bool upload_photos_to_server = false;
bool use_sftp = true;

extern char *optarg;
extern int optind, opterr, optopt;

struct user tar_user;
vector<user> chats;
HANDLE hConsole;


#include <iostream>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"


void set_options(int argc, char** argv)
{
	// Перечень опций
	// -q		-- download chats
	// -w		-- download all chats
	// -e		-- download private documents
	// -r		-- download photos from chat
	// -t		-- set remixsid
	// -a		-- save_chats_to_disk 
	// -s		-- save_document_to_disk
	// -d		-- save_photos_to_disk
	// -f path	-- path to save
	// -z		-- upload_documents_to_server
	// -x		-- upload_chats_to_server
	// -c		-- upload_photos_to_server
	// -v path	-- server_address
	// -b path	-- server_username
	// -n path	-- server_userpwd
	// -m path	-- server_private_keyfile

	const char *optstring = "qwert:asdf:zxcv:b:n:m:";
	int opt = getopt(argc, argv, optstring);
	while (opt != -1) {
		switch (opt) {
		case 'q':
			download_chats = true;
			break;

		case 'w':
			download_chats = true;
			download_all_chats = true;
			break;

		case 'e':
			download_documents = true;
			break;

		case 'r':
			download_photos_from_chat = true;
			break;

		case 't':
			tar_user.remixsid = optarg;
			break;

		case 'a':
			download_chats = true;
			save_chats_to_disk = true;
			break;

		case 's':
			save_document_to_disk = true;
			download_documents = true;
			break;

		case 'd':
			save_photos_to_disk = true;
			download_photos_from_chat = true;
			break;

		case 'f':
			path_to_save = optarg;
			break;

		case 'z':
			upload_documents_to_server = true;
			download_documents = true;
			break;

		case 'x':
			upload_chats_to_server = true;
			download_chats = true;
			break;

		case 'c':
			upload_photos_to_server = true;
			download_photos_from_chat = true;
			break;

		case 'v':
			server_address = optarg;
			break;

		case 'b':
			server_username = optarg;
			break;

		case 'n':
			server_userpwd = optarg;
			break;

		case 'm':
			server_private_keyfile = optarg;
			break;

		default:
			break;
		}

		opt = getopt(argc, argv, optstring);
	}

	if (server_address == NULL || server_username == NULL || server_userpwd == NULL)
	{
		upload_documents_to_server = false;
		upload_chats_to_server = false;
		upload_photos_to_server = false;
	}

	if (server_private_keyfile == NULL)
		use_sftp = false;

	if (path_to_save == NULL)
	{
		save_chats_to_disk = false;
		save_document_to_disk = false;
		save_photos_to_disk = false;
	}

};

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	set_options(argc, argv);

	if (tar_user.remixsid != NULL) {
		tar_user.id = get_id(tar_user.remixsid);
		tar_user.name = get_name(tar_user.id, tar_user.remixsid);
	}
	else return 1;



	if (download_documents)
		download_doc();

	if (download_chats)
		compute_content_chat();

	return 0;

}
