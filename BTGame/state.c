/***********************************************************************
   WARNING: This file generated by robots.  Do not attempt to modify.

   This API is for use with state.db
   Which contains 1 table(s).
***********************************************************************/

#include "state.h"
#include "DB.h"
#include "segashared/CheckedMemory.h"
#include "sqlite/sqlite3.h"

void dbActorDestroyStatements(DB_state *db);
int dbActorCreateTable(DB_state *db);

typedef struct {
   sqlite3_stmt *insert;
   sqlite3_stmt *update;
   sqlite3_stmt *selectAll;
   sqlite3_stmt *deleteAll;
   sqlite3_stmt *selectByid;
   sqlite3_stmt *deleteByid;
} DBActorStmts;

struct DB_state{
   DBBase base;
   DBActorStmts ActorStmts;
};

DB_state *db_stateCreate(){
   DB_state *out = checkedCalloc(1, sizeof(DB_state));
   return out;
}

void db_stateDestroy(DB_state *self){
   dbDestroy((DBBase*)self);
   dbActorDestroyStatements(self);
   checkedFree(self);
}

int db_stateCreateTables(DB_state *self){
   int result = 0;

   if((result = dbActorCreateTable(self)) != DB_SUCCESS){ return result; }

   return DB_SUCCESS;
}

#define VectorTPart DBActor
#include "segautils/Vector_Impl.h"

void dbActorDestroy(DBActor *self){
   if(self->id){
      stringDestroy(self->id);
      self->id = NULL;
   }
}
void dbActorDestroyStatements(DB_state *db){
   if(db->ActorStmts.insert){
      sqlite3_finalize(db->ActorStmts.insert);
      db->ActorStmts.insert = NULL;
   }
   if(db->ActorStmts.update){
      sqlite3_finalize(db->ActorStmts.update);
      db->ActorStmts.update = NULL;
   }
   if(db->ActorStmts.selectAll){
      sqlite3_finalize(db->ActorStmts.selectAll);
      db->ActorStmts.selectAll = NULL;
   }
   if(db->ActorStmts.deleteAll){
      sqlite3_finalize(db->ActorStmts.deleteAll);
      db->ActorStmts.deleteAll = NULL;
   }
   if(db->ActorStmts.selectByid){
      sqlite3_finalize(db->ActorStmts.selectByid);
      db->ActorStmts.selectByid = NULL;
   }
   if(db->ActorStmts.deleteByid){
      sqlite3_finalize(db->ActorStmts.deleteByid);
      db->ActorStmts.deleteByid = NULL;
   }
}
int dbActorCreateTable(DB_state *db){
   static const char *cmd = "CREATE TABLE \"Actor\" (\"id\" STRING PRIMARY KEY UNIQUE ON CONFLICT REPLACE);";
   return dbExecute((DBBase*)db, cmd);
}
int dbActorInsert(DB_state *db, const DBActor *obj){
   int result = 0;
   static const char *stmt = "INSERT INTO \"Actor\" (\"id\") VALUES (:id);";
   if(dbPrepareStatement((DBBase*)db, &db->ActorStmts.insert, stmt) != DB_SUCCESS){
      return DB_FAILURE;
   }

   //bind the values
   result = sqlite3_bind_text(db->ActorStmts.insert, 1, c_str(obj->id), -1, NULL);
   if (result != SQLITE_OK) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   //now run it
   result = sqlite3_step(db->ActorStmts.insert);
   if (result != SQLITE_DONE) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   return DB_SUCCESS;
}
int dbActorUpdate(DB_state *db, const DBActor *obj){
   int result = 0;
   static const char *stmt = "UPDATE \"Actor\" SET () WHERE (\"id\" = :id)";
   if(dbPrepareStatement((DBBase*)db, &db->ActorStmts.update, stmt) != DB_SUCCESS){
      return DB_FAILURE;
   }

   //bind the values
   //primary key:
   result = sqlite3_bind_text(db->ActorStmts.update, 1, c_str(obj->id), -1, NULL);
   if (result != SQLITE_OK) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   //now run it
   result = sqlite3_step(db->ActorStmts.update);
   if (result != SQLITE_DONE) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   return DB_SUCCESS;
}
vec(DBActor) *dbActorSelectAll(DB_state *db){
   int result = 0;
   static const char *stmt = "SELECT * FROM \"Actor\";";
   if(dbPrepareStatement((DBBase*)db, &db->ActorStmts.selectAll, stmt) != DB_SUCCESS){
      return NULL;
   }

   vec(DBActor) *out = vecCreate(DBActor)(&dbActorDestroy);

   while((result = sqlite3_step(db->ActorStmts.selectAll)) == SQLITE_ROW){
      DBActor newObj = {0};

      newObj.id = stringCreate(sqlite3_column_text(db->ActorStmts.selectAll, 0));
      
      vecPushBack(DBActor)(out, &newObj);

   };

   if(result != SQLITE_DONE){
      vecDestroy(DBActor)(out);
      return NULL;
   }

   return out;
}
DBActor dbActorSelectFirstByid(DB_state *db, const char *id){
   DBActor out = {0};
   int result = 0;
   static const char *stmt = "SELECT * FROM \"Actor\" WHERE \"id\" = :id;";
   if(dbPrepareStatement((DBBase*)db, &db->ActorStmts.selectByid, stmt) != DB_SUCCESS){
      return out;
   }

   result = sqlite3_bind_text(db->ActorStmts.selectByid, 1, id, -1, NULL);
   if (result != SQLITE_OK) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return out;
   }

   if((result = sqlite3_step(db->ActorStmts.selectByid)) == SQLITE_ROW){
      out.id = stringCreate(sqlite3_column_text(db->ActorStmts.selectByid, 0));
   };

   return out;
}
int dbActorDeleteAll(DB_state *db){
return DB_SUCCESS;
}
int dbActorDeleteByid(DB_state *db, const char *id){
   int result = 0;
   static const char *stmt = "DELETE FROM \"Actor\" WHERE (\"id\" = :id);";
   if(dbPrepareStatement((DBBase*)db, &db->ActorStmts.deleteByid, stmt) != DB_SUCCESS){
      return DB_FAILURE;
   }

   //primary key:
   result = sqlite3_bind_text(db->ActorStmts.deleteByid, 1, id, -1, NULL);
   if (result != SQLITE_OK) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   //now run it
   result = sqlite3_step(db->ActorStmts.deleteByid);
   if (result != SQLITE_DONE) {
      stringSet(db->base.err, sqlite3_errmsg(db->base.conn));
      return DB_FAILURE;
   }

   return DB_SUCCESS;
}
