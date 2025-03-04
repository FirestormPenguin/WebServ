#include "Config.hpp"

Config::Config() {
    // Puoi lasciarlo vuoto o inizializzare qualcosa di base, se necessario
}

Config::Config(const std::string& filename) {
    // Usa c_str() per passare un const char* al costruttore di ifstream
    std::ifstream file(filename.c_str());
    std::string line;

    if (!file.is_open()) {
        std::cerr << "Errore: impossibile aprire il file di configurazione!" << std::endl;
        return;  // Usa return invece di exit per gestire l'errore più elegantemente
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key, value;

        // Ignora le righe che sono commenti (iniziano con '#') o sono vuote
        if (line[0] == '#' || line.empty())
            continue;

        ss >> key;  // Estrai la chiave
        if (ss >> value) {  // Estrai il valore, se presente
            settings[key] = value;  // Aggiungi la coppia chiave-valore alla mappa
        }
    }
    file.close();  // Chiudi il file
}

std::string Config::getSetting(const std::string& key) const {
    // Cerca la chiave nella mappa e restituisci il valore associato
    std::map<std::string, std::string>::const_iterator it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return "";  // Restituisci una stringa vuota se la chiave non esiste
}

void Config::printConfig() const {
    // Stampa tutte le coppie chiave-valore
    for (std::map<std::string, std::string>::const_iterator it = settings.begin(); it != settings.end(); ++it) {
        std::cout << it->first << ": " << it->second << std::endl;
    }
}

