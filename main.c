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

int main(int argc, char *argv[]) {

  if (argc < 2) {
    printf("\nExpected the menu definition as argument");
    return 1;
  }

  cJSON *menu_json = cJSON_Parse(argv[1]);
  if (menu_json == NULL) {
    const char *error_ptr = cJSON_GetErrorPtr();
    if (error_ptr != NULL) {
        fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    return 1;
  }

  cJSON* item;
  cJSON* text;
  cJSON* menu;
  cJSON* icon;
  cJSON* sub_menu;
  cJSON* sub_item;
  cJSON* sub_text;

  char* icon_path;

  icon = cJSON_GetObjectItemCaseSensitive(menu_json, "icon");
  if (cJSON_IsString(icon) && (icon->valuestring != NULL)) {
    icon_path = icon->valuestring;
  }
  free(icon);

  struct tray ideckia_tray = {.icon = icon_path};

  menu = cJSON_GetObjectItemCaseSensitive(menu_json, "menu");
  int menu_length = cJSON_GetArraySize(menu);
  ideckia_tray.menu = (struct tray_menu*) malloc(sizeof(struct tray_menu) * (menu_length + 1));
  for (int i = 0; i < menu_length; i++) {
    item = cJSON_GetArrayItem(menu, i);
    ideckia_tray.menu[i] = (struct tray_menu) {.text = cJSON_GetObjectItemCaseSensitive(item, "text")->valuestring, .cb = tray_cb};
    if (cJSON_HasObjectItem(item, "disabled")) {
        ideckia_tray.menu[i].disabled = cJSON_GetObjectItemCaseSensitive(item, "disabled")->valueint;
    }
    if (cJSON_HasObjectItem(item, "exit")) {
        ideckia_tray.menu[i].exit = cJSON_GetObjectItemCaseSensitive(item, "exit")->valueint;
    }

    if (cJSON_HasObjectItem(item, "sub")) {
      sub_menu = cJSON_GetObjectItemCaseSensitive(item, "sub");
      int sub_menu_length = cJSON_GetArraySize(sub_menu);
      ideckia_tray.menu[i].submenu = (struct tray_menu *) malloc(sizeof(struct tray_menu) * (sub_menu_length + 1));

      for (int j = 0; j < sub_menu_length; j++) {
        sub_item = cJSON_GetArrayItem(sub_menu, j);
        ideckia_tray.menu[i].submenu[j] = (struct tray_menu) {.text = cJSON_GetObjectItemCaseSensitive(sub_item, "text")->valuestring, .cb = tray_cb};
        if (cJSON_HasObjectItem(item, "disabled")) {
            ideckia_tray.menu[i].submenu[j].disabled = cJSON_GetObjectItemCaseSensitive(sub_item, "disabled")->valueint;
        }
        if (cJSON_HasObjectItem(item, "exit")) {
            ideckia_tray.menu[i].submenu[j].exit = cJSON_GetObjectItemCaseSensitive(sub_item, "exit")->valueint;
        }
      }
      ideckia_tray.menu[i].submenu[sub_menu_length] = (struct tray_menu) {.text = NULL};
    }
    
  }

  ideckia_tray.menu[menu_length] = (struct tray_menu) {.text = NULL};
  free(menu);
  free(item);
  free(sub_menu);
  free(sub_item);

  if (tray_init(&ideckia_tray) < 0) {
    printf("failed to create tray\n");
    return 1;
  }
  while (tray_loop(1) == 0) {}

  return 0;
}
