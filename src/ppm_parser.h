#include <bits/stdc++.h>

using namespace std;

struct Buffer {
	char * buf;
	int size;
};

Buffer getFileContent(const std::string& path);

bool isEmpty(char c);

void printOK();
void printINVALID();
void printInvalidBits();
void printInvalidPixelcount();
void printInvalidSize();
void printTODO();
void printUnexpected();
void printUnexpectedEnd();
void printUnexpectedMagic();
void printSizeUnwanted();
void printMoreStuff();
void printMaxsize();

typedef void(*func)(void);
typedef bool(*func2)(const Buffer,int,uint8_t*,int);

func parseInt(const Buffer, int & cur, int & val);

void clear(Buffer s);

uint8_t* handleHeader(Buffer s);

bool handleP0(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP1(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP2(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP3(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP4(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP5(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP6(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP7(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP8(const Buffer s, int cur,uint8_t * buf, int pxscount);
bool handleP9(const Buffer s, int cur,uint8_t * buf, int pxscount);
