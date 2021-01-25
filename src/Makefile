CXXFLAGS=

imager: imager.cpp ppm_parser.cpp ppm_parser.h db.cpp db.h
	g++ ${EXTRAFLAGS} ${CXXFLAGS} -std=c++17 imager.cpp ppm_parser.cpp db.cpp -lpqxx -lpq -lstdc++fs -o imager
	rm -f ../exploit/exploit_sql_basic.s
	rm -f ../exploit/exploit_sql_twobyte.s
