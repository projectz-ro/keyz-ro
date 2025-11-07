#include "raylib.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_KEYBINDS 100
#define MAX_PROGRAMS 20
#define MAX_STRING 256

typedef struct {
  char key[MAX_STRING];
  char description[MAX_STRING];
} Keybind;

typedef struct {
  char name[MAX_STRING];
  Keybind keys[MAX_KEYBINDS];
  int keyCount;
} Program;

typedef struct {
  Program programs[MAX_PROGRAMS];
  int programCount;
} Config;

void createDefaultConfig(const char *path) {
  FILE *file = fopen(path, "w");
  if (!file)
    return;

  fprintf(file, "[\n");
  fprintf(file, "  {\n");
  fprintf(file, "    \"name\": \"Example Program\",\n");
  fprintf(file, "    \"keys\": [\n");
  fprintf(file, "      {\n");
  fprintf(file, "        \"key\": \"A\",\n");
  fprintf(file, "        \"description\": \"Edit the config at %s\"\n", path);
  fprintf(file, "      }\n");
  fprintf(file, "    ]\n");
  fprintf(file, "  }\n");
  fprintf(file, "]\n");

  fclose(file);
}

int loadConfig(const char *path, Config *config) {
  FILE *file = fopen(path, "r");
  if (!file)
    return 0;

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *data = malloc(length + 1);
  if (!data) {
    fclose(file);
    return 0;
  }
  fread(data, 1, length, file);
  data[length] = '\0';
  fclose(file);

  cJSON *root = cJSON_Parse(data);
  free(data);
  if (!root || !cJSON_IsArray(root)) {
    if (root)
      cJSON_Delete(root);
    return 0;
  }

  config->programCount = 0;
  int programIndex = 0;
  cJSON *prog = NULL;
  cJSON_ArrayForEach(prog, root) {
    if (programIndex >= MAX_PROGRAMS)
      break;
    cJSON *name = cJSON_GetObjectItem(prog, "name");
    cJSON *keys = cJSON_GetObjectItem(prog, "keys");
    if (!cJSON_IsString(name) || !cJSON_IsArray(keys))
      continue;

    Program *p = &config->programs[programIndex++];
    strncpy(p->name, name->valuestring, MAX_STRING - 1);
    p->keyCount = 0;

    int keyIndex = 0;
    cJSON *keyItem = NULL;
    cJSON_ArrayForEach(keyItem, keys) {
      if (keyIndex >= MAX_KEYBINDS)
        break;
      cJSON *key = cJSON_GetObjectItem(keyItem, "key");
      cJSON *desc = cJSON_GetObjectItem(keyItem, "description");
      if (!cJSON_IsString(key) || !cJSON_IsString(desc))
        continue;
      strncpy(p->keys[keyIndex].key, key->valuestring, MAX_STRING - 1);
      strncpy(p->keys[keyIndex].description, desc->valuestring, MAX_STRING - 1);
      keyIndex++;
    }
    p->keyCount = keyIndex;
  }
  config->programCount = programIndex;
  cJSON_Delete(root);
  return 1;
}

