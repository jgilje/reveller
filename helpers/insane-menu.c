#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "insane-menu.h"

#include "6510.h"
#include "sidheader.h"

#define MENU_LINES 4
#define MENU_BUF_SIZE 1024

typedef struct menu_data_t {
	char top_dir[MENU_BUF_SIZE];
	char cur_dir[MENU_BUF_SIZE];
	char prev_dir[MENU_BUF_SIZE];
	
	int dir_scanned;
	int dir_pointer;
	int dir_entries_len;
	struct dirent **dir_entries;
	
	int state;
	int redraw;
} menu_data_t;
static menu_data_t menu_data;

static int sid_scandir_filter(const struct dirent* d) {
	int len = strlen(d->d_name);
	int z;
	char* pos = strstr(d->d_name, ".sid");
	
	if (d->d_name[0] == '.') {
		return 0;
	}
	
	if (d->d_type == DT_DIR) {
		return 1;
	}
	
	if (pos == NULL) {
		return 0;
	}
	
	z = len - (pos - d->d_name);
	if (z == 4) {
		return 1;
	}
	
	return 0;
}

static void menu_scandir(const char* dir) {
	int n;
	n = scandir(dir, &menu_data.dir_entries, sid_scandir_filter, alphasort);
   if (n < 0) {
       perror("scandir");
   }
   /*
    else {
       int i = 0;
       while (i < n) {
           printf("%s\n", namelist[i]->d_name);
           free(namelist[i]);
           i++;
       }
       free(namelist);
   }
   */
	menu_data.dir_entries_len = n;
	
	if (menu_data.prev_dir[0] != 0) {
		int i = 0;
		while (i < n) {
			if (! strcmp(menu_data.dir_entries[i]->d_name, menu_data.prev_dir)) {
				menu_data.dir_pointer = i;
				break;
			}
			i++;
		}
		
		menu_data.prev_dir[0] = 0;
	}
}

/*
void readdir_demo(void) {
  DIR *dp;
  struct dirent *ep;     
  dp = opendir ("./");

  if (dp != NULL) {
    while (ep = readdir (dp)) {
      puts (ep->d_name);
    }

    (void) closedir (dp);
  } else {
    perror ("Couldn't open the directory");
  }
}
*/

static void menu_dir_reset(void) {
	menu_data.dir_pointer = 0;
	menu_data.dir_scanned = 0;
}
static void menu_main_display(void) {
	if (! menu_data.redraw) {
		return;
	}
	
	char l0[1024];
	char* l[1]; l[0] = l0;
	
	snprintf(l0, 1024, "FX SID");
	lcd_print_lines(l, 1);
	
	menu_data.redraw = 0;
}
static void menu_main_handle(int c) {
	switch (c) {
		case LEFT:
			lcd_init();
			break;
		case RIGHT:
			snprintf(menu_data.cur_dir, MENU_BUF_SIZE, "%s", menu_data.top_dir);
			menu_data.state = DIRECTORY;
			menu_dir_reset();
			break;
		case UP:
			break;
		case DOWN:
			break;
	}
}

static void menu_dir_append_dir(const char* name) {
	char temp[MENU_BUF_SIZE];
	snprintf(temp, MENU_BUF_SIZE, "%s", menu_data.cur_dir);
	snprintf(menu_data.cur_dir, MENU_BUF_SIZE, "%s/%s", temp, name);
	menu_dir_reset();
}

static void menu_dir_prev_dir(void) {
	char* p = strrchr(menu_data.cur_dir, '/');
	snprintf(menu_data.prev_dir, MENU_BUF_SIZE, "%s", &p[1]);
	p[0] = 0;
	menu_dir_reset();
}

