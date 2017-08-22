/**
* Author: Vusal Salmanov (vusal_centr@mail.ru)
*/

#include "curl.h"
#include "parser.h"

int main(int argc, char** argv)
{
	char id[MAX_LENGTH_USERID] = { 0 };
	int size = 0;

	user User(argv[1], atoi(argv[2]));
	analyse(User);
	print(User);
	return 0;
}

