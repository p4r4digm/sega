#include "Entities.h"
#include "segautils\IntrusiveHeap.h"
#include "segashared\CheckedMemory.h"
#include "segautils\Defs.h"
#include "segautils\BitTwiddling.h"

typedef struct Entity_t{ 
   QueueNode node;
   int ID;
   byte loaded;
   EntitySystem *system;
};

struct ComponentList_t{
   ComponentVTable *cvt;

   ComponentListData list;
   int lookup[MAX_ENTITIES];
};

int *compListGetLookup(ComponentList *self){
   return self->lookup;
}

ComponentListData compListGetList(ComponentList *self){
   return self->list;
}

ComponentVTable *compListGetVTable(ComponentList *self){
   return self->cvt;
}

#define VectorT ComponentList
#include "segautils\Vector_Create.h"

void _cvtDestroy(ComponentList *self){
   if (self->cvt->destroy){
      self->cvt->destroy(self->list);
   }
}

struct EntitySystem_t {
   Entity *entityPool;
   PriorityQueue *eQueue;
   size_t eCount;

   vec(ComponentList) *lists;
};

Entity *_eNodeCompareFunc(Entity *n1, Entity *n2){
   return n1->ID < n2->ID ? n1 : n2;
}

EntitySystem *entitySystemCreate(){
   EntitySystem *out = checkedCalloc(1, sizeof(EntitySystem));
   out->eQueue = priorityQueueCreate(offsetof(Entity, node), (PQCompareFunc)&_eNodeCompareFunc);
   out->entityPool = checkedCalloc(MAX_ENTITIES, sizeof(Entity));
   out->lists = vecCreate(ComponentList)(&_cvtDestroy);

   return out;
}
void entitySystemDestroy(EntitySystem *self){   
   priorityQueueDestroy(self->eQueue);
   checkedFree(self->entityPool);
   vecDestroy(ComponentList)(self->lists);
   checkedFree(self);
}

void entitySystemRegisterCompList(EntitySystem *self, size_t rtti, ComponentVTable *table){
   ComponentList cl = { 0 };
   int undef = -1;
   size_t listCount = vecSize(ComponentList)(self->lists);

   cl.cvt = table;
   if (table->create){
      cl.list = table->create();
   }

   //init to -1
   STOSD((unsigned long*)cl.lookup, *(unsigned long*)&undef, MAX_ENTITIES);

   if (rtti >= listCount){
      vecResize(ComponentList)(self->lists, rtti + 1, NULL);
   }

   //copy component list into the lists vector
   memcpy(vecAt(ComponentList)(self->lists, rtti), &cl, sizeof(cl));
}
ComponentList *entitySystemGetCompList(EntitySystem *self, size_t rtti){

   if (rtti >= vecSize(ComponentList)(self->lists)){
      return NULL;
   }

   return vecAt(ComponentList)(self->lists, rtti);
}


Entity *entityCreate(EntitySystem *system){
   Entity *out;

   if (priorityQueueIsEmpty(system->eQueue)){
      int ID = system->eCount++;
      
      out = system->entityPool + ID;
      out->system = system;      
      out->ID = ID;
   }
   else {
      int ID = ((Entity*)priorityQueuePop(system->eQueue))->ID;
      out = system->entityPool + ID;
   }
   
   out->loaded = true;
   return out;
}

void entityDestroy(Entity *self){
   vec(ComponentList) *v = self->system->lists;
   size_t compCount = vecSize(ComponentList)(v);
   size_t i;
   for (i = 0; i < compCount; ++i){
      ComponentList *list = vecAt(ComponentList)(v, i);
      int compIndex = list->lookup[self->ID];
      Component moved;

      list->lookup[self->ID] = -1;

      list->cvt->remove(list->list, compIndex);
      moved = list->cvt->getAt(list->list, compIndex);
      if (moved){
         int movedEntity = componentGetParentID(moved);
         list->lookup[movedEntity] = compIndex;
      }

   }

   self->loaded = false;
   priorityQueuePush(self->system->eQueue, self);
}

Entity *componentGetParent(Component self, EntitySystem *system){
   return system->entityPool + componentGetParentID(self);
}
int componentGetParentID(Component self){
   return *(int*)((byte *)self - sizeof(int));
}

int entityGetID(Entity *self){
   return self->ID;
}
EntitySystem *entityGetSystem(Entity *self){
   return self->system;
}