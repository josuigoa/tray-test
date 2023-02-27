#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cJSON.h"

#if defined (_WIN32) || defined (_WIN64)
#define TRAY_WINAPI 1
#elif defined (__linux__) || defined (linux) || defined (__linux)
#define TRAY_AYATANA_APPINDICATOR 1
#elif defined (__APPLE__) || defined (__MACH__)
#define TRAY_APPKIT 1
#endif

#include "tray.h"

static void tray_cb(struct tray_menu *item) {
  (void)item;
  printf("%s\n", item->text);
  fflush(NULL);
  if (item->exit) {
    tray_exit();
  }
}

struct tray_menu* _create_menu(cJSON* json_menu) {
  cJSON* item;
  int menu_length = cJSON_GetArraySize(json_menu);
  struct tray_menu* menu = (struct tray_menu *) malloc(sizeof(struct tray_menu) * (menu_length + 1));
  for (int i = 0; i < menu_length; i++) {

    item = cJSON_GetArrayItem(json_menu, i);
    menu[i] = (struct tray_menu) {.text = cJSON_GetObjectItemCaseSensitive(item, "text")->valuestring, .cb = tray_cb};
  
    if (cJSON_HasObjectItem(item, "disabled")) {
      menu[i].disabled = cJSON_GetObjectItemCaseSensitive(item, "disabled")->valueint;
    }

    if (cJSON_HasObjectItem(item, "checked")) {
      menu[i].exit = cJSON_GetObjectItemCaseSensitive(item, "checked")->valueint;
    }

    if (cJSON_HasObjectItem(item, "checkbox")) {
      menu[i].exit = cJSON_GetObjectItemCaseSensitive(item, "checkbox")->valueint;
    }

    if (cJSON_HasObjectItem(item, "exit")) {
      menu[i].exit = cJSON_GetObjectItemCaseSensitive(item, "exit")->valueint;
    }

    if (cJSON_HasObjectItem(item, "menu")) {
      menu[i].submenu = _create_menu(cJSON_GetObjectItemCaseSensitive(item, "menu"));
    }
  }
  menu[menu_length] = (struct tray_menu) {.text = NULL};
  return menu;
}

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("\nExpected the menu definition as argument");
    return 1;
  }

  cJSON *json_menu = cJSON_Parse(argv[1]);
  if (json_menu == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
        fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    return 1;
  }

  char* icon_path;

  cJSON* icon = cJSON_GetObjectItemCaseSensitive(json_menu, "icon");
  if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
    icon_path = icon->valuestring;
  }

  struct tray ideckia_tray = { .icon = icon_path };
  if (cJSON_HasObjectItem(json_menu, "menu")) {
    ideckia_tray.menu = _create_menu(cJSON_GetObjectItemCaseSensitive(json_menu, "menu"));
  }

  if (tray_init(&ideckia_tray) < 0) {
    printf("failed to create tray\n");
    return 1;
  }
  while (tray_loop(1) == 0) {}

  return 0;
}