static void menu_dir_display(void) {
	if (! menu_data.redraw) {
		return;
	}
	
	if (! menu_data.dir_scanned) {
		if (menu_data.dir_entries != 0) {
			int i;
			
			for (i = 0; i < menu_data.dir_entries_len; i++) {
				free(menu_data.dir_entries[i]);
			}
			free(menu_data.dir_entries);
			menu_data.dir_entries = 0;
		}
	
		menu_scandir(menu_data.cur_dir);
		menu_data.dir_scanned = 1;
	}
	
	int screen_pos = menu_data.dir_pointer % MENU_LINES;
	int screen_page = menu_data.dir_pointer / MENU_LINES;
	int dir_entries_start = screen_page * MENU_LINES;
	int i;
	
	char l0[21], l1[21], l2[21], l3[21];
	char* l[4];	l[0] = l0; l[1] = l1; l[2] = l2; l[3] = l3;
	l0[0] = 0; l1[0] = 0; l2[0] = 0; l3[0] = 0;
	
	// printf("DIR (pos %d, page %d, dir_start %d)\n", screen_pos, screen_page, dir_entries_start);
	for (i = 0; i < MENU_LINES && dir_entries_start+i < menu_data.dir_entries_len; i++) {
		if (i == screen_pos) {
			snprintf(l[i], 21, "* %s", menu_data.dir_entries[screen_page*MENU_LINES+i]->d_name);
		} else {
			snprintf(l[i], 21, "  %s", menu_data.dir_entries[screen_page*MENU_LINES+i]->d_name);
		}
	}
	
	lcd_print_lines(l, 4);
	
	menu_data.redraw = 0;
}
static void menu_dir_handle(int c) {
	switch (c) {
		case LEFT:
			if (!strcmp(menu_data.top_dir, menu_data.cur_dir)) {
				menu_data.state = MAIN;
			} else {
				menu_dir_prev_dir();
			}
			
			break;
		case RIGHT:
			{
				struct dirent *d = menu_data.dir_entries[menu_data.dir_pointer];
				if (d->d_type == DT_DIR) {
					menu_dir_append_dir(d->d_name);
				}
				
				if (d->d_type == DT_REG) {
					char temp[MENU_BUF_SIZE];
					snprintf(temp, MENU_BUF_SIZE, "%s/%s", menu_data.cur_dir, d->d_name);
					menu_callback_set_file(temp);
					menu_data.state = SID;
				}
			}
			break;
		case UP:
			menu_data.dir_pointer--;
			if (menu_data.dir_pointer < 0) {
				menu_data.dir_pointer = menu_data.dir_entries_len-1;
			}
			break;
		case DOWN:
			menu_data.dir_pointer++;
			if (menu_data.dir_pointer >= menu_data.dir_entries_len) {
				menu_data.dir_pointer = 0;
			}
			break;
	}
}

static void menu_sid_display(void) {
	if (! menu_data.redraw) {
		return;
	}
	
	char songname[21];
	char songauthor[21];
	char songsubsong[21];
	char copyright[21];
	
	char* lines[4];
	lines[0] = songname;
	lines[1] = songauthor;
	lines[2] = songsubsong;
	lines[3] = copyright;
	
	snprintf(songname, 21, "%s", sh.name);
	snprintf(songauthor, 21, "%s", sh.author);
	snprintf(copyright, 21, "(C) %s", sh.released);
	snprintf(songsubsong, 21, "Song %d of %d", c64_current_song+1, sh.songs);
	
	lcd_print_lines(lines, 4);
	menu_data.redraw = 0;
}

static void menu_sid_handle(int c) {
	switch (c) {
		case LEFT:
			menu_data.state = DIRECTORY;
			break;
		case RIGHT:
			lcd_init();
			break;
		case UP:
			{
				int n = c64_current_song + 2;
				if (n > sh.songs) {
					n = 1;
				}
				setSubSong(n);
			}
			break;
		case DOWN:
			{
				int n = c64_current_song;
				if (n <= 0) {
					n = sh.songs;
				}
				setSubSong(n);
			}
			break;
	}
}

void menu_init(char* top_dir) {
	memset(&menu_data, 0, sizeof(menu_data));
	
	snprintf(menu_data.top_dir, MENU_BUF_SIZE, "%s", top_dir);
	snprintf(menu_data.cur_dir, MENU_BUF_SIZE, "%s", menu_data.top_dir);
	
	menu_data.state = MAIN;
	menu_data.redraw = 1;
}

void menu_run(void) {
	int c;
	switch (menu_data.state) {
		case MAIN:
			menu_main_display();
			break;
		case DIRECTORY:
			menu_dir_display();
			break;
		case SID:
			menu_sid_display();
			break;
	}
	
	c = menu_callback_read();
	if (c >= 0) {
		menu_data.redraw = 1;
		switch (menu_data.state) {
			case MAIN:
				menu_main_handle(c);
				break;
			case DIRECTORY:
				menu_dir_handle(c);
				break;
			case SID:
				menu_sid_handle(c);
				break;
		}
	}
}

int menu_state(void) {
	return menu_data.state;
}

