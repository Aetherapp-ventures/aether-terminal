#include <iostream>
#include <string>
#include <windows.h>
#include <vector>
#include <sstream>
#include <map>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>
#include <random>
#include <conio.h>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <iomanip>
#include <regex>
#include <future>
#include <memory>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <optional>
#include <variant>
#include <any>
#include <charconv>
#include <bitset>
#include <numeric>

namespace fs = std::filesystem;

class ConsoleColor {
public:
    static void set(int textColor, int bgColor) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
    }
};

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const std::vector<std::string>& args) = 0;
    virtual std::string getHelp() const = 0;
};

class Terminal;

class HelpCommand : public Command {
private:
    const std::map<std::string, std::unique_ptr<Command>>& commands;
    std::map<std::string, std::string> commandDescriptions;

public:
    HelpCommand(const std::map<std::string, std::unique_ptr<Command>>& cmds) : commands(cmds) {
        initializeCommandDescriptions();
    }

    void execute(const std::vector<std::string>& args) override {
        if (args.empty()) {
            std::cout << "Verfügbare Befehle:\n";
            for (const auto& [name, description] : commandDescriptions) {
                std::cout << std::left << std::setw(15) << name << " - " << description << "\n";
            }
            std::cout << "\nFür detaillierte Informationen zu einem Befehl, geben Sie 'help <Befehlsname>' ein.\n";
        } else {
            const std::string& commandName = args[0];
            auto it = commandDescriptions.find(commandName);
            if (it != commandDescriptions.end()) {
                std::cout << commandName << " - " << it->second << "\n";
            } else {
                std::cout << "Kein Hilfetext verfügbar für den Befehl: " << commandName << "\n";
            }
        }
    }

    std::string getHelp() const override {
        return "Zeigt Hilfe zu verfügbaren Befehlen an";
    }

private:
    void initializeCommandDescriptions() {
        commandDescriptions = {
            {"help", "Zeigt diese Hilfemeldung an. Verwendung: help [Befehlsname]"},
            {"cls", "Löscht den Bildschirminhalt. Verwendung: cls"},
            {"echo", "Gibt einen Text aus. Verwendung: echo <Text>"},
            {"time", "Zeigt die aktuelle Systemzeit an. Verwendung: time"},
            {"calc", "Öffnet den Windows-Taschenrechner. Verwendung: calc"},
            {"history", "Zeigt den Befehlsverlauf an. Verwendung: history"},
            {"alias", "Zeigt alle definierten Aliase an. Verwendung: alias"},
            {"setalias", "Erstellt ein neues Alias. Verwendung: setalias <Aliasname> <Befehl>"},
            {"create", "Erstellt eine neue Datei. Verwendung: create <Dateiname>"},
            {"delete", "Löscht eine Datei. Verwendung: delete <Dateiname>"},
            {"list", "Listet Dateien im aktuellen Verzeichnis auf. Verwendung: list"},
            {"ping", "Sendet ICMP-Echo-Anforderungen an einen Host. Verwendung: ping <Hostname oder IP>"},
            {"random", "Generiert eine Zufallszahl zwischen 1 und 100. Verwendung: random"},
            {"sleep", "Pausiert die Ausführung für eine bestimmte Zeit. Verwendung: sleep <Sekunden>"},
            {"theme", "Ändert das Farbschema des Terminals. Verwendung: theme"},
            {"writefile", "Schreibt Text in eine Datei. Verwendung: writefile <Dateiname> <Text>"},
            {"readfile", "Liest den Inhalt einer Datei. Verwendung: readfile <Dateiname>"},
            {"encrypt", "Verschlüsselt eine Datei. Verwendung: encrypt <Eingabedatei> <Ausgabedatei>"},
            {"decrypt", "Entschlüsselt eine Datei. Verwendung: decrypt <Eingabedatei> <Ausgabedatei>"},
            {"compress", "Komprimiert eine Datei. Verwendung: compress <Eingabedatei> <Ausgabedatei>"},
            {"decompress", "Dekomprimiert eine Datei. Verwendung: decompress <Eingabedatei> <Ausgabedatei>"},
            {"search", "Sucht nach Dateien mit einem bestimmten Muster. Verwendung: search <Suchmuster>"},
            {"schedule", "Plant die Ausführung eines Befehls. Verwendung: schedule <Verzögerung in Sekunden> <Befehl>"},
            {"task", "Verwaltet Hintergrundaufgaben. Verwendung: task <start|stop|list> [Befehl]"},
            {"network", "Zeigt Netzwerkinformationen an. Verwendung: network"},
            {"sysinfo", "Zeigt Systeminformationen an. Verwendung: sysinfo"},
            {"weather", "Zeigt Wetterinformationen für eine Stadt an. Verwendung: weather <Stadt>"},
            {"math", "Führt einfache mathematische Operationen durch. Verwendung: math <Zahl1> <Operator> <Zahl2>"},
            {"sort", "Sortiert eine Liste von Elementen. Verwendung: sort <Element1> <Element2> ..."},
            {"base64", "Kodiert oder dekodiert Text in Base64. Verwendung: base64 <encode|decode> <Text>"},
            {"hash", "Berechnet den Hash-Wert eines Textes. Verwendung: hash <Text>"},
            {"edit", "Öffnet einen einfachen Texteditor. Verwendung: edit <dateiname>"},
            {"exit", "Beendet das Terminal. Verwendung: exit"}
        };
    }
};

