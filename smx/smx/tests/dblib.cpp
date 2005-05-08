#include <db4/db_cxx.h>

int main()
{
Db *x;

x = new Db(NULL, 0);
// DIFFERENT VERSIONS OF BERKELEY HAVE DIFFERENT ARGUMMENTS

x->open(NULL, NULL, DB_BTREE, DB_CREATE, 0);
//x->open(NULL,NULL, NULL, DB_BTREE, DB_CREATE, 0);
return 0;
}
