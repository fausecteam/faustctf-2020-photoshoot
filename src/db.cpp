#if NO_DB
#else

#include <bits/stdc++.h>

#include "db.h"

#include <pqxx/pqxx>

using namespace std;

string connectionString = "dbname=photoshoot";

int queryInt(char * query) {
	pqxx::connection c(connectionString);
	pqxx::work txn(c);
	pqxx::row r = txn.exec1(query);
    int bs = r[0].as<int>();
	return bs;
}

const char * queryString(char * query) {
	pqxx::connection c(connectionString);
	pqxx::work txn(c);
	pqxx::row r = txn.exec1(query);
    return r[0].c_str();
}

#endif
