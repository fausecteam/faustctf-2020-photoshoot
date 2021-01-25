#if NO_DB
#else

#include <pqxx/pqxx>
#include <string>

using DBCon = pqxx::connection;

int queryInt(char * query);
const char * queryString(char * query);

#endif
