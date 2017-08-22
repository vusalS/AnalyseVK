#include "v_parser.h"
#include "v_curl.h"

extern char command[MAX_SIZE_COMMAND];
extern char userPath[MAX_PATH];
extern vector<user*> allChats;
extern vector<user*> allUsers;
extern HANDLE hConsole;

/**
* Download content from dialog
*/
void compute_content_chat(const char* remixsid)
{
	char		 chatPath[MAX_PATH] = { 0 };
	char		 chatFile[MAX_PATH] = { 0 };
	char*		 patternDialog;
	struct user* currentChat;
	queue<int>	 chatsForDwnl;

	// Загружаем список чатов пользователя.
	printfc(10, "Downloading Dialog List ... ");
	get_dialog_list(remixsid);
	printfc(10, "OK!\n\n");
	print_users(allChats);
	printfc(12, ">> [%d - all] ", allChats.size());

	// Выбираем нужные чаты.
	do {
		cin_command();
	} while (!isNumber(command));

	if (atoi(command) == allChats.size())
		for (int i = 0; i < allChats.size(); i++)
			chatsForDwnl.push(i);
	else
		chatsForDwnl = split(command);

	// Читаем PATTERNDIALOG_FILE_PATH
	patternDialog = get_pattern_dialog();

	while (!chatsForDwnl.empty())
	{
		// Поочередно берем каждого выбранного пользователя
		// из списка диалогов и качаем содержимое чата.
		int n = chatsForDwnl.front();
		chatsForDwnl.pop();
		if (n >= allChats.size())
			continue;
		currentChat = allChats.at(n);

		// Создаем дирректорию чата
		_snprintf(chatPath, MAX_PATH, "%s/%s - %s",
			userPath, currentChat->name, currentChat->id);
		mmkdir(chatPath);

		// Создаем файл, куда будем выводить диалог
		_snprintf(chatFile, MAX_PATH, "%s/%s - %s.html",
			chatPath, currentChat->name, currentChat->id);

		clearscr();
		currentChat->download = "Downloading...\0";
		print_users(allChats);

		// Загружаем чаты
		fstream fs(chatFile, ios::out);
		char* dialogHtml = download_chat(remixsid, currentChat->id);
		fs << patternDialog;
		fs << dialogHtml;
		fs.flush();
		fs.close();

		// Выгружаем чаты на сервер 
		if (UPLOAD_CHAT) {
			//upload_file_sftp(SERVER_ADDRESS, SERVER_USERNAME, SERVER_PRIVATE_KEYFILE, chatFile);
			upload_file_ftp(SERVER_ADDRESS, SERVER_USERNAME, SERVER_USERPWD, chatFile);
		}

		// Загружаем изображения с чатов
		if (DOWNLOAD_PHOTO_FROM_CHAT)
			download_photo(dialogHtml, chatPath);
		
		free(dialogHtml);

		clearscr();
		currentChat->download = "OK!\0";
		print_users(allChats);
	}

	free(patternDialog);
}

/**
* Download Content Chat (messages).
*/
char* download_chat(const char* remixsid, const char* id)
{
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char		formdata[MAX_LENGTH_FORMDATA] = { 0 };
	char*		buf;
	char*		result;
	int			offset = 0;
	int			whole = 0;
	const int	sizeCL = 10;
	char		respLen[sizeCL];
	struct str	resp;

	do {
		struct str rbuf;

		_snprintf(url, MAX_LENGTH_URL, "https://vk.com/al_im.php");
		_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
		_snprintf(formdata, MAX_LENGTH_FORMDATA, "act=a_history&al=1&gid=0&"
			"offset=%d&peer=%s&rev=0&whole=%d", offset, id, whole);

		send_request(url, &rbuf, cookie, formdata);

		buf = rbuf.ptr;
		buf = strstr(buf, "Content-Length: ") + strlen("Content-Length: ");

		// Bad style :((
		memcpy_s(respLen, sizeCL, buf, sizeCL);
		if (atoi(respLen) <= 400)
			break;

		// Удалить лишние комментарии из html-ответа.
		// Если это не сделать, то в результате браузер
		// не выведет в вкладке содержимое чата. 
		remove_comment(&rbuf);

		// Добавить подгруженные данные к уже загруженной
		// части диалога
		join_chat(&resp, &rbuf);

		// При первом запросу загружается 30 сообщений.
		// Далее величину запроса следует увеличивать на 200,
		// а значение whole=1. Сообщения буду подгружаться.
		if (offset == 0) {
			offset = 30;
			whole = 1;
		}
		else
			offset += 200;
	} while (1);

	result = (char*)calloc(resp.len + 1, sizeof(char));
	memcpy(result, resp.ptr, resp.len);

	return result;
}

