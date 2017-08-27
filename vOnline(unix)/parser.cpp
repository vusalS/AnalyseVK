#include "parser.h"
#include "curl.h"


void read_user(user& u)
{
	string str;
	ifstream file;

	file.open(SOURCE_PATH);
	if (!file.is_open()) {
		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	getline(file, str);
	u.set_id(str);
	u.set_name("NONE");

}

string get_name(string id)
{
	char		url[MAX_LENGTH_URL] = { 0 };
	char*		buf;
	string		name;
	struct str	resp;

	snprintf(url, MAX_LENGTH_URL, "http://vk.com/id%s", id.c_str());
	send_request(url, &resp, NULL, "");

	buf = resp.ptr;
	if (memcmp(buf, VK_RESPONSE_OK, strlen(VK_RESPONSE_OK)) != 0)
		return VK_RESPONSE_NOT_FOUND;

	buf = strstr(buf, "<title>") + strlen("<title>");
	for (int n = 0; buf[n] != '|' && n < MAX_LENGTH_USERNAME; n++)
		name.push_back(buf[n]);

	return name;
}

bool get_status(const string& id, string& text, time_t& time_reply)
{
	char		url[MAX_LENGTH_URL] = { 0 };
	char*		buf = NULL;
	struct str	resp;

	snprintf(url, MAX_LENGTH_URL, "http://vk.com/id%s", id.c_str());
	send_request(url, &resp, NULL, "");

	time(&time_reply);
	buf = strstr(resp.ptr, "<div id=\"profile_time_lv\">");


	if (buf == NULL) {
		text = string("");
		return true;
	}
	else {
		buf += strlen("<div id=\"profile_time_lv\">");
		text = string(buf, 30);
		return false;
	}
}

int analyse(user& u)
{
	int n = 0;
	bool s_flaq = false, isOnline = false;
	time_t time_reply;
	string reply;

	u.set_name(get_name(u.id));
	while (n < u.act_size) {
		if (n != 0)
			usleep(PAUSE_MS);

		isOnline = get_status(u.id, reply, time_reply);
		memcpy(&u.act[n].rawtime, &time_reply, sizeof(time_reply));
		if (!isOnline && s_flaq) {
			if (reply.find("минут") != string::npos)
			{
				int found, val;
				found = reply.find_first_of("0123456789");
				reply.erase(0, found);
				found = reply.find_first_not_of("0123456789");
				reply.erase(found);
				val = atoi(reply.c_str());
				for (int i = n; i > n - val && i >= 0; i--) // && i >= 0 
					u.act[i].status = false;
			}
			else u.act[n].status = false;
			s_flaq = false;
		}
		else if (!isOnline && !s_flaq)
		{
			u.act[n].status = false;
			s_flaq = false;
		}
		else
		{
			u.act[n].status = true;
			s_flaq = true;
		}

		n++;
	}

	return 1;
}

void print(user& u)
{
	char		fname[MAX_PATH] = { 0 };
	FILE*		file;
	struct tm*	time;

	snprintf(fname, MAX_PATH, "%s/%s", RESULT_PATH, u.name.c_str());
	file = fopen(fname, "wb");
	fprintf(file, "%s | %s\n", u.name.c_str(), u.id.c_str());
	for (int n = 0; n < u.act_size; n++) {
		time = localtime(&u.act[n].rawtime);
		fprintf(file, "%02i:%02i:%02i %02i.%02i.%02i ", time->tm_hour, time->tm_min, time->tm_sec, 
									time->tm_mday, time->tm_mon+1, time->tm_year+1900);
		if (u.act[n].status) fprintf(file, "1");
		else fprintf(file, "0");
		fprintf(file, "\n");
	}
	fclose(file);
}
