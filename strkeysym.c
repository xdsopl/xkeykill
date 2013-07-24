
/*
xkeykill - kill started application with key combination
Written in 2013 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <string.h>
#include "strkeysym.h"

struct keysym {
	char *str;
	uint32_t sym;
};

static struct keysym keysymdef[] = {
#include "keysymdef.h"
};

static int keysymdef_size = sizeof(keysymdef) / sizeof(struct keysym);

uint32_t strkeysym(char *str)
{
	for (int i = 0; i < keysymdef_size; i++)
		if (!strcmp(str, keysymdef[i].str))
			return keysymdef[i].sym;
	return 0;
}

