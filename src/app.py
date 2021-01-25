#!/bin/python3

import os
from flask import Flask, request, send_file, render_template
import subprocess
import tempfile
import uuid
import psycopg2

UPLOAD_FOLDER = '/tmp/'

app = Flask(__name__)

app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 200 * 1024

connectionString = "dbname=photoshoot"

@app.route("/upload", methods = ['POST'])
def upload():
	if "operation" not in request.form:
		return "failed op missing"
	if "file" not in request.files:
		return "failed file missing"
	file = request.files["file"]
	if file.filename == "":
		return "failed name empty"

	operations = request.form['operation'].split(" ")
	for x in operations:
		if not x.isnumeric():
			return "failed: invalid op"

	fd, inFile = tempfile.mkstemp()
	fd, outFile = tempfile.mkstemp()
	try:
		file.save(os.path.join(app.config['UPLOAD_FOLDER'], inFile))
		if "year" in request.form:
			option = request.form['year']
			if not option.isnumeric():
				options = []
			elif option == "21":
				return "failed: not yet available"
			else:
				options = ["--config", option]
		else:
			options = []

		cmd = [
			"./imager"
			] + options + operations + [
			inFile, outFile
		]
		subprocess.check_output(cmd, stderr=subprocess.STDOUT)
		return send_file(os.path.join(app.config['UPLOAD_FOLDER'], outFile))
	except subprocess.CalledProcessError as e:
		return "failed: " + str(e.output)
	except Exception as e:
		return "failed: " + str(e)
	finally:
		os.unlink(inFile)
		os.unlink(outFile)

@app.route("/store", methods = ['POST'])
def store():
	if "value" not in request.form:
		return "failed: missing value"
	val = request.form["value"][:64]
	key = uuid.uuid4().hex
	con = psycopg2.connect(connectionString)
	cur = con.cursor()
	cur.execute("INSERT INTO comments VALUES (%s, %s)", (key, val))
	con.commit()
	cur.close()
	con.close()
	return key

@app.route("/load", methods = ['POST'])
def load():
	if "key" not in request.form:
		return "failed: missing key"
	key = request.form["key"]
	con = psycopg2.connect(connectionString)
	cur = con.cursor()
	cur.execute("SELECT value FROM comments WHERE key = %s;", (key,))
	res = cur.fetchone()
	if res == None:
		return "failed: data not found"
	val = res[0]
	cur.close()
	con.close()
	return val

@app.route("/")
def index():
	return render_template('index.html', title='Photoshooting')

if __name__ == "__main__":
	#app.run(host='0.0.0.0')
	app.run(host='::1', port=8574, debug=True)
