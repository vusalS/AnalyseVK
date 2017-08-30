#include "parser.h"
#include "curl.h"
#include "localdef.h"

extern vector<user> chats;
extern struct user tar_user;
extern HANDLE hConsole;
extern char* server_address;
extern char* server_username;
extern char* server_userpwd;
extern char* server_private_keyfile;
extern char* path_to_save;
extern char* pattern_dialog_file;
extern bool download_chats;
extern bool download_all_chats;
extern bool download_documents;
extern bool download_photos_from_chat;
extern bool save_chats_to_disk;
extern bool save_document_to_disk;
extern bool save_photos_to_disk;
extern bool upload_documents_to_server;
extern bool upload_chats_to_server;
extern bool upload_photos_to_server;
extern bool use_sftp;

/**
* Download all content from dialog
*/
void compute_content_chat()
{
	const char* rmx = tar_user.remixsid;
	char*		patternDialog = NULL;
	queue<int>	chatsDwnl;
	longstring	data;

	get_dialog_list();
	print_users(&chats);

	if (download_all_chats)
		for (int i = 0; i < chats.size(); i++)
			chatsDwnl.push(i);
	else {
		char command[MAX_SIZE_COMMAND];
		printfc(12, ">> ");
		do {
			gets_s(command, MAX_SIZE_COMMAND);
			fflush(stdin);
		} while (!isNumber(command));
		chatsDwnl = split_number(command);
	}

	if (pattern_dialog_file != NULL)
		patternDialog = get_pattern_dialog();
	while (!chatsDwnl.empty()) {
		int n = chatsDwnl.front(); chatsDwnl.pop();
		if (n >= chats.size()) continue;
		struct user curUser = chats.at(n);

		clearscr();
		curUser.status = "Downloading...\0";
		print_users(&chats);

		// Download Chat
		if (download_chat(curUser.id, &data)) {
			if (save_chats_to_disk)
			{
				char chatPath[MAX_PATH] = { 0 };
				char chatFile[MAX_PATH] = { 0 };
				_snprintf(chatPath, MAX_PATH, "%s/%s - %s",
					path_to_save, curUser.name, curUser.id);
				mmkdir(chatPath);
				_snprintf(chatFile, MAX_PATH, "%s/%s - %s.html",
					chatPath, curUser.name, curUser.id);
				fstream fs(chatFile, ios::out);
				if (patternDialog != NULL) fs << patternDialog;
				fs << data.ptr;
				fs.flush();
				fs.close();
			}
			// Upload chat to the (s)ftp server. 
			if (upload_chats_to_server)
				if (use_sftp)
					upload_file_sftp(server_address, server_username, server_private_keyfile, &data);
				else upload_file_ftp(server_address, server_username, server_userpwd, &data);

				// Download photo from chat. ПОРАБОТАТЬ
				//if (download_photos_from_chat)
				//	download_photo(data.ptr, "111");
		}
		clearscr();
		chats[n].status = "YES\0";
		print_users(&chats);
		data.clear();
	}

	free(patternDialog);
}

/**
* Download Content Chat (messages).
*/
int download_chat(const char* id, longstring* data)
{
	const char* rmx = tar_user.remixsid;
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char		formdata[MAX_LENGTH_FORMDATA] = { 0 };
	char*		buf;
	int			offset = 0;
	int			whole = 0;
	const int	sizeCL = 10;
	char		respLen[sizeCL];

	do {
		longstring body, header;

		_snprintf(url, MAX_LENGTH_URL, "https://vk.com/al_im.php");
		_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", rmx);
		_snprintf(formdata, MAX_LENGTH_FORMDATA, "act=a_history&al=1&gid=0&"
			"offset=%d&peer=%s&rev=0&whole=%d", offset, id, whole);

		if (send_request_safety(url, cookie, formdata, &header, &body) != CURLE_OK)
			return 0;

		buf = header.ptr;
		if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0) {
			fprintf(stderr, "%s", VK_RESPONSE_NOT_FOUND);
			exit(EXIT_FAILURE);
		}

		buf = strstr(buf, "Content-Length: ") + strlen("Content-Length: ");
		memcpy_s(respLen, sizeCL, buf, sizeCL);
		if (atoi(respLen) <= 400)
			break;

		remove_comment(&body);
		data->append_begin(&body, body.length);

		// При первом запросу загружается 30 сообщений.
		// Далее величину запроса следует увеличивать на 200,
		// а значение whole=1. Сообщения буду подгружаться.
		if (offset == 0) {
			offset = 30;
			whole = 1;
		}
		else offset += 100;
	} while (1);

	return 1;
}