int main(void) {
  const char *home = getenv("HOME");
  char configPath[512];
  snprintf(configPath, sizeof(configPath), "%s/.config/keyz-ro/config.json",
           home);

  char dirPath[512];
  snprintf(dirPath, sizeof(dirPath), "%s/.config/keyz-ro", home);
  mkdir(dirPath, 0755);

  Config config = {0};
  if (!loadConfig(configPath, &config)) {
    createDefaultConfig(configPath);
    loadConfig(configPath, &config);
  }

  int totalKeybinds = 0;
  for (int i = 0; i < config.programCount; i++) {
    totalKeybinds += config.programs[i].keyCount;
  }

  // Colors
  Color winBG = {42, 42, 42, 255};            // #2a2a2a
  Color headBG = {53, 51, 49, 255};           // #353331
  Color headTitleText = {226, 161, 35, 255};  // #e2a123
  Color headStatsText = {153, 137, 122, 255}; // #99897a
  Color programText = {138, 173, 159, 255};   // #8aad9f
  Color kbText = {255, 77, 55, 255};          // #ff4d37
  Color kbDescText = {224, 207, 170, 255};    // #e0cfaa

  // Initialize window
  SetConfigFlags(FLAG_WINDOW_UNDECORATED | FLAG_BORDERLESS_WINDOWED_MODE |
                 FLAG_WINDOW_TRANSPARENT);
  const int screenW = 600;
  const int screenH = 400;
  InitWindow(screenW, screenH, "KeyZ-ro");
  SetWindowMinSize(screenW, screenH);
  SetWindowMaxSize(screenW, screenH);

  // Center window
  int monitorW = GetMonitorWidth(0);
  int monitorH = GetMonitorHeight(0);
  SetWindowPosition((monitorW - screenW) / 2, (monitorH - screenH) / 2);

  char *jbMonoPath = "/usr/share/fonts/TTF/JetBrainsMonoNerdFontMono-Bold.ttf";
  char *dvMonoPath = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";

  // Load font
  Font fontSm = LoadFontEx(jbMonoPath, 20, 0, 250);
  Font fontMd = LoadFontEx(jbMonoPath, 32, 0, 250);
  Font fontLg = LoadFontEx(jbMonoPath, 42, 0, 250);
  if (fontMd.baseSize == 0) {
    fontSm = LoadFontEx(dvMonoPath, 12, 0, 250);
    fontMd = LoadFontEx(dvMonoPath, 24, 0, 250);
    fontLg = LoadFontEx(dvMonoPath, 42, 0, 250);
  }
  if (fontSm.baseSize == 0 || fontMd.baseSize == 0 || fontLg.baseSize == 0) {
    fontSm = GetFontDefault();
    fontMd = GetFontDefault();
    fontLg = GetFontDefault();
  }
  SetTargetFPS(60);

  float baseFontSize = 20.0f;
  float lineHeight = baseFontSize + 8.0f;
  float scroll = 0.0f;
  float padding = 20.0f;
  float headerHeight = 64.0f;

  while (!WindowShouldClose()) {
    scroll -= GetMouseWheelMove() * 30.0f;
    if (scroll < 0)
      scroll = 0;

    float contentHeight = 0;
    for (int i = 0; i < config.programCount; i++) {
      contentHeight += 32 + 8.0f;
      contentHeight += config.programs[i].keyCount * lineHeight;
    }

    float maxScroll = contentHeight - headerHeight;
    if (scroll < 0)
      scroll = 0;
    if (scroll > maxScroll)
      scroll = maxScroll;

    BeginDrawing();
    ClearBackground(winBG);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    DrawRectangle(0, 0, sw, (int)headerHeight, headBG);
    DrawTextEx(fontLg, "KeyZ-ro", (Vector2){20, 18}, 42, 1, headTitleText);

    char statsText[128];
    int statTextSize = 18;

    snprintf(statsText, sizeof(statsText), "%d binding(s)", totalKeybinds);
    int tKbStart = screenW - MeasureText(statsText, statTextSize) - 32;
    DrawTextEx(fontSm, statsText, (Vector2){tKbStart, 30}, statTextSize, 1,
               headStatsText);

    snprintf(statsText, sizeof(statsText), "%d program(s)",
             config.programCount);
    int tPStart = tKbStart - MeasureText(statsText, statTextSize) - 12;
    DrawTextEx(fontSm, statsText, (Vector2){tPStart, 30}, statTextSize, 1,
               headStatsText);

    DrawLine(0, (int)headerHeight, sw, (int)headerHeight, (Color){2, 2, 2, 1});

    float startY = headerHeight + padding - scroll;

    for (int i = 0; i < config.programCount; i++) {
      Program *program = &config.programs[i];

      if (i) {
        startY += lineHeight * 1.5f;
      }

      if (startY >= headerHeight && startY < sh) {
        DrawTextEx(fontMd, program->name, (Vector2){padding, startY},
                   baseFontSize + 12, 1, programText);
      }
      startY += lineHeight * 1.5f;

      for (int j = 0; j < program->keyCount; j++) {
        Keybind *kb = &program->keys[j];

        if (startY >= headerHeight && startY + lineHeight <= sh) {
          float keyWidth = MeasureTextEx(fontMd, kb->key, baseFontSize, 1).x;
          DrawTextEx(fontMd, kb->key, (Vector2){padding + 20, startY},
                     baseFontSize + 4, 1, kbText);

          float descX = padding + 40;
          startY += lineHeight;
          DrawTextEx(fontLg, kb->description, (Vector2){descX, startY},
                     baseFontSize, 1, kbDescText);
        }
        startY += lineHeight;
      }
    }
    float arrowSize = baseFontSize * 2;
    if (scroll < maxScroll) {
      DrawTextPro(fontLg, "^",
                  (Vector2){screenW - arrowSize, screenH - arrowSize},
                  (Vector2){(int){arrowSize / 2}, (int){arrowSize / 2}}, 180.0f,
                  arrowSize, 1, headTitleText);
    }
    if (scroll > 0) {
      DrawTextEx(fontLg, "^",
                 (Vector2){screenW - arrowSize, headerHeight + arrowSize},
                 arrowSize, 1, headTitleText);
    }

    EndDrawing();
  }

  if (fontMd.baseSize != 0)
    UnloadFont(fontMd);
  CloseWindow();

  return 0;
}
