#include "Lua.h"
#include "Console.h"
#include "DB.h"

#include "WorldView.h"
#include "Managers.h"
#include "GameState.h"
#include "ImageLibrary.h"
#include "assets.h"
#include "segautils/BitBuffer.h"
#include "SEGA/App.h"

#include "liblua/lauxlib.h"
#include "liblua/lualib.h"


static int slua_DBInsertImage(lua_State *L);
static int slua_DBInsertImageFolder(lua_State *L);
static int slua_DBInsertPalette(lua_State *L);
static int slua_DBInsertPaletteFolder(lua_State *L);


void luaLoadDBLibrary(lua_State *L) {

   lua_newtable(L);
   luaPushFunctionTable(L, "insertImage", &slua_DBInsertImage);
   luaPushFunctionTable(L, "insertImageFolder", &slua_DBInsertImageFolder);
   luaPushFunctionTable(L, "insertPalette", &slua_DBInsertPalette);
   luaPushFunctionTable(L, "insertPaletteFolder", &slua_DBInsertPaletteFolder);


   lua_setglobal(L, LLIB_DB);

}

int slua_DBInsertImageFolder(lua_State *L) {
   WorldView *view = luaGetWorldView(L);
   const char *path = lua_tostring(L, 1);
   vec(StringPtr) *list = NULL;   
   int r = appListFiles(path, APP_FILE_FILE_ONLY, &list, "ega");

   consolePrintLine(view->console, "Inserting (*.ega) images in [c=0,5]%s[/c]", path);

   if (!r){
      vecForEach(StringPtr, str, list, {
         String *fnameOnly = stringGetFilename(*str);
         lua_pushcfunction(L, &slua_DBInsertImage);
         lua_pushstring(L, c_str(fnameOnly));
         lua_pushstring(L, c_str(*str));
         
         lua_call(L, 2, 0);
         stringDestroy(fnameOnly);
      });

      consolePrintLine(view->console, "Inserted [c=0,5]%i[/c] images.", vecSize(StringPtr)(list));
      vecDestroy(StringPtr)(list);
   }   
   else {
      consolePrintLine(view->console, "Inserted [c=0,5]%i[/c] images.", 0);
   }
   
   
   return 0;
}

int slua_DBInsertImage(lua_State *L) {
   WorldView *view = luaGetWorldView(L);
   const char *id = lua_tostring(L, 1);
   const char *path = lua_tostring(L, 2);
   StringView interned = stringIntern(id);

   DBImage img = {.id = stringCreate(interned) };
   if (!(img.image = readFullFile(path, &img.imageSize))) {      
      lua_pushstring(L, "Unable to open image file");
      lua_error(L);
   }


   if (dbImageInsert(view->db, &img)) {
      lua_pushstring(L, dbGetError((DBBase*)view->db));
      lua_error(L);
   }

   dbImageDestroy(&img);
   consolePrintLine(view->console, "Image [c=0,5]%s[/c] inserted.", id);
   return 0;
}

int slua_DBInsertPalette(lua_State *L) {
   WorldView *view = luaGetWorldView(L);
   const char *id = lua_tostring(L, 1);
   const char *path = lua_tostring(L, 2);
   StringView interned = stringIntern(id);
   DBPalette pal = { .id = stringCreate(interned) };
   Palette p = paletteDeserialize(path);

   if (!memcmp(&p, &(Palette){0}, sizeof(Palette))) {
      lua_pushstring(L, "Failed to open palette file");
      lua_error(L);
   }

   pal.palette = &p;
   pal.paletteSize = sizeof(Palette);

   if (dbPaletteInsert(view->db, &pal)) {
      lua_pushstring(L, dbGetError((DBBase*)view->db));
      lua_error(L);
   }

   pal.palette = NULL;//dont want destory to free our stack-palette
   dbPaletteDestroy(&pal);

   consolePrintLine(view->console, "Palette [c=0,5]%s[/c] inserted.", id);
   return 0;
}

int slua_DBInsertPaletteFolder(lua_State *L) {
   WorldView *view = luaGetWorldView(L);
   const char *path = lua_tostring(L, 1);
   vec(StringPtr) *list = NULL;
   int r = appListFiles(path, APP_FILE_FILE_ONLY, &list, "pal");

   consolePrintLine(view->console, "Inserting (*.pal) palettes in [c=0,5]%s[/c]", path);

   if (!r) {
      vecForEach(StringPtr, str, list, {
         String *fnameOnly = stringGetFilename(*str);
         lua_pushcfunction(L, &slua_DBInsertPalette);
         lua_pushstring(L, c_str(fnameOnly));
         lua_pushstring(L, c_str(*str));

         lua_call(L, 2, 0);
         stringDestroy(fnameOnly);
      });

      consolePrintLine(view->console, "Inserted [c=0,5]%i[/c] palette.", vecSize(StringPtr)(list));
      vecDestroy(StringPtr)(list);
   }
   else {
      consolePrintLine(view->console, "Inserted [c=0,5]%i[/c] palette.", 0);
   }


   return 0;
}