/**
* Download Photo from Chat.
* Return number success download photo.
*/
int download_photo(const char* dialogHtml, const char* path)
{
	char url[MAX_LENGTH_URL] = { 0 };
	char outFile[MAX_PATH] = { 0 };
	const char* buf;
	int n = 0;
	size_t lenFile;
	longstring data;

	_snprintf(outFile, MAX_PATH, "%s/Photo/", path);
	lenFile = strlen(outFile);

	buf = dialogHtml;
	while ((buf = strstr(buf, "showPhoto")) != 0)
	{
		int i = 0, j = 0;
		char* linkByFullPhoto;
		_snprintf(outFile, MAX_PATH, "%s/Photo/", path);

		buf = strstr(buf, "event");
		while (*(buf + 1) != '&') buf--;
		for (; *buf != ';'; buf--)
		{
			if (*buf == '/')
			{
				memcpy_s(outFile + lenFile, MAX_PATH, buf + 1, i);
				memcpy_s(outFile + lenFile + i, MAX_PATH, ".jpg\0", 5);
			}
			i++;
		}
		linkByFullPhoto = (char*)calloc(i + 1, sizeof(char));
		memcpy_s(linkByFullPhoto, i + 1, buf + 1, i);

		buf = buf + 1;
		buf = strstr(buf, "https://");
		for (int k = 0; k < 5;) {
			if (buf[j] == '/')
				k++;
			j++;
		}

		memcpy_s(url, MAX_LENGTH_URL, buf, j);
		memcpy_s(url + j, MAX_LENGTH_URL, linkByFullPhoto, i);
		memcpy_s(url + j + i, MAX_LENGTH_URL, ".jpg\0", 5);

		if (download_file(url, " ", &data) == CURLE_OK) {
			if (save_photos_to_disk) {
				mmkdir(outFile);
				fstream fs(outFile, ios::out);
				fs << data.ptr;
				fs.flush();
				fs.close();
			}
			if (upload_photos_to_server)
				upload_file_ftp(server_address, server_username, server_userpwd, &data);
			n++;
		}
		memset(outFile + lenFile, 0, MAX_PATH - lenFile);
	}

	// Иногда возвращает значение, отличающееся от
	// итогового количества созданный файлов.
	// Возможно, мы пытаемся скачать несуществующее фото.
	return n;
}

/**
* Download Documents by User.
* Return number success download document.
*/
int download_doc() {
	const char* rmx = tar_user.remixsid;
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char*		buf;
	int			n = 0;
	size_t		lenPath, lenHeadUrl;
	longstring	resp;

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/docs");
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", rmx);
	if (send_request(url, cookie, " ", &resp) != CURLE_OK)
		return 0;

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com");
	lenHeadUrl = strlen(url);

	buf = resp.ptr;
	while ((buf = strstr(buf, "<a class=\"docs_item_name\" href=\"")) != NULL) {
		char		urlDoc[MAX_LENGTH_URL] = { 0 };
		char*		bufDoc;
		longstring  rspDoc;
		longstring	data;
		int i;

		buf += strlen("<a class=\"docs_item_name\" href=\"");
		i = 0; while (buf[i] != '\"') i++;
		memcpy_s(url + lenHeadUrl, MAX_LENGTH_URL, buf, i);
		send_request(url, cookie, " ", &rspDoc);

		bufDoc = strstr(rspDoc.ptr, "var src = \'");
		if (bufDoc != NULL) {
			bufDoc += strlen("var src = \'");
			i = 0; while (bufDoc[i] != '\'') i++;
			memcpy_s(urlDoc, MAX_LENGTH_URL, bufDoc, i);

			if (download_file(urlDoc, rmx, &data) == CURLE_OK) {
				if (save_document_to_disk) {
					char outFile[MAX_PATH] = { 0 };
					_snprintf(outFile, MAX_PATH, "%s/Documents/", path_to_save);
					mmkdir(outFile);
					buf = strstr(buf, "title=\"");
					buf += strlen("title=\"");
					i = 0; while (buf[i] != '\"') i++;
					memcpy_s(outFile + strlen(outFile), MAX_PATH, buf, i);

					fstream fs(outFile, ios::out);
					fs << data.ptr;
					fs.flush();
					fs.close();
				}

				if (upload_documents_to_server) {
					if (use_sftp) upload_file_sftp(server_address, server_username, server_private_keyfile, &data);
					else upload_file_ftp(server_address, server_username, server_userpwd, &data);
				}
				n++;
			}
		}

		memset(url + lenHeadUrl, 0, MAX_LENGTH_URL - lenHeadUrl);
	}

	return n;
}

