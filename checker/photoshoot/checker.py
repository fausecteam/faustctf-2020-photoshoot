#!/usr/bin/env python3

import logging
import socket
import requests
import random
import subprocess
import tempfile
import os


from utils import generate_message

from ctf_gameserver import checkerlib

class PicdumpChecker(checkerlib.BaseChecker):

	def place_flag(self, tick):
		self.currentTick = tick

		flag = checkerlib.get_flag(tick)

		logging.info('Sending Flag %s', flag)
		key, error = self.storeData(flag)
		if error != None:
			return error

		logging.info('Flag gets identifier: %s', key)
		checkerlib.store_state("{}:{}".format(self.ip, tick), {"flag": flag, "id": key})

		return checkerlib.CheckResult.OK

	def check_flag(self, tick):
		flag = checkerlib.get_flag(tick)

		flag = checkerlib.load_state("{}:{}".format(self.ip, tick))
		if flag == None:
			logging.error('Flag not existent in local storage')
			return checkerlib.CheckResult.FLAG_NOT_FOUND
			# assert flag != None # TODO internal error

		logging.info('Retrieving flag with id {}'.format(flag["id"]))
		value, error = self.loadData(flag["id"])
		if error != None:
			return error
		
		if "data not found" in value:
			logging.warning('Flag not found')
			return checkerlib.CheckResult.FLAG_NOT_FOUND
		if value != flag["flag"]:
			logging.warning('Found data, but flag value is invalid. Expected %s but found %s', flag["flag"], value)
			return checkerlib.CheckResult.FAULTY

		return checkerlib.CheckResult.OK

	def check_service(self):
		base = os.path.dirname(os.path.realpath(__file__))
		# TODO: need to check store/load? checkflag needs this ..

		logging.info("Checking basic store/load")
		v = generate_message()[:32]
		a, e = self.storeData(v)
		if e != None:
			return e
		v2, e = self.loadData(a)
		if e != None:
			return e
		if v != v2:
			return checkerlib.CheckResult.FAULTY

		# generate local image and expected result
		opMax = 6
		fts = [5]

		opSeq = []

		opCount = self.currentTick % 4
		random.seed(self.currentTick)
		
		for _ in range(opCount):
			opSeq += [str(random.randint(0, opMax))] # TODO: pseudorandomize
		
		ft = fts[self.currentTick % len(fts)]


		try:
			if True: # uncomment im ./imager is avaible # TODO
				alt = self.genImage()
				inFD, inFile = tempfile.mkstemp()
				outFD, outFile = tempfile.mkstemp()

				os.close(inFD)
				os.close(outFD)

				with open(inFile, "wb") as img:
					img.write(alt)

				cmd = [
					"{}/imager".format(base),
					str(opCount)
				] + opSeq + [
					inFile,
					outFile
				]
				logging.info("Running %s", str(cmd))
				subprocess.check_output(cmd, cwd=base)
				with open(outFile, "rb") as img:
					neu = img.read()
			else:
				with open("{}/imgs/test.ppm".format(base), "rb") as img:
					alt = img.read()

				with open("{}/imgs/test_op{}.ppm".format(base, op), "rb") as img:
					neu = img.read()
		except Exception as e:
			logging.error("Failed image manipulation: %s", str(e))
			raise e
		finally:
			if True: # uncomment im ./imager is avaible # TODO
				os.unlink(inFile)
				os.unlink(outFile)

		# check service
		logging.info("Asking team to manipulate image with operation %s", str(opSeq))
		r = requests.post("http://[{}]:{}/upload".format(self.ip, 8574), data={"operation": " ".join([str(opCount)] + opSeq)}, files={"file":alt})
		
		if r.status_code != 200:
			logging.warning('Unexpected Statuscode: %d', r.status_code)
			return checkerlib.CheckResult.FAULTY
		if r.content != neu:
			logging.warning('Unexpected Image result for operation %s.\nExp: %s\nGot: %s', str(opSeq), str(neu[:100]), str(r.content[:100]))
			return checkerlib.CheckResult.FAULTY
		
		# check if data is still persistent
		logging.info("Second retrieve of fake data")
		v2, e = self.loadData(a)
		if e != None:
			return e
		if v != v2:
			return checkerlib.CheckResult.FAULTY
		
		return checkerlib.CheckResult.OK

	def storeData(self, data):
		r = requests.post("http://[{}]:{}/store".format(self.ip, 8574), data={"value": data})

		if r.status_code != 200:
			logging.warning('Unexpected Statuscode %d while storing %s', r.status_code, data)
			return None, checkerlib.CheckResult.FAULTY
		if len(r.text) != 32:
			logging.warning('Invalid UUID: %s', r.text)
			return None, checkerlib.CheckResult.FAULTY
		return r.text, None

	def loadData(self, key):
		r = requests.post("http://[{}]:{}/load".format(self.ip, 8574), data={"key": key})

		if r.status_code != 200:
			logging.warning('Unexpected Statuscode %d while loading from key %s', r.status_code, key)
			return None, checkerlib.CheckResult.FAULTY
		return r.text, None

	def genImage(self, ft = 5):
		if ft == 5:
			res = b"P5\n200\n200\n255\n"
			res += bytes([random.randint(0,255) for _ in range(200 * 200)])
		else:
			return None
		return res

if __name__ == '__main__':

	checkerlib.run_check(PicdumpChecker)