class TaskManager {
public:
    void addTask(const std::function<void()>& task) {
        std::lock_guard<std::mutex> lock(mutex);
        tasks.push(task);
        condition.notify_one();
    }

    void run() {
        while (!shouldStop) {
            std::unique_lock<std::mutex> lock(mutex);
            condition.wait(lock, [this] { return !tasks.empty() || shouldStop; });

            if (shouldStop) break;

            auto task = tasks.front();
            tasks.pop();
            lock.unlock();

            task();
        }
    }

    void stop() {
        std::lock_guard<std::mutex> lock(mutex);
        shouldStop = true;
        condition.notify_all();
    }

private:
    std::queue<std::function<void()>> tasks;
    std::mutex mutex;
    std::condition_variable condition;
    std::atomic<bool> shouldStop{false};
};

class SimpleTextEditor {
private:
    std::vector<std::string> lines;
    std::string filename;

public:
    SimpleTextEditor(const std::string& fname) : filename(fname) {
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }
    }

    void display() {
        for (size_t i = 0; i < lines.size(); ++i) {
            std::cout << i + 1 << ": " << lines[i] << std::endl;
        }
    }

    void addLine(const std::string& line) {
        lines.push_back(line);
    }

    void editLine(size_t lineNumber, const std::string& newContent) {
        if (lineNumber > 0 && lineNumber <= lines.size()) {
            lines[lineNumber - 1] = newContent;
        }
    }

    void deleteLine(size_t lineNumber) {
        if (lineNumber > 0 && lineNumber <= lines.size()) {
            lines.erase(lines.begin() + lineNumber - 1);
        }
    }

    void save() {
        std::ofstream file(filename);
        for (const auto& line : lines) {
            file << line << std::endl;
        }
    }
};

class EditCommand : public Command {
private:
    Terminal& terminal;

public:
    EditCommand(Terminal& t) : terminal(t) {}

    void execute(const std::vector<std::string>& args) override;

    std::string getHelp() const override {
        return "Öffnet einen einfachen Texteditor. Verwendung: edit <dateiname>";
    }
};

class Terminal {
public:
    Terminal() : taskThread(&TaskManager::run, &taskManager) {}

    ~Terminal() {
        taskManager.stop();
        if (taskThread.joinable()) {
            taskThread.join();
        }
    }

    void run() {
        printWelcomeMessage();
        loadCommandHistory();
        loadAliases();
        loadThemes();
        registerCommands();

        std::string command;
        while (true) {
            ConsoleColor::set(currentTheme["promptTextColor"], currentTheme["promptBgColor"]);
            std::cout << "Aether > ";
            ConsoleColor::set(currentTheme["defaultTextColor"], currentTheme["defaultBgColor"]);

            command = getCommandInput();

            if (command.empty()) continue;

            if (command == "exit") {
                saveAliases();
                break;
            }

            addCommandToHistory(command);
            auto start = std::chrono::high_resolution_clock::now();
            executeCommand(command);
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Befehl in " << duration << " ms ausgeführt." << std::endl;
        }
    }

    void editFile(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: edit <dateiname>\n";
            return;
        }

        SimpleTextEditor editor(args[0]);
        std::string command;

        while (true) {
            editor.display();
            std::cout << "\nEditor-Befehle: add, edit <zeilennummer>, delete <zeilennummer>, save, quit\n";
            std::cout << "Befehl: ";
            std::getline(std::cin, command);

            std::istringstream iss(command);
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) {
                tokens.push_back(token);
            }

            if (tokens.empty()) continue;

