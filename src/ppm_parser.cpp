#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <filesystem>
#include <unistd.h>
#include "ppm_parser.h"

#define MAXSIZE 5000
using namespace std;

namespace fs = filesystem;
Buffer getFileContent(const std::string& path) {
	fs::path p = path;
	int size = fs::file_size(p);
	cout << "Opening " << path.c_str() << endl;
	int x = open(path.c_str(), O_RDONLY);
	cout << "FD = " << x << endl;
	void * addr = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, x, 0);
	close(x);
	if (addr == MAP_FAILED) {
		perror("mmap");
		return {nullptr, 0};
	}
	Buffer b{(char*)malloc(size), size};
	for (int i = 0; i < size; i++) {
		b.buf[i] = ((char*)addr)[i];
		//((char*)addr)[i] = 0;
	}
	munmap(addr, size);
	return b;
//	return string((char*) addr, 120100);
}

bool isEmpty(char c) {
	return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

void printOK() { cout << "Everything is OK" << endl; }
void printINVALID() { cerr << "Invalid P type" << endl; }
void printInvalidBits() { cerr << "Invalid bit value" << endl; }
void printInvalidPixelcount() { cerr << "Pixelcount does not match header" << endl; }
void printInvalidSize() { cerr << "invalid or unsupported size" << endl; }
void printTODO() { cerr << "To be implemented" << endl; }
void printUnexpected() { cerr << "Unexpected character" << endl; }
void printUnexpectedEnd() { cerr << "File ended unexpectedly" << endl; }
void printUnexpectedMagic() { cerr << "Unexpected Magic value" << endl; }
void printSizeUnwanted() { cerr << "I only accept 200x200 images" << endl; }
void printMoreStuff() { cerr << "Found additional bytes at the end" << endl; }
void printMaxsize() { cerr << "Max value for pixels should be 255" << endl; }
void printNone() {  }

// only supports up to 3 digits
int parseInt(const Buffer s, int & cur) {
    if (cur >= s.size) { printUnexpectedEnd(); return -1; }
    int ze = cur;
    while (ze < s.size && s.buf[ze] >= '0' && s.buf[ze] <= '9') ze++;
	if (ze == cur) { printUnexpectedEnd(); return -1; }
	if (cur + 3  < ze) { printInvalidSize(); return -1; }
	int val = atoi(string(&s.buf[cur], &s.buf[ze]).c_str());
	cur = ze;
	return val;
}

void clear(Buffer s) {
    for (int i = 0; i < s.size; i++) {
        s.buf[i] = 0;
    }
}

uint8_t* handleHeader(Buffer s) {
	int cur = 0;
	if (cur >= s.size) {printUnexpectedEnd(); return nullptr;}
	if (s.buf[cur] != 'P') { printUnexpectedMagic(); return nullptr;}
	cur++;
	if (cur >= s.size) { printUnexpectedEnd(); return nullptr;}
	int ft = s.buf[cur] - '0';
	if (ft < 0 || ft > 9) { printINVALID(); return nullptr;}
	cur++;
	if (cur >= s.size) { printUnexpectedEnd(); return nullptr;}
	if (s.buf[cur] != '\n') { printUnexpected(); return nullptr;}
	cur++; // now starts the size
	int ze = cur;

	while (ze < s.size && s.buf[ze] >= '0' && s.buf[ze] <= '9') ze++;
	if (ze == cur) { printUnexpectedEnd(); return nullptr;}
	if (cur + 3  < ze) { printInvalidSize(); return nullptr;}
	int width = atoi(string(&s.buf[cur], &s.buf[ze]).c_str());
	cur = ze;

	// space separating dimensions
	if (cur >= s.size) { printUnexpectedEnd(); return nullptr;}
	if (!isEmpty(s.buf[cur])) { printInvalidSize(); return nullptr;}
	/*while (cur < s.size && isEmpty(s.buf[cur])) */ // unkomment this to allow arbitrary whitespacing, but this allows to store some information
	cur++;

	//height
	ze = cur;
	while (ze < s.size && s.buf[ze] >= '0' && s.buf[ze] <= '9') ze++;
	if (ze == cur) { printUnexpectedEnd(); return nullptr;}
	if (cur + 3  < ze) { printInvalidSize(); return nullptr;}
	int height = atoi(string(&s.buf[cur], &s.buf[ze]).c_str());
	cur = ze;
	//if (height * width > MAXSIZE) return printInvalidSize();
	if (height != 200 || width != 200) { printSizeUnwanted(); return nullptr;}

    // space after dimensions
	if (cur >= s.size) { printUnexpectedEnd(); return nullptr;}
	if (!isEmpty(s.buf[cur])) { printInvalidSize(); return nullptr;}
	cur++;

	vector<func2> tbl = {handleP0, handleP1, handleP2, handleP3, handleP4, handleP5, handleP6, handleP7, handleP8, handleP9};
	//uint8_t * buffer = (uint8_t*) malloc(200 * 200);
	cout << "Opening anonymous " << endl;
	uint8_t * buffer = (uint8_t*) mmap(nullptr, 200 * 200, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if (buffer == MAP_FAILED) {
		perror("mmap");
		return nullptr;
	}
	/*if (buffer != (void*)0xBEEFBEEF) {
		cout << "Wrong address" << endl;
		return nullptr;
	}*/
	if (buffer == nullptr) return nullptr; // TODO: fehlermeldung
	bool a = tbl[ft](s, cur, buffer, width * height); // call the handler
	clear(s); // overwrite old data
	if (a) return buffer;
	return nullptr;
}

bool handleP0(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printINVALID();
	return false;
}
// Portable Bitmap ASCII
// 1 0 0 1 1 0 1 0 ...
bool handleP1(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	for (int i = 0; i < pxscount; i++) {
	    // 1 whitespace
	    if (cur >= s.size) { printUnexpectedEnd(); return false; }
	    if (!isEmpty(s.buf[cur])) { printUnexpected(); return false; }
	    cur++;
	    if (cur >= s.size) { printUnexpectedEnd(); return false; }
		if (s.buf[cur] == '1') {
		    buf[i] = 255;
		} else if (s.buf[cur] == '0') {
		    buf[i] = 0;
		} else {
		    buf[i] = 0;
		    printInvalidBits();
		    return false;
		}
		cur++;
	}
	if (cur < s.size && isEmpty(s.buf[cur])) cur++;

	return true;
}

bool handleP2(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printTODO();
	return false;
}
bool handleP3(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printTODO();
	return false;
}
bool handleP4(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printTODO();
	return false;
}
// Portable Graymap Binary
// <maxVal> <binaryBlob>
// 1 byte = 1 gray value pixel
bool handleP5(const Buffer s, int cur,uint8_t * buf, int pxscount) {
    // space separating dimensions
    int maxSize = parseInt(s, cur);
    if (maxSize == -1) { printNone(); return false; }
    if (maxSize != 255) { printMaxsize(); return false; }

    // separator whitespace
    if (cur >= s.size) { printUnexpectedEnd(); return false; }
    if (!isEmpty(s.buf[cur])) { printUnexpected(); return false; }
    cur++;

    // blob
    if (cur + pxscount > s.size) { printUnexpectedEnd(); return false; }
    for (int i = 0; i < pxscount; i++) buf[i] = s.buf[cur+i]; // copy blob
	return true;
}
// Portable Pixmap Binary
// <maxVal> <binaryBlob>
// 3 byte = 1 RGB values pixel
bool handleP6(const Buffer s, int cur,uint8_t * buf, int pxscount) {
    // space separating dimensions
    int maxSize = parseInt(s, cur);
    if (maxSize == -1) { printNone; return false; }
    if (maxSize != 255) { printMaxsize; return false; }

    // separator whitespace
    if (cur >= s.size) { printUnexpectedEnd(); return false; }
    if (!isEmpty(s.buf[cur])) { printUnexpected(); return false; }
    cur++;

    // blob
    if (cur + pxscount * 3 > s.size) { printUnexpectedEnd(); return false; }
    for (int i = 0; i < pxscount; i++) buf[i] = s.buf[cur+i*3]; // copy blob
	return true;
}
bool handleP7(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printTODO();
	return false;
}
bool handleP8(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printINVALID();
	return false;
}
bool handleP9(const Buffer s, int cur,uint8_t * buf, int pxscount) {
	printINVALID();
	return false;
}