/**
* Add information about exist chats (dialogs) by user
* Return number download chats's information.
*/
int get_dialog_list()
{
	const char* rmx = tar_user.remixsid;
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char		formdata[MAX_LENGTH_FORMDATA] = { 0 };
	longstring	body, header;
	char*		buf;
	int			offset = 0;
	const int	sizeCL = 10;
	char		contLen[10];
	int			n = 0;

	do {
		_snprintf(url, MAX_LENGTH_URL, "https://vk.com/al_im.php");
		_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", rmx);
		_snprintf(formdata, MAX_LENGTH_FORMDATA, "act=a_get_dialogs&al=1&gid=0&offset=%d&tab=all", offset);
		send_request_safety(url, cookie, formdata, &header, &body);

		buf = header.ptr;
		if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0) {
			fprintf(stderr, "%s", VK_RESPONSE_NOT_FOUND);
			exit(EXIT_FAILURE);
		}

		buf = strstr(buf, "Content-Length: ") + strlen("Content-Length: ");

		memcpy(contLen, buf, sizeCL);
		if (atoi(contLen) <= 400)
			break;

		buf = body.ptr;
		while (buf = strstr(buf, "\"tab\":\""))
		{
			struct user user;
			int			i;

			// NAME 
			// Движемся к концу строки, до символа: \
																																																																																																															// До этого символа указано имя.
			buf += strlen("\"tab\":\"");
			i = 0; while (buf[i] != '\"') i++;
			i = min(MAX_LENGTH_USERNAME, i);
			user.name = new char[i + 1];
			memcpy_s(user.name, MAX_LENGTH_USERNAME, buf, i);
			user.name[i] = '\0';

			// ID
			// В html-ответе ищем строчку: "peerId":
			// где указан ID.
			buf = strstr(buf, "\"peerId\":");
			buf += strlen("\"peerId\":");
			i = 0; while (buf[i] != ',') i++;
			i = min(MAX_LENGTH_USERID, i);
			user.id = new char[i + 1];
			memcpy_s(user.id, MAX_LENGTH_USERID, buf, i);
			user.id[i] = '\0';

			// TIME LAST MESSAGE
			// В том же классе (<td class="dialogs_info">)
			// содержится информация о последнем сообщении
			// Переходим к строчке: class="dialogs_date" 
			// и читаем значение.
			// ПОСЛЕ ОБНОВЛЕНИЯ VK - ???
			/*
			buf = strstr(buf, "dialogs_date\">");
			buf += strlen("dialogs_date\">");
			i = 0;
			while (buf[i] != '<')
			i++;
			lastMsg = (char*)calloc(i + 1, sizeof(char));
			memcpy_s(lastMsg, MAX_LENGTH_LAST_MESSAGE, buf, i);
			*/

			user.timeLastMessage = "NO";
			user.remixsid = "NO";
			user.status = "NO";

			chats.insert(chats.end(), user);
			n++;
		}
		header.clear();
		body.clear();

		// При первом запросе получаем 40 диалогов.
		// Далее подгружаются по 20 диалогов
		offset == 0 ? offset = 80 : offset += 40;
	} while (1); // Продолжаем пока "Content-Length: " >= 400

	return n;
}

/**
* If user not found - return VK_RESPONSE_NOT_FOUND.
* Else - return user's name.
*/
char* get_name(const char* id, const char* remixsid)
{
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char*		name;
	char*		buf;
	int			n = 0;
	longstring  body, header;


	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/id%s", id);
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
	send_request_safety(url, cookie, " ", &header, &body);

	buf = header.ptr;
	if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0){
		fprintf(stderr, "%s", VK_RESPONSE_NOT_FOUND);
		exit(EXIT_FAILURE);
	}

	buf = body.ptr;
	buf = strstr(buf, "<title>") + strlen("<title>");
	while (buf[n] != '<')
		n++;
	n = min(n, MAX_LENGTH_USERNAME);
	name = (char*)calloc(n + 1, sizeof(char));
	memcpy_s(name, MAX_LENGTH_USERNAME, buf, n);

	return name;
}

/**
* If user not found - return VK_RESPONSE_NOT_FOUND.
* Else - return user's id.
*/
char* get_id(const char* remixsid)
{
	char	   url[MAX_LENGTH_URL] = { 0 };
	char	   cookie[MAX_LENGTH_COOKIE] = { 0 };
	char*	   id;
	char*	   buf;
	longstring body, header;

	int		   n = 0;

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/im");
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
	send_request_safety(url, cookie, " ", &header, &body);

	buf = header.ptr;
	if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0) {
		fprintf(stderr, "%s", VK_RESPONSE_NOT_FOUND);
		exit(EXIT_FAILURE);
	}

	buf = body.ptr;
	buf = strstr(buf, "id: ") + strlen("id: ");
	while (buf[n] != ',')
		n++;
	id = (char*)calloc(n + 1, sizeof(char));
	memcpy_s(id, MAX_LENGTH_USERID, buf, n);

	return id;
}