            if (tokens[0] == "add") {
                std::cout << "Neue Zeile: ";
                std::string newLine;
                std::getline(std::cin, newLine);
                editor.addLine(newLine);
            } else if (tokens[0] == "edit" && tokens.size() > 1) {
                size_t lineNumber = std::stoul(tokens[1]);
                std::cout << "Neue Inhalte: ";
                std::string newContent;
                std::getline(std::cin, newContent);
                editor.editLine(lineNumber, newContent);
            } else if (tokens[0] == "delete" && tokens.size() > 1) {
                size_t lineNumber = std::stoul(tokens[1]);
                editor.deleteLine(lineNumber);
            } else if (tokens[0] == "save") {
                editor.save();
                std::cout << "Datei gespeichert.\n";
            } else if (tokens[0] == "quit") {
                break;
            } else {
                std::cout << "Unbekannter Befehl.\n";
            }
        }
    }

private:
    std::vector<std::string> commandHistory;
    std::map<std::string, std::string> aliases;
    std::map<std::string, int> currentTheme;
    std::map<std::string, std::unique_ptr<Command>> commands;
    int historyIndex = -1;
    TaskManager taskManager;
    std::thread taskThread;

    void registerCommands() {
        commands["help"] = std::make_unique<HelpCommand>(commands);
        commands["edit"] = std::make_unique<EditCommand>(*this);
    }

    void printWelcomeMessage() {
        ConsoleColor::set(9, 0);
        std::cout << R"(
         ___           _       _        
        / _ \         | |     | |       
       / /_\ \_ __ __| |_   _| | __ ___ 
       |  _  | '_ \ / _` | | | | |/ _` |
       | | | | | | | (_| | |_| | | (_| |
       \_| |_/_| |_|\__,_|\__,_|_|\__,_|
                                         
        )" << std::endl;
        ConsoleColor::set(7, 0);
        std::cout << "Willkommen im fortgeschrittenen Aether Terminal.\n";
        std::cout << "(C) 2024 CaydenDev. Lizenziert unter der MIT-Lizenz.\n";
        std::cout << "Geben Sie 'help' ein, um eine Liste der Befehle anzuzeigen.\n" << std::endl;
    }

    void loadCommandHistory() {
        std::ifstream historyFile("command_history.txt");
        std::string line;
        while (std::getline(historyFile, line)) {
            commandHistory.push_back(line);
        }
    }

    void saveCommandHistory() {
        std::ofstream historyFile("command_history.txt");
        for (const auto& cmd : commandHistory) {
            historyFile << cmd << std::endl;
        }
    }

    void loadAliases() {
        std::ifstream aliasFile("aliases.txt");
        std::string line;
        while (std::getline(aliasFile, line)) {
            size_t pos = line.find("=");
            if  (pos != std::string::npos) {
                std::string alias = line.substr(0, pos);
                std::string command = line.substr(pos + 1);
                aliases[alias] = command;
            }
        }
    }

    void saveAliases() {
        std::ofstream aliasFile("aliases.txt");
        for (const auto& pair : aliases) {
            aliasFile << pair.first << "=" << pair.second << std::endl;
        }
    }

    void loadThemes() {
        std::ifstream themeFile("themes.txt");
        if (themeFile.is_open()) {
            std::string themeName;
            themeFile >> themeName;
            for (auto& [key, value] : currentTheme) {
                themeFile >> value;
            }
        } else {
            currentTheme = {
                {"promptTextColor", 10},
                {"promptBgColor", 0},
                {"defaultTextColor", 7},
                {"defaultBgColor", 0}
            };
        }
    }

    void saveThemes() {
        std::ofstream themeFile("themes.txt");
        themeFile << "current_theme" << std::endl;
        for (const auto& [key, value] : currentTheme) {
            themeFile << value << std::endl;
        }
    }

    std::string getCommandInput() {
        std::string command;
        char ch;
        while ((ch = _getch()) != '\r') {
            if (ch == '\b') {
                if (!command.empty()) {
                    command.pop_back();
                    std::cout << "\b \b";
                }
            } else if (ch == -32) {
                char arrowKey = _getch();
                if (arrowKey == 72 && historyIndex < (int)commandHistory.size() - 1) {
                    historyIndex++;
                    command = commandHistory[commandHistory.size() - 1 - historyIndex];
                    std::cout << "\rAether > " << command << std::string(50, ' ');
                    std::cout << "\rAether > " << command;
                } else if (arrowKey == 80 && historyIndex >= 0) {
                    historyIndex--;
                    command = (historyIndex == -1) ? "" : commandHistory[commandHistory.size() - 1 - historyIndex];
                    std::cout << "\rAether > " << command << std::string(50, ' ');
                    std::cout << "\rAether > " << command;
                }
            } else {
                command.push_back(ch);
                std::cout << ch;
            }
        }
        std::cout << std::endl;
        return command;
    }

    void addCommandToHistory(const std::string& command) {
        commandHistory.push_back(command);
        historyIndex = -1;
        saveCommandHistory();
    }

    void executeCommand(const std::string& command) {
        std::vector<std::string> args;
        std::istringstream iss(command);
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }

        if (args.empty()) return;

        std::string cmd = args[0];
        args.erase(args.begin());

        if (aliases.find(cmd) != aliases.end()) {
            cmd = aliases[cmd];
        }

        if (commands.find(cmd) != commands.end()) {
            commands[cmd]->execute(args);
        } else if (cmd == "cls") {
            system("cls");
        } else if (cmd == "echo") {
            std::cout << command.substr(5) << std::endl;
        } else if (cmd == "time") {
            displayCurrentTime();
        } else if (cmd == "calc") {
            system("start calc");
        } else if (cmd == "history") {
            printHistory();
        } else if (cmd == "alias") {
            printAliases();
        } else if (cmd == "setalias") {
            setAlias(args);
        } else if (cmd == "create") {
            createFile(args);
        } else if (cmd == "delete") {
            deleteFile(args);
        } else if (cmd == "list") {
            listFiles();
        } else if (cmd == "ping") {
            pingHost(args);
        } else if (cmd == "random") {
            generateRandomNumber();
        } else if (cmd == "sleep") {
            sleepForSeconds(args);
        } else if (cmd == "theme") {
            switchTheme();
        } else if (cmd == "writefile") {
            writeToFile(args);
        } else if (cmd == "readfile") {
            readFromFile(args);
        } else if (cmd == "encrypt") {
            encryptFile(args);
        } else if (cmd == "decrypt") {
            decryptFile(args);
        } else if (cmd == "compress") {
            compressFile(args);
        } else if (cmd == "decompress") {
            decompressFile(args);
        } else if (cmd == "search") {
            searchFiles(args);
        } else if (cmd == "schedule") {
            scheduleCommand(args);
        } else if (cmd == "task") {
            manageTask(args);
        } else if (cmd == "network") {
            showNetworkInfo();
        } else if (cmd == "sysinfo") {
            showSystemInfo();
        } else if (cmd == "weather") {
            showWeather(args);
        } else if (cmd == "math") {
            performMathOperation(args);
        } else if (cmd == "sort") {
            sortItems(args);
        } else if (cmd == "base64") {
            base64Operation(args);
        } else if (cmd == "hash") {
            hashString(args);
        } else {
            std::cout << "Unbekannter Befehl: " << cmd << std::endl;
        }
    }

    void displayCurrentTime() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        std::cout << "Aktuelle Zeit: " << ss.str() << std::endl;
    }

    void printHistory() {
        std::cout << "Befehlsverlauf:\n";
        for (const auto& cmd : commandHistory) {
            std::cout << " - " << cmd << std::endl;
        }
    }

    void printAliases() {
        std::cout << "Aliase:\n";
        for (const auto& pair : aliases) {
            std::cout << " - " << pair.first << " -> " << pair.second << std::endl;
        }
    }

    void setAlias(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: setalias <alias> <befehl>\n";
            return;
        }
        aliases[args[0]] = args[1];
        std::cout << "Alias gesetzt: " << args[0] << " -> " << args[1] << std::endl;
        saveAliases();
    }

    void createFile(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: create <dateiname>\n";
            return;
        }
        std::ofstream outFile(args[0]);
        if (outFile) {
            std::cout << "Datei '" << args[0] << "' erstellt.\n";
        } else {
            std::cerr << "Fehler: Datei '" << args[0] << "' konnte nicht erstellt werden.\n";
        }
    }

    void deleteFile(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: delete <dateiname>\n";
            return;
        }
        if (fs::remove(args[0])) {
            std::cout << "Datei '" << args[0] << "' gelöscht.\n";
        } else {
            std::cerr << "Fehler: Datei '" << args[0] << "' konnte nicht gelöscht werden.\n";
        }
    }

    void listFiles() {
        std::cout << "Dateien im aktuellen Verzeichnis:\n";
        for (const auto& entry : fs::directory_iterator(fs::current_path())) {
            std::cout << " - " << entry.path().filename().string() << std::endl;
        }
    }

    void pingHost(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: ping <host>\n";
            return;
        }
        std::string command = "ping " + args[0];
        system(command.c_str());
    }

    void generateRandomNumber() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1, 100);
        std::cout << "Zufallszahl: " << distr(gen) << std::endl;
    }

    void sleepForSeconds(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: sleep <sekunden>\n";
            return;
        }
        int seconds = std::stoi(args[0]);
        std::cout << "Schlafe für " << seconds << " Sekunden...\n";
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        std::cout << "Aufgewacht!\n";
    }

    void switchTheme() {
        std::cout << "Wählen Sie ein Theme: (1) Dunkel, (2) Hell, (3) Benutzerdefiniert\n";
        std::string choice;
        std::getline(std::cin, choice);
        if (choice == "1") {
            currentTheme["promptTextColor"] = 10;
            currentTheme["promptBgColor"] = 0;
            currentTheme["defaultTextColor"] = 7;
            currentTheme["defaultBgColor"] = 0;
            std::cout << "Zu dunklem Theme gewechselt.\n";
        } else if (choice == "2") {
            currentTheme["promptTextColor"] = 0;
            currentTheme["promptBgColor"] = 7;
            currentTheme["defaultTextColor"] = 0;
            currentTheme["defaultBgColor"] = 7;
            std::cout << "Zu hellem Theme gewechselt.\n";
        } else if (choice == "3") {
            std::cout << "Geben Sie Farben für Prompt-Text, Prompt-Hintergrund, Standard-Text und Standard-Hintergrund ein (0-15):\n";
            std::cin >> currentTheme["promptTextColor"] >> currentTheme["promptBgColor"]
                     >> currentTheme["defaultTextColor"] >> currentTheme["defaultBgColor"];
            std::cout << "Benutzerdefiniertes Theme angewendet.\n";
        }
        saveThemes();
    }

    void writeToFile(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Verwendung: writefile <dateiname> <text>\n";
            return;
        }
        std::ofstream outFile(args[0]);
        if (outFile.is_open()) {
            for (size_t i = 1; i < args.size(); ++i) {
                outFile << args[i] << " ";
            }
            std::cout << "Text in Datei '" << args[0] << "' geschrieben.\n";
        } else {
            std::cerr << "Fehler: Konnte nicht in Datei '" << args[0] << "' schreiben.\n";
        }
    }

    void readFromFile(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: readfile <dateiname>\n";
            return;
        }
        std::ifstream inFile(args[0]);
        if (inFile.is_open()) {
            std::string line;
            while (std::getline(inFile, line)) {
                std::cout << line << std::endl;
            }
            inFile.close();
        } else {
            std::cerr << "Fehler: Konnte Datei '" << args[0] << "' nicht lesen.\n";
        }
    }

    void encryptFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: encrypt <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht öffnen.\n";
            return;
        }
        char ch;
        while (inFile.get(ch)) {
            ch = ch ^ 0x5A;
            outFile.put(ch);
        }
        std::cout << "Datei erfolgreich verschlüsselt.\n";
    }

    void decryptFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: decrypt <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht öffnen.\n";
            return;
        }
        char ch;
        while (inFile.get(ch)) {
            ch = ch ^ 0x5A;
            outFile.put(ch);
        }
        std::cout << "Datei erfolgreich entschlüsselt.\n";
    }

    void compressFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: compress <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht öffnen.\n";
            return;
        }
        
        char current;
        char count = 0;
        while (inFile.get(current)) {
            count = 1;
            char next;
            while (inFile.get(next) && next == current && count < 255) {
                count++;
            }
            outFile.put(count);
            outFile.put(current);
            if (next != current) {
                inFile.putback(next);
            }
        }
        std::cout << "Datei erfolgreich komprimiert.\n";
    }

    void decompressFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: decompress <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht öffnen.\n";
            return;
        }
        
        char count, ch;
        while (inFile.get(count) && inFile.get(ch)) {
            for (int i = 0; i < count; i++) {
                outFile.put(ch);
            }
        }
        std::cout << "Datei erfolgreich dekomprimiert.\n";
    }

    void searchFiles(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: search <muster>\n";
            return;
        }
        std::regex pattern(args[0]);
        for (const auto& entry : fs::recursive_directory_iterator(fs::current_path())) {
            if (std::regex_search(entry.path().string(), pattern)) {
                std::cout << entry.path().string() << std::endl;
            }
        }
    }

    void scheduleCommand(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Verwendung: schedule <verzögerung_in_sekunden> <befehl>\n";
            return;
        }
        int delay = std::stoi(args[0]);
        std::string cmd;
        for (size_t i = 1; i < args.size(); ++i) {
            cmd += args[i] + " ";
        }
        taskManager.addTask([this, delay, cmd]() {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            executeCommand(cmd);
        });
        std::cout << "Befehl geplant, wird in " << delay << " Sekunden ausgeführt.\n";
    }

    void manageTask(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: task <start|stop|list> [Befehl]\n";
            return;
        }

        if (args[0] == "start" && args.size() > 1) {
            std::string taskCommand = args[1];
            taskManager.addTask([this, taskCommand]() {
                executeCommand(taskCommand);
            });
            std::cout << "Hintergrundaufgabe gestartet: " << taskCommand << std::endl;
        } else if (args[0] == "list") {
            std::cout << "Hintergrundaufgaben-Verwaltung noch nicht vollständig implementiert.\n";
        } else {
            std::cout << "Ungültige Task-Operation.\n";
        }
    }

    void showNetworkInfo() {
        system("ipconfig");
    }

    void showSystemInfo() {
        system("systeminfo");
    }

    void showWeather(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: weather <stadt>\n";
            return;
        }
        std::string city = args[0];
        std::cout << "Wetterinformationen für " << city << " werden abgerufen...\n";
        std::cout << "Wetterabfrage-Funktionalität noch nicht vollständig implementiert.\n";
    }

    void performMathOperation(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            std::cout << "Verwendung: math <zahl1> <operator> <zahl2>\n";
            return;
        }
        
        double num1 = std::stod(args[0]);
        double num2 = std::stod(args[2]);
        std::string op = args[1];

        double result;
        if (op == "+") result = num1 + num2;
        else if (op == "-") result = num1 - num2;
        else if (op == "*") result = num1 * num2;
        else if (op == "/") {
            if (num2 == 0) {
                std::cout << "Fehler: Division durch Null!\n";
                return;
            }
            result = num1 / num2;
        }
        else {
            std::cout << "Ungültiger Operator.\n";
            return;
        }

        std::cout << "Ergebnis: " << result << std::endl;
    }

    void sortItems(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: sort <item1> <item2> ...\n";
            return;
        }

        std::vector<std::string> items = args;
        std::sort(items.begin(), items.end());

        std::cout << "Sortierte Elemente:\n";
        for (const auto& item : items) {
            std::cout << item << " ";
        }
        std::cout << std::endl;
    }

    void base64Operation(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Verwendung: base64 <encode|decode> <text>\n";
            return;
        }

        std::string operation = args[0];
        std::string input = args[1];

        if (operation == "encode") {
            std::cout << "Base64-Kodierung: " << base64_encode(input) << std::endl;
        } else if (operation == "decode") {
            std::cout << "Base64-Dekodierung: " << base64_decode(input) << std::endl;
        } else {
            std::cout << "Ungültige Operation. Verwenden Sie 'encode' oder 'decode'.\n";
        }
    }

    void hashString(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: hash <text>\n";
            return;
        }

        std::string input = args[0];
        std::size_t hash = std::hash<std::string>{}(input);
        std::cout << "Hash-Wert: " << hash << std::endl;
    }

    static std::string base64_encode(const std::string& input) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        std::string encoded;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        for (char c : input) {
            char_array_3[i++] = c;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; i < 4; i++)
                    encoded += base64_chars[char_array_4[i]];
                i = 0;
            }
        }

        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; j < i + 1; j++)
                encoded += base64_chars[char_array_4[j]];

            while (i++ < 3)
                encoded += '=';
        }

        return encoded;
    }

    static std::string base64_decode(const std::string& encoded_string) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";

        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string decoded;

        while (in_len-- && (encoded_string[in_] != '=') && (isalnum(encoded_string[in_]) || (encoded_string[in_] == '+') || (encoded_string[in_] == '/'))) {
            char_array_4[i++] = encoded_string[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; i < 3; i++)
                    decoded += char_array_3[i];
                i = 0;
            }
        }

        if (i) {
            for (j = 0; j < i; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

            for (j = 0; j < i - 1; j++) decoded += char_array_3[j];
        }

        return decoded;
    }
};

void EditCommand::execute(const std::vector<std::string>& args) {
    terminal.editFile(args);
}

int main() {
    Terminal terminal;
    terminal.run();
    return 0;
}