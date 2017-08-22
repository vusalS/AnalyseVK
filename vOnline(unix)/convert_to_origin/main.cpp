#include <stdio.h>
#include <fstream>
#include <string>
#pragma warning(disable:4996)

using namespace std;

#define fname "input"

int main()
{
	string buf;
	ifstream ifs;
	ofstream ofs_t, ofs_s;
	int found;

	ifs.open(fname);
	if (!ifs.is_open()) {
		fprintf(stderr, "fopen failed: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}

	getline(ifs, buf);
	found = buf.find_first_of("|");
	buf.erase(found);
	ofs_t.open(buf + "_time", ofstream::out);
	ofs_s.open(buf + "_stat", ofstream::out);
	getline(ifs, buf);
	while (!ifs.eof())
	{
		found = buf.find_first_of(" ");
		ofs_t.write(buf.c_str(), found); ofs_t << '\n';
		ofs_s.write(buf.c_str() + found + 1, 1); ofs_s << '\n';
		getline(ifs, buf);
	}
	ofs_t.close();
	ofs_s.close();
	return 0;
}