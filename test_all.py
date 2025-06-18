import http.client
import os

def print_response(resp, name):
	print(f"\n=== {name} ===")
	print(f"Status: {resp.status} {resp.reason}")
	print("Headers:")
	for k, v in resp.getheaders():
		print(f"  {k}: {v}")
	body = resp.read().decode(errors="replace")
	print("Body:")
	print(body[:500] + ("..." if len(body) > 500 else ""))
	print("="*40)

def test_get(conn, path, name):
	conn.request("GET", path)
	resp = conn.getresponse()
	print_response(resp, name)

def test_post(conn, path, data, name):
	conn.request("POST", path, body=data, headers={"Content-Type": "text/plain"})
	resp = conn.getresponse()
	print_response(resp, name)

def test_delete(conn, path, name):
	conn.request("DELETE", path)
	resp = conn.getresponse()
	print_response(resp, name)

def test_put(conn, path, data, name):
	conn.request("PUT", path, body=data, headers={"Content-Type": "text/plain"})
	resp = conn.getresponse()
	print_response(resp, name)

def main():
	conn = http.client.HTTPConnection("localhost", 8080, timeout=5)

	test_get(conn, "/", "GET / (index.html)")
	test_get(conn, "/notfound.html", "GET /notfound.html (404)")
	test_get(conn, "/testdir/", "GET /testdir/ (autoindex)")

	test_post(conn, "/", "Hello World", "POST / (upload)")
	test_delete(conn, "/upload.data", "DELETE /upload.data")
	test_put(conn, "/", "test", "PUT / (405)")

	# Test body troppo grande
	test_post(conn, "/", "A" * 1000001, "POST / (413 Payload Too Large)")

	# Test CGI PHP
	test_get(conn, "/cgi-bin/test.php", "GET /cgi-bin/test.php (CGI PHP)")

	# Test CGI Python
	test_get(conn, "/cgi-bin/test.py", "GET /cgi-bin/test.py (CGI Python)")

	# Test redirezione (se configurata)
	test_get(conn, "/old", "GET /old (redirect)")

	conn.close()

if __name__ == "__main__":
	main()