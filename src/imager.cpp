#include <bits/stdc++.h>

#include "ppm_parser.h"
#include "db.h"

#define MAXSIZE 5000
using namespace std;

typedef bool(*operation)(uint8_t*);

const int w = 200, h = 200;

int year = 20;

string curDir;

void fail(uint8_t * img) {
	cout << "Failing" << endl;
}

void caller(operation call, uint8_t * data) {
	call(data);
}

int c2i(int r, int c) { return r * w + c; }

bool downsample(uint8_t * img) {
	for (int r = 0; r < 200 / 2; r++) {
		for (int c = 0; c < 200 / 2; c++) {
			int val = 0;
			val += img[c2i(2*r, 2*c)];
			val += img[c2i(2*r, 2*c+1)];
			val += img[c2i(2*r+1, 2*c)];
			val += img[c2i(2*r+1, 2*c+1)];
			val /= 4;
			img[c2i(2*r, 2*c)] = (uint8_t)val;
			img[c2i(2*r, 2*c+1)] = (uint8_t)val;
			img[c2i(2*r+1, 2*c)] = (uint8_t)val;
			img[c2i(2*r+1, 2*c+1)] = (uint8_t)val;
		}
	}
	return true;
}

bool todo(uint8_t * img) {
	cout << "Todoing" << endl;
	return true;
}

bool watermark(uint8_t * img) {
	Buffer content = getFileContent(curDir + string("watermark.ppm"));
	uint8_t * data = handleHeader(content);
	bool ok = true;
	if (data == nullptr) {
		cerr << "Failed reading watermark image" << endl;
		ok = false;
		return true;
	}
	for (int i = 0; i < w * h; i++) {
		if (!ok || data[i] == 0x00) {
			img[i] = 0x80;
		}
	}
	free(content.buf);
	//free(data);
	return true;
}

bool lineblur(uint8_t * img) { // unused. remove?
	uint8_t * np = (uint8_t*) malloc(w);
	for (int row = 0; row < h; row++) {
		for (int col = 0; col < w; col++) {
			int idx = row * w + col;
			int a = col==0?0:img[idx-1];
			int b = img[idx];
			int c = col==(w-1)?0:img[idx+1];
			np[col] = (a + b + c) / 3;
		}
		memcpy(&img[row * w], np, w);
	}
	free(np);
	return true;
}

#if NO_DB
int getBlurSize() { return 2; }
#else
int getBlurSize() {
	char buffer[120];
	snprintf(buffer, 80, "SELECT value FROM config%d WHERE key = 'blursize'", year);
	int x = queryInt(buffer);
	assert (x >= 1 && x <= 5); // TODO is this required?
	return x;
}
#endif

bool blur(uint8_t * img) {
	struct {
		int i, k, ring, row, col, x, y, idx, sum;
		int bs;// = getBlurSize();
		uint8_t * newImg;// = (uint8_t*) malloc(h * w);
		uint8_t tmp[20];
		bool fail = false;
		uint8_t nothing[10];
	} S;
	S.bs = getBlurSize();
	S.newImg = (uint8_t*) malloc(w * h);
	if (S.newImg == nullptr) S.fail = true;
	memset(S.newImg, 0, w * h);
//	for (int i = 0; i < w * h; i++) cout << img[i] << endl;
	for (S.row = 1; S.row < h - 1; S.row++) {
		for (S.col = 1; S.col < w - 1; S.col++) {
			S.k = 0;
			// ring 0
			S.idx = S.row * w + S.col;
			S.tmp[S.k++] = img[S.idx];
			for (S.ring = 1; S.ring < S.bs; S.ring++) {
				S.x = S.row - S.ring;
				S.y = S.col + S.ring;
				// down
				for (S.i = 0; S.i < S.ring * 2; S.i++) {
					S.x++;
					S.idx = S.x * w + S.y;
					if (S.idx < 200 * 200) { S.tmp[S.k++] = img[S.idx]; } else { S.tmp[S.k++] = 0; }
				}
				// left
				for (S.i = 0; S.i < S.ring * 2; S.i++) {
					S.y--;
					S.idx = S.x * w + S.y;
					if (S.idx < 200 * 200) { S.tmp[S.k++] = img[S.idx]; } else { S.tmp[S.k++] = 0; }
				}
				// up
				for (S.i = 0; S.i < S.ring * 2; S.i++) {
					S.x--;
					S.idx = S.x * w + S.y;
					if (S.idx < 200 * 200) { S.tmp[S.k++] = img[S.idx]; } else { S.tmp[S.k++] = 0; }
				}
				// right
				for (S.i = 0; S.i < S.ring * 2; S.i++) {
					S.y++;
					S.idx = S.x * w + S.y;
					if (S.idx < 200 * 200) { S.tmp[S.k++] = img[S.idx]; } else { S.tmp[S.k++] = 0; }
				}
			}
			S.sum = 0;
			for (S.i = 0; S.i < 9; S.i++) {
				S.sum += S.tmp[S.i];
			}
			S.newImg[S.row * w + S.col] = S.sum / 9;
		}
	}
	memcpy(img, S.newImg, w * h);
	memset(S.tmp, 0, 20);
	free(S.newImg);
	return !S.fail;
}

