#!/usr/bin/env python3

import time

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Inizio loop infinito</h1>")
while True:
	time.sleep(1)
print("</body></html>")