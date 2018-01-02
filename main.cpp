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
char* pattern_dialog_file = NULL;

bool download_chats				= 1;
bool download_all_chats			= 0;
bool download_documents			= 0;
bool download_photos_from_chat	= 0;
bool save_chats_to_disk			= 1;
bool save_document_to_disk		= 0;
bool save_photos_to_disk		= 0;
bool upload_documents_to_server = 0;
bool upload_chats_to_server		= 0;
bool upload_photos_to_server	= 0;
bool use_sftp					= 0;

extern char *optarg;
extern int optind, opterr, optopt;

struct user tuser;
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
	// -t param	-- set remixsid
	// -y param	-- pattern dialog file
	// -a		-- save_chats_to_disk 
	// -s		-- save_document_to_disk
	// -d		-- save_photos_to_disk
	// -f param	-- path to save
	// -z		-- upload_documents_to_server
	// -x		-- upload_chats_to_server
	// -c		-- upload_photos_to_server
	// -v param	-- server_address
	// -b param	-- server_username
	// -n param	-- server_userpwd
	// -m param	-- server_private_keyfile

	const char *optstring = "qwert:y:asdf:zxcv:b:n:m:";
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
			tuser.remixsid = optarg;
			break;

		case 'y':
			pattern_dialog_file = optarg;
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
		upload_documents_to_server = 0;
		upload_chats_to_server = 0;
		upload_photos_to_server = 0;
	}

	if (server_private_keyfile == NULL)
		use_sftp = 0;

	if (path_to_save == NULL) {
		save_chats_to_disk = 0;
		save_document_to_disk = 0;
		save_photos_to_disk = 0;
	}

};

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "Russian");
	set_options(argc, argv);

	if (tuser.remixsid != NULL) {
		tuser.id = get_id(tuser.remixsid);
		tuser.name = get_name(tuser.id, tuser.remixsid);
	}
	else return 1;
	
	if (download_documents)
		download_doc();

	if (download_chats)
		compute_content_chat();

	return 0;

}
