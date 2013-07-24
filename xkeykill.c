
/*
xkeykill - kill started application with key combination
Written in 2013 by <Ahmet Inan> <ainan@mathematik.uni-freiburg.de>
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include "strkeysym.h"

static pid_t child;
void sig_handler(int sig)
{
	signal(sig, SIG_IGN);
	exit(kill(-child, sig));
}

uint16_t extract_modifiers(char *str)
{
	static char *mods[3] = { "<Shift>", "<Ctrl>", "<Alt>" };
	static uint16_t masks[3] = { XCB_MOD_MASK_SHIFT, XCB_MOD_MASK_CONTROL, XCB_MOD_MASK_1 };
	uint16_t mask = 0;
	for (int i = 0; i < 3; i++) {
		char *where = strstr(str, mods[i]);
		if (where) {
			strcpy(where, where + strlen(mods[i]));
			mask |= masks[i];
		}
	}
	return mask;
}

xcb_keycode_t get_keycode(xcb_keysym_t keysym)
{
	xcb_connection_t *con = xcb_connect(0, 0);
	xcb_key_symbols_t *symbols = xcb_key_symbols_alloc(con);
	xcb_keycode_t *keycodes = xcb_key_symbols_get_keycode(symbols, keysym);
	xcb_keycode_t keycode = 0;
	if (keycodes)
		keycode = keycodes[0];
	free(keycodes);
	xcb_key_symbols_free(symbols);
	xcb_disconnect(con);
	return keycode;
}

int main(int argc, char **argv)
{
	if (argc < 3) {
		fprintf(stderr, "Usage: %s KEYCOMBO COMMAND [ARG]...\n", argv[0]);
		fprintf(stderr, "Start COMMAND, and kill it when KEYCOMBO is pressed.\n");
		return 1;
	}

	char *keycombo = argv[1];

	uint16_t modifiers = extract_modifiers(keycombo);

	xcb_keysym_t keysym = strkeysym(keycombo);
	if (!keysym) {
		fprintf(stderr, "unknown key or modifier: %s\n", keycombo);
		exit(1);
	}

	xcb_keycode_t keycode = get_keycode(keysym);
	if (!keycode) {
		fprintf(stderr, "couldnt get keycode for %s\n", keycombo);
		exit(1);
	}

	pid_t pid = fork();

	if (pid < 0)
		return 1;

	if (0 == pid) {
		argv += 2;
		setpgid(0, 0);
		execvp(argv[0], argv);
		perror(argv[0]);
		return 1;
	}

	child = pid;
	signal(SIGINT, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGTERM, sig_handler);

	xcb_connection_t *con = xcb_connect(0, 0);
	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(con)).data;
	uint32_t mask = XCB_CW_EVENT_MASK;
	uint32_t values[] = { XCB_EVENT_MASK_KEY_PRESS };
	xcb_change_window_attributes_checked(con, screen->root, mask, values);
	uint16_t ignore_masks[] = { XCB_MOD_MASK_LOCK, XCB_MOD_MASK_2 };
	for (int i = 0; i < (1 << (sizeof(ignore_masks) / 2)); i++) {
		uint16_t ignore = 0;
		for (int j = 0; j < (int)(sizeof(ignore_masks) / 2); j++)
			ignore |= (i & (1 << j) ? ignore_masks[j] : 0);
		xcb_grab_key(con, 1, screen->root, modifiers | ignore, keycode, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	}
	xcb_flush(con);

	while (1) {
		sleep(1);
		if (waitpid(child, 0, WNOHANG))
			return 0;
		xcb_generic_event_t *event;
		while ((event = xcb_poll_for_event(con))) {
			int type = event->response_type & ~0x80;
			if (XCB_KEY_PRESS == type) {
				xcb_key_press_event_t *ke = (xcb_key_press_event_t *)event;
				xcb_keycode_t kc = ke->detail;
				uint16_t km = ke->state & (XCB_MOD_MASK_SHIFT|XCB_MOD_MASK_CONTROL|XCB_MOD_MASK_1);
				if (keycode == kc && modifiers == km)
					return kill(-child, SIGTERM);
			}
			free(event);
		}
	}
	return 0;
}