/**
* Function return pointer by content's
* #define PATTERNDIALOG_FILE_PATH
*/
char* get_pattern_dialog()
{
	ifstream file;
	int lenght;
	char* pattern;

	file.open(pattern_dialog_file);
	if (!file.is_open()) {
		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
		return NULL;
	}
	file.seekg(0, file.end);
	lenght = (int)file.tellg();
	file.seekg(0, file.beg);
	pattern = (char*)calloc(lenght + 1, sizeof(char));
	file.read(pattern, lenght);
	pattern[lenght] = '\n';
	file.close();

	return pattern;
}

/**
* Read content from #define IN_FILE_PATH.
* Find cookie.
* Authorization "https://vk.com" with cookie
* and add information about user to vector<> allUsers;
*/
/*void set_option(int argc, char** argv)
{
char*		   p;
char*          rmx;
char*          buf;
int	           length;
struct user*   userNo;
ifstream       file;
unordered_set<string> listRmx;

file.open(INPUT_COOKIE_FILE);
if (!file.is_open()) {
fprintf(stderr, "fopen failed: %s\n", strerror(errno));
exit(EXIT_FAILURE);
}

file.seekg(0, file.end);
length = (int)file.tellg();
file.seekg(0, file.beg);

buf = (char*)calloc(length + 1, sizeof(char));
file.read(buf, length);
p = buf;

while ((buf = strstr(buf, "remixsid=")) != NULL)
{
buf += strlen("remixsid=");
rmx = (char*)calloc(REMIXSID_LENGTH + 1, sizeof(char));
memcpy_s(rmx, REMIXSID_LENGTH + 1, buf, REMIXSID_LENGTH);

// If rms exists already - continue
if (listRmx.find(rmx) != listRmx.end())
continue;
else
listRmx.emplace(rmx);

// Bad style :((
userNo = new user();
userNo->remixsid = rmx;
userNo->id = get_id(rmx);
if (memcmp(userNo->id, VK_RESPONSE_NOT_FOUND, strlen(VK_RESPONSE_NOT_FOUND)) == 0) {
delete userNo;
continue;
}
userNo->name = get_name(userNo->id, rmx);

allUsers.insert(allUsers.end(), userNo);
}

buf = p;

free(buf);
file.close();
}*/

/**
* Rerplace the "<!-- -<>->" and "<!><!json>"
* by empty string (spaces).
*/
void remove_comment(longstring* str)
{
	// Этот участок требует доработки
	char* buf;

	buf = str->ptr;
	while ((buf = strstr(buf, "<!--")) != 0)
		while (*buf != '\0' && memcmp(buf, "<div", 4) != 0) { *buf = ' '; buf++; }
	
	buf = str->ptr;
	while ((buf = strstr(buf, "<!json>")) != 0)
		while (*buf != '\0' && memcmp(buf, "<div", 4) != 0) { *buf = ' '; buf++; }
}

/**
* If string is empty - return false.
* If string content nonnumber symbol - return false.
* Else - return true.
*/
bool isNumber(const char* text)
{
	int size = strlen(text);
	if (size == 0)
		return false;

	for (int i = 0; i < size; i++) {
		char c = text[i];
		if (c == 32 || (c >= 48 && c <= 57))
			continue;
		else
			return false;
	}
	return true;
}

/**
* If string content nonnumber symbol - return empty queue.
* Else - return queue with divided number.
*/
queue<int> split_number(const char* text)
{
	queue<int> intsplit_number;
	const size_t n = 4;

	char number[n] = { 0 };
	for (int i = 0; text[i] != NULL; i++)
	{
		if (text[i] >= 48 && text[i] <= 57)
			number[i] = text[i];
		else if (text[i] == ' ') {
			intsplit_number.push(atoi(number));
			memset(number, '\0', n);
			text += i + 1;
			i = -1;
		}
	}
	intsplit_number.push(atoi(number));

	return intsplit_number;
}

void print_users(vector<user>* vector)
{
	for (int i = 0; i < vector->size(); i++) {
		user u = vector->at(i);
		printfc(15, "%d. ", i);
		printfc(14, "%s", u.name);
		printfc(15, " - ");
		printfc(5, "%s", u.id);
		printfc(15, " - ");
		printfc(8, "%s ", u.timeLastMessage);
		printfc(2, "%s\n", u.status);
	}
}

#ifdef WIN32
void printfc(int color, const char* text, ...)
{
	char buffer[256];
	va_list args;
	SetConsoleTextAttribute(hConsole, (WORD)color);
	va_start(args, text);
	vsprintf(buffer, text, args);
	printf(buffer);
	va_end(args);
	SetConsoleTextAttribute(hConsole, (WORD)15);
}
#endif

#ifdef LINUX
void printfc(int color = 0, const char* text, ...)
{
	char buffer[256];
	va_list args;
	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	printf(buffer);
	va_end(args);
}
#endif
