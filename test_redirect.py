#!/usr/bin/env python3

import requests

def test_redirect(url, allow_redirects, headers=None, desc=""):
	try:
		resp = requests.get(url, allow_redirects=allow_redirects, headers=headers or {}, timeout=5)
		print(f"\n=== {desc} ===")
		print(f"URL: {url}")
		print(f"Allow redirects: {allow_redirects}")
		print(f"Status: {resp.status_code}")
		print(f"Location header: {resp.headers.get('Location')}")
		print(f"Final URL: {resp.url}")
		print(f"History: {[r.status_code for r in resp.history]}")
		print(f"Body (first 200 chars):\n{resp.text[:200]}")
		print("="*40)
	except Exception as e:
		print(f"ERROR: {e}")

def main():
	base = "http://localhost:8080"
	tests = [
		# Redirect relativa
		{"path": "/old", "desc": "Redirect relativa, no follow", "allow_redirects": False},
		{"path": "/old", "desc": "Redirect relativa, follow", "allow_redirects": True},
		# Redirect assoluta (se il server la supporta, modifica la config per testarla)
		# {"path": "/old_abs", "desc": "Redirect assoluta, no follow", "allow_redirects": False},
		# {"path": "/old_abs", "desc": "Redirect assoluta, follow", "allow_redirects": True},
		# Con Host custom
		{"path": "/old", "desc": "Redirect relativa, Host: localhost", "allow_redirects": False, "headers": {"Host": "localhost"}},
		{"path": "/old", "desc": "Redirect relativa, Host: example.com", "allow_redirects": False, "headers": {"Host": "example.com"}},
	]

	for t in tests:
		url = base + t["path"]
		test_redirect(url, t["allow_redirects"], t.get("headers"), t["desc"])

if __name__ == "__main__":
	main()