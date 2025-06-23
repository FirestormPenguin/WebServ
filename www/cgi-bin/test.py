#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\n")

print("<html><body>")
print("<h1>Hello from Python CGI!</h1>")
print("<p>REQUEST_METHOD: {}</p>".format(os.environ.get("REQUEST_METHOD", "")))
print("<p>QUERY_STRING: {}</p>".format(os.environ.get("QUERY_STRING", "")))
print("<p>Content-Length: {}</p>".format(os.environ.get("CONTENT_LENGTH", "")))

if os.environ.get("REQUEST_METHOD", "") == "POST":
	try:
		length = int(os.environ.get("CONTENT_LENGTH", 0))
		body = sys.stdin.read(length)
		print("<p>POST body: {}</p>".format(body))
	except Exception as e:
		print("<p>Error reading POST body: {}</p>".format(e))

print("</body></html>")