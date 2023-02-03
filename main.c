#include <stdio.h>
#include <string.h>

#if defined (_WIN32) || defined (_WIN64)
#define TRAY_WINAPI 1
#elif defined (__linux__) || defined (linux) || defined (__linux)
#define TRAY_AYATANA_APPINDICATOR 1
#elif defined (__APPLE__) || defined (__MACH__)
#define TRAY_APPKIT 1
#endif

#include "tray.h"

static struct tray tray;

static void editor_cb(struct tray_menu *item) {
  (void)item;
  printf("editor\n");
  fflush(NULL);
  tray_update(&tray);
}

static void quit_cb(struct tray_menu *item) {
  (void)item;
  printf("quit\n");
  fflush(NULL);
  tray_exit();
}

static struct tray tray = {
    .icon = NULL,
    .menu =
        (struct tray_menu[]){
            {.text = "ideckia port: xxxx", .disabled = 1},
            {.text = "editor", .cb = editor_cb},
            {.text = "-"},
            {.text = "quit", .cb = quit_cb},
            {.text = NULL}},
};

int main(int argc, char *argv[]) {

  if (argc < 3) {
    printf("\nExpected the icon path and the port as argument");
    return 1;
  }

  tray.icon = argv[1];
  char menu_port[30] = "ideckia port: ";
  strcat(menu_port, argv[2]);
  tray.menu[0].text = menu_port;

  if (tray_init(&tray) < 0) {
    printf("failed to create tray\n");
    return 1;
  }
  while (tray_loop(1) == 0) {}

  return 0;
}
