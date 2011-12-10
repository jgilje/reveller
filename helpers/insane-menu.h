#ifndef INSANE_MENU
#define INSANE_MENU

enum MENU_STATE {
	MAIN,
	DIRECTORY,
	SID
};

enum KEYS {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

extern int menu_callback_read(void);
extern void menu_callback_set_file(char* file);

void menu_init(char* top_dir);
void menu_run(void);

#endif

