import subprocess
import time
import os

def run_server_with_config(config_path):
	try:
		proc = subprocess.Popen(
			["./webserv", config_path],
			stdout=subprocess.PIPE,
			stderr=subprocess.PIPE
		)
		# Attendi un attimo che il server provi a partire
		time.sleep(1)
		out, err = proc.communicate(timeout=2)
		return out.decode(), err.decode(), proc.returncode
	except subprocess.TimeoutExpired:
		proc.kill()
		out, err = proc.communicate()
		return out.decode(), err.decode(), proc.returncode

def test_missing_config():
	print("Test: Config file mancante")
	out, err, code = run_server_with_config("non_esiste.conf")
	print("STDERR:", err.strip())
	assert "Error opening config file" in err

def test_invalid_directive():
	print("Test: Config con direttiva errata")
	with open("invalid.conf", "w") as f:
		f.write("server {\ninvalid_directive on;\n}\n")
	out, err, code = run_server_with_config("invalid.conf")
	print("STDERR:", err.strip())
	assert "Invalid directive" in err
	os.remove("invalid.conf")

def test_port_already_in_use():
	print("Test: Porta già occupata")
	# Avvia un server sulla porta 8080
	proc1 = subprocess.Popen(["./webserv", "webserv.conf"])
	time.sleep(1)
	# Avvia un secondo server sulla stessa porta
	out, err, code = run_server_with_config("webserv.conf")
	print("STDERR:", err.strip())
	assert "bind failed" in err or "listen failed" in err
	proc1.terminate()
	proc1.wait()

if __name__ == "__main__":
	test_missing_config()
	print("-" * 40)
	test_invalid_directive()
	print("-" * 40)
	test_port_already_in_use()
	print("-" * 40)
	print("Tutti i test di gestione errori sono passati!")