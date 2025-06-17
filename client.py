import requests
import threading
import time

# Parametri
URL = "http://localhost:8080"
NUM_UTENTI = 100
DURATA_CONN_SECONDI = 30

def utente_virtuale(id):
    print(f"[Utente {id}] Connettendo a {URL}...")
    try:
        # Keep-alive è abilitato di default in requests
        with requests.Session() as session:
            response = session.get(URL, stream=True)
            print(f"[Utente {id}] Connessione aperta. Status code: {response.status_code}")
            time.sleep(DURATA_CONN_SECONDI)
            print(f"[Utente {id}] Disconnessione dopo {DURATA_CONN_SECONDI} secondi.")
    except Exception as e:
        print(f"[Utente {id}] Errore: {e}")

# Lancia i thread
threads = []
for i in range(NUM_UTENTI):
    t = threading.Thread(target=utente_virtuale, args=(i+1,))
    threads.append(t)
    t.start()
    time.sleep(0.2)  # intervallo per simulare utenti che si connettono uno dopo l’altro

# Attendi la fine di tutti
for t in threads:
    t.join()

print("✅ Test completato.")