bool emboss(uint8_t * img) {
	uint8_t * newImg = (uint8_t*) malloc(h * w);
	for (int r = 0; r < h; r++) {
		for (int c = 0; c < w; c++) {
			int sum = 0;
			if (r > 0) sum += img[c2i(r-1, c)];
			if (r+1 < h) sum -= img[c2i(r+1, c)];
			sum /= 2;
			sum += 128;
			sum = max(0, min(255, sum)); // clamp
			newImg[c2i(r, c)] = sum;
		}
	}
	memcpy(img, newImg, w * h);
	free(newImg);
	return true;
}

bool tv(uint8_t * img) {
	for (int r = 0; r < h; r++) {
		int shift = abs((r % 7) - 3);
		if (shift < 0) {
			for (int c = w-1; c >= shift; c--) {
				img[c2i(r, c)] = img[c2i(r, c+shift)];
			}
		} else if (shift > 0) {
			for (int c = 0; c < w - shift; c++) {
				img[c2i(r, c)] = img[c2i(r, c+shift)];
			}
		}
	}
	return true;
}

bool nop(uint8_t * img) {
	return true;
}

struct Operation {
	string name;
	int id;
	operation function;
};

vector<Operation> ops = {
	{"nop", 0, nop},
	{"blur", 1, blur},
	{"downsample", 2, downsample},
	{"lineblur", 3, lineblur},
	{"watermark", 4, watermark}, // watermus MUST be 4 to enforce the attack using watermark
	{"emboss", 5, emboss},
	{"tv", 6, tv}
};

struct Task {
	uint64_t op[3];
	uint64_t n = 0;
	operation opFunc[3];
	uint8_t * data = nullptr;
	Task() {
		op[0] = op[1] = op[2] = 0;
		opFunc[0] = opFunc[1] = opFunc[2] = nullptr;
	}
};

void usage(string fail) {
	cout << "Usage: ImageEnhance <OpCount> [<Operation> ...] <InputFile> <OutputFile>" << endl;
	cout << "OpCount\tNumber of operations to perform, at most 3" << endl;
	cout << "Operation\tList of 'OpCount' operation IDs" << endl;
	cout << "Valid operations are:" << endl;
	for (auto & p : ops) {
		cout << "\t" << p.id << "  " << p.name << endl;
	}
	cout << "Failed: " << fail << endl;
}

void doIt(Task & t) {
	if (t.n > 4) { cout << "Don't overload the poor martian servers." << endl; return; }
	operation * o = &t.opFunc[0];
	int i = 0;
	int maxRuns = 8;
	while (i < t.n && maxRuns > 0) {
		maxRuns--;
		bool success = true;
		if (*o != nullptr) {
			success = (*o)(t.data);
		}
		if (success)
			i++;
			o++; // HINT: Bug, missing {}
	}
}

int main(int argc, char ** argv) {
	curDir = string(argv[0]);
	int pos = curDir.rfind("/");
	if (pos >= 0) {
		curDir.erase(pos+1);
	}
	// parse config year
	if (argc >= 3 && strncmp(argv[1], "--config", 8) == 0) {
		year = strtol(argv[2], nullptr, 0);
		argc -= 2;
		argv += 2;
	}

	if (argc < 4 || argc > 7) {
		usage("Need between 3 and 6 arguments"); return 1;
	}

	int opcount = strtol(argv[1], nullptr, 0);
	if (opcount < 0 or opcount > 4) { // (intended) BUG: opcount >= 4 should be the way to go ..
		usage("Need between 0 and 3 operations"); return 1;
	}

	Task t;
	t.n = opcount;
	int op = 0;
	char * endptr = nullptr;
	for (int i = 0; i < opcount; i++) {
		if (2 + i >= argc) { usage("Not enough arguments"); return 1; }
		int x = strtol(argv[2 + i], &endptr, 0);
		if (endptr[0] == '\0') {
			op = x; // valid integer string
		} // BUG stuff: if this is an invalid string, the last op is used (this is strange ..)
		if (op < 0 || op >= ops.size()) {
			usage("Opcode out of range"); return 1;
		}
		t.op[i] = op;
		t.opFunc[i] = ops[op].function;
	}

	Buffer content = getFileContent(argv[argc-2]);
	uint8_t * data = handleHeader(content);
	free(content.buf);

	if (data == nullptr) return 1;
	t.data = data;

	// perform operations
	doIt(t);

    std::ofstream file(argv[argc-1]);
    file << "P5\n200 200 255\n";
    for (int i = 0; i < 200 * 200; i++) {
        file << (char) t.data[i];
    }
    //free(data);
    return 0;
}