/**
* Download Photo from Chat.
* Return: number success downloaded photo.
*/
int download_photo(const char* dialogHtml, const char* path)
{
	char url[MAX_LENGTH_URL] = { 0 };
	char outFile[MAX_PATH] = { 0 };
	const char* buf;
	int n = 0;
	size_t lenFile;

	_snprintf(outFile, MAX_PATH, "%s/Photo/", path);
	mmkdir(outFile);
	lenFile = strlen(outFile);

	buf = dialogHtml;
	while ((buf = strstr(buf, "showPhoto")) != 0)
	{
		int i = 0, j = 0;
		char* linkByFullPhoto;
		_snprintf(outFile, MAX_PATH, "%s/Photo/", path);

		buf = strstr(buf, "event");
		while (*(buf + 1) != '&')
			buf--;
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

		if (download_file(url, outFile, NULL) == CURLE_OK)
		{
			if (UPLOAD_PHOTO_FROM_CHAT)
				upload_file_ftp(SERVER_ADDRESS, SERVER_USERNAME, SERVER_USERPWD, outFile);
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
* Download User's Documents.
* Return number success downloaded document.
*/
int download_doc(const char* remixsid)
{
	char       outFile[MAX_PATH] = { 0 };
	char       url[MAX_LENGTH_URL] = { 0 };
	char	   cookie[MAX_LENGTH_COOKIE] = { 0 };
	char*      buf;
	struct str resp;
	int		   n = 0;
	size_t	   lenFile, lenHeadUrl;

	_snprintf(outFile, MAX_PATH, "%s/Docs/", userPath);
	mmkdir(outFile);

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/docs");
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
	send_request(url, &resp, cookie, NULL);

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com");
	lenFile = strlen(outFile);
	lenHeadUrl = strlen(url);

	buf = resp.ptr;
	while ((buf = strstr(buf, "<a class=\"docs_item_name\" href=\"")) != NULL)
	{
		char		urlByDoc[MAX_LENGTH_URL] = { 0 };
		char*		bufByDoc;
		struct	str respByDoc;
		int			i;

		buf += strlen("<a class=\"docs_item_name\" href=\"");
		i = 0;
		while (buf[i] != '\"')
			i++;
		memcpy_s(url + lenHeadUrl, MAX_LENGTH_URL, buf, i);

		send_request(url, &respByDoc, cookie, NULL);

		bufByDoc = strstr(respByDoc.ptr, "var src = \'");
		if (bufByDoc != NULL)
		{
			bufByDoc += strlen("var src = \'");
			i = 0;
			while (bufByDoc[i] != '\'')
				i++;
			memcpy_s(urlByDoc, MAX_LENGTH_URL, bufByDoc, i);


			buf = strstr(buf, "title=\"");
			buf += strlen("title=\"");
			i = 0;
			while (buf[i] != '\"')
				i++;
			memcpy_s(outFile + lenFile, MAX_PATH, buf, i);

			if (download_file(urlByDoc, outFile, remixsid) == CURLE_OK)
			{
				if (UPLOAD_DOC)
					upload_file_ftp(SERVER_ADDRESS, SERVER_USERNAME, SERVER_USERPWD, outFile);
				n++;
			}
		}

		// Готовим url и ourFile для повторного использования
		memset(url + lenHeadUrl, 0, MAX_LENGTH_URL - lenHeadUrl);
		memset(outFile + lenFile, 0, MAX_PATH - lenFile);
	}

	return n;
}

/**
* Add information about exist chats.
* Return number downloaded chats.
*/
int get_dialog_list(const char* remixsid)
{
	char		url[MAX_LENGTH_URL] = { 0 };
	char		cookie[MAX_LENGTH_COOKIE] = { 0 };
	char		formdata[MAX_LENGTH_FORMDATA] = { 0 };
	struct str* resp;
	char*		buf;
	int			offset = 0;
	const int	sizeCL = 10;
	char		contLen[sizeCL];
	int			n = 0;

	do {
		resp = new struct str();

		_snprintf(url, MAX_LENGTH_URL, "https://vk.com/al_im.php");
		_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
		//_snprintf(formdata, MAX_LENGTH_FORMDATA, "act=a_get_dialogs&al=1&offset=%d&gid=0", offset);
		_snprintf(formdata, MAX_LENGTH_FORMDATA, "act=a_get_dialogs&al=1&gid=0&offset=%d&tab=all", offset);
		send_request(url, resp, cookie, formdata);

		buf = resp->ptr;
		buf = strstr(buf, "Content-Length: ") + strlen("Content-Length: ");

		// Bad style :((.
		// "Content-Length: " < 400 means that 
		// all information about exist chats download already
		memcpy(contLen, buf, sizeCL);
		if (atoi(contLen) <= 400)
			break;

		// Ищем части кода, в которых содержится информация о беседах
		while (buf = strstr(buf, "\"tab\":\""))
		{
			struct user* userNo = new user();
			char* lastMsg;
			char* id;
			char* name;
			int	  i;

			// NAME 
			// Далее ищем строчку: <td class="dialogs_info">.
			// Потом движемся к концу этого класса, до строчки: </a></div>
			// До этой строчки указано имя. Шагаем назад до символа '>'
			// и читаем название диалога.
			// ПОСЛЕ ОБНОВЛЕНИЯ VK - "\"tab\":\""
			buf += strlen("\"tab\":\"");
			i = 0;
			while (buf[i] != '\"')
				i++;
			i = min(MAX_LENGTH_USERNAME, i);
			name = (char*)calloc(i + 1, sizeof(char));
			memcpy_s(name, MAX_LENGTH_USERNAME, buf, i);

			// ID
			// В html-ответе ищем строчку: id="im_dialog..."
			// где вместо точек указан ID.
			// ПОСЛЕ ОБНОВЛЕНИЯ VK - "\"peerId\":"
			buf = strstr(buf, "\"peerId\":");
			buf += strlen("\"peerId\":");
			i = 0;
			while (buf[i] != ',')
				i++;
			id = (char*)calloc(i + 1, sizeof(char));
			memcpy_s(id, MAX_LENGTH_USERID, buf, i);

			// TIME LAST MESSAGE
			// В том же классе (<td class="dialogs_info">)
			// содержится информация о последнем сообщении
			// Переходим к строчке: class="dialogs_date" 
			// и читаем значение.
			// ПОСЛЕ ОБНОВЛЕНИЯ VK - нет возможости узнать
			/*
			buf = strstr(buf, "dialogs_date\">");
			buf += strlen("dialogs_date\">");
			i = 0;
			while (buf[i] != '<')
				i++;
			lastMsg = (char*)calloc(i + 1, sizeof(char));
			memcpy_s(lastMsg, MAX_LENGTH_LAST_MESSAGE, buf, i);
			*/
			userNo->id = id;
			userNo->name = name;
			userNo->timeLastMessage = "NON";//lastMsg;
			userNo->remixsid = "NON";
			userNo->download = "NO";

			allChats.insert(allChats.end(), userNo);
			n++;
		}

		// Готовим resp для следующего запроса
		resp->~str();

		// При первом запросе получаем 40 диалогов.
		// Далее подгружаются по 20 диалогов
		offset == 0 ? offset = 80 : offset += 40;
	} while (1); // Continue until "Content-Length: " >= 400

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
	struct str  resp;


	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/id%s", id);
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
	send_request(url, &resp, cookie, NULL);

	buf = resp.ptr;
	if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0)
		return VK_RESPONSE_NOT_FOUND;

	// В начале html-ответа после строки <title>
	// указано имя пользователя. Его мы читаем до тех пор,
	// пока на символы тега '</title>'.
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
	struct str resp;

	int		   n = 0;

	_snprintf(url, MAX_LENGTH_URL, "https://vk.com/im");
	_snprintf(cookie, MAX_LENGTH_COOKIE, "remixsid=%s", remixsid);
	send_request(url, &resp, cookie, NULL);

	buf = resp.ptr;
	if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0)
		return VK_RESPONSE_NOT_FOUND;

	// В начале html-ответа после строки <title>
	// указано имя пользователя. Его мы читаем до тех пор,
	// пока на символы ','.
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

	file.open(PATTERNDIALOG_FILE_PATH);
	if (!file.is_open()) {
		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
		return "PatternDialog Not Found\n";
	}

	file.seekg(0, file.end);
	lenght = (int)file.tellg();
	file.seekg(0, file.beg);

	pattern = (char*)calloc(lenght + 1, sizeof(char));
	file.read(pattern, lenght);

	file.close();

	return pattern;
}

/**
* Read content.
* Find cookie.
* Authorization "https://vk.com" with cookie
* Add information about user in vector<> allUsers;
*/
void read_cookies()
{
	char*		   p;
	char*          rmx;
	char*          buf;
	int	           length;
	struct user*   userNo;
	ifstream       file;
	unordered_set<string> listRmx;

	file.open(IN_FILE_PATH);
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
}

/**
* Src's content append to the top dst's content
*/
void join_chat(struct str *dst, struct str *src)
{
	size_t new_size = dst->len + src->len;
	dst->ptr = (char*)realloc(dst->ptr, new_size + 1);
	memcpy_s(dst->ptr + src->len, new_size, dst->ptr, dst->len);
	memcpy_s(dst->ptr, new_size, src->ptr, src->len);
	dst->ptr[new_size] = '\0';
	dst->len = new_size;
}

/**
* Rerplace the "<!-- -<>->" and "<!><!json>"
* by empty string (spaces).
*/
void remove_comment(struct str *str)
{
	char* garbage = "                    ";
	char* buf;
	char* p;
	char* newPtr;
	size_t oldSize, newSize;

	oldSize = str->len;
	//p = strstr(str->ptr, "<table class");
	p = str->ptr;

	/*buf = p;
	while ((buf = strstr(buf, "<!-- -<>->")) != 0)
		memcpy_s(buf, oldSize, garbage, strlen("<!-- -<>->"));*/
	buf = p;
	while ((buf = strstr(buf, "<!><!json>")) != 0)
		memcpy_s(buf, oldSize, garbage, strlen("<!><!json>"));

	newSize = strlen(p);
	newPtr = (char*)calloc(newSize, sizeof(char));
	memcpy_s(newPtr, newSize, p, newSize);

	free(str->ptr);

	str->ptr = newPtr;
	str->len = newSize;
}

/**
* If string is empty - return false.
* If string content nonnumber symbol - return false.
* Else - return true.
*/
bool isNumber(const char* text)
{
	int size = strlen(text);

	if (size == 0) {
		return 0;
	}

	for (int i = 0; i < size; i++) {
		char c = text[i];
		if (c == 32 || (c >= 48 && c <= 57)) // symbol is 0, 1, ..., 9
			continue;
		else
			return 0;
	}
	return 1;
}

/**
* If string content nonnumber symbol - return empty queue.
* Else - return queue with divided number.
*/
queue<int> split(const char* text)
{
	queue<int> intsplit;
	const size_t n = 4;

	char number[n] = { 0 };
	for (int i = 0; text[i] != NULL; i++)
	{
		if (text[i] >= 48 && text[i] <= 57)
			number[i] = text[i];
		else if (text[i] == ' ') {
			intsplit.push(atoi(number));
			memset(number, '\0', n);
			text += i + 1;
			i = -1;
		}
	}
	intsplit.push(atoi(number));

	return intsplit;
}

/**
*
*/
void cin_command()
{
	gets_s(command, MAX_SIZE_COMMAND);
	//fflush(stdin);
}

/**
*
*/
void print_users(vector<user*> vector)
{
	for (int i = 0; i < vector.size(); i++) {
		user* userNo = vector.at(i);
		printfc(15, "%d. ", i);
		printfc(14, "%s", userNo->name);
		printfc(15, " - ");
		printfc(5, "%s", userNo->id);
		printfc(15, " - ");
		printfc(8, "%s ", userNo->timeLastMessage);
		printfc(2, "%s\n", userNo->download);
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
