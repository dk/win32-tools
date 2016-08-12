#include "list.h"
#include <string.h>
#include <stdlib.h>

void
list_create( PList slf, int size, int delta)
{
   if ( !slf) return;
   memset( slf, 0, sizeof( List));
   slf-> delta = ( delta > 0) ? delta : 1;
   slf-> size  = size;
   slf-> items = ( size > 0) ? malloc( size * sizeof( Handle)) : nil;
}

PList
plist_create( int size, int delta)
{
   PList new_list = ( PList) malloc( sizeof( List));
   if ( new_list != nil) {
      list_create( new_list, size, delta);
   }
   return new_list;
}

void
list_destroy( PList slf)
{
   if ( !slf) return;
   free( slf-> items);
   slf-> items = nil;
   slf-> count = 0;
   slf-> size  = 0;
}

void
plist_destroy( PList slf)
{
   if ( slf != NULL) {
      list_destroy( slf);
      free( slf);
   }
}

int
list_add( PList slf, Handle item)
{
   if ( !slf) return -1;
   if ( slf-> count == slf-> size)
   {
      Handle * old = slf-> items;
      slf-> items = malloc(( slf-> size + slf-> delta) * sizeof( Handle));
      if ( old) {
         memcpy( slf-> items, old, slf-> size * sizeof( Handle));
         free( old);
      }
      slf-> size += slf-> delta;
   }
   slf-> items[ slf-> count++] = item;
   return slf-> count - 1;
}

void
list_insert_at( PList slf, Handle item, int pos)
{
   int max;
   Handle save;
   if ( list_add( slf, item) < 0) return;
   max = slf-> count - 1;
   if ( pos < 0 || pos >= max) return;
   save = slf-> items[ pos];
   memmove( &slf-> items[ pos], &slf-> items[ pos + 1], ( max - pos) * sizeof( Handle));
   slf-> items[ max] = save;
}

int
list_index_of( PList slf, Handle item)
{
   int i;
   if ( !slf ) return -1;
   for ( i = 0; i < slf-> count; i++)
      if ( slf-> items[ i] == item) return i;
   return -1;
}

void
list_delete( PList slf, Handle item)
{
   list_delete_at( slf, list_index_of( slf, item));
}

void
list_delete_at( PList slf, int index)
{
   if ( !slf || index < 0 || index >= slf-> count) return;
   slf-> count--;
   if ( index == slf-> count) return;
   memmove( &slf-> items[ index], &slf-> items[ index + 1], ( slf-> count - index) * sizeof( Handle));
}

Handle
list_at( PList slf, int index)
{
   return (( index < 0 || !slf) || index >= slf-> count) ? nilHandle : slf-> items[ index];
}

int
list_first_that( PList slf, void * action, void * params)
{
   int toRet = -1, i, cnt = slf-> count;
   Handle * list;
   if ( !action || !slf || !cnt) return -1;
   list = malloc( slf-> count * sizeof( Handle));
   memcpy( list, slf-> items, slf-> count * sizeof( Handle));
   for ( i = 0; i < cnt; i++)
      if ((( PListProc) action)( list[ i], params)) {
         toRet = i;
         break;
      }
   free( list);
   return toRet;
}

void
list_delete_all( PList slf, Bool kill)
{
   if ( !slf || ( slf-> count == 0)) return;
   if ( kill ) {
      int i;
      for ( i = 0; i < slf-> count; i++)
         free(( void*) slf-> items[ i]);
   }
   slf-> count = 0;
}

