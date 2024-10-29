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
#include <execution>

namespace fs = std::filesystem;

class ConsoleColor {
public:
    static void set(int textColor, int bgColor) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, (bgColor << 4) | textColor);
    }
};

class CommandResult {
public:
    enum class Status {
        Success,
        Error,
        Warning
    };

    CommandResult(Status s, std::string msg) : status(s), message(msg) {}
    
    Status getStatus() const { return status; }
    const std::string& getMessage() const { return message; }

private:
    Status status;
    std::string message;
};

class Command {
public:
    virtual ~Command() = default;
    virtual CommandResult execute(const std::vector<std::string>& args) = 0;
    virtual std::string getHelp() const = 0;
    virtual std::string getUsage() const = 0;
    virtual bool validateArgs(const std::vector<std::string>& args) const = 0;
    virtual std::vector<std::string> getCompletions(const std::string& prefix) const { return {}; }
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

    CommandResult execute(const std::vector<std::string>& args) override {
        if (args.empty()) {
            std::cout << "Verfuegbare Befehle:\n";
            for (const auto& [name, description] : commandDescriptions) {
                std::cout << std::left << std::setw(15) << name << " - " << description << "\n";
            }
            std::cout << "\nFuer detaillierte Informationen zu einem Befehl, geben Sie 'help <Befehlsname>' ein.\n";
            return CommandResult(CommandResult::Status::Success, "Displayed available commands");
        } else {
            const std::string& commandName = args[0];
            auto it = commandDescriptions.find(commandName);
            if (it != commandDescriptions.end()) {
                std::cout << commandName << " - " << it->second << "\n";
                return CommandResult(CommandResult::Status::Success, "Displayed help for command: " + commandName);
            } else {
                std::cout << "Kein Hilfetext verfuegbar fuer den Befehl: " << commandName << "\n";
                return CommandResult(CommandResult::Status::Error, "No help available for command: " + commandName);
            }
        }
    }

    std::string getHelp() const override {
        return "Zeigt Hilfe zu verfuegbaren Befehlen an";
    }

    std::string getUsage() const override {
        return "help [Befehlsname]";
    }

    bool validateArgs(const std::vector<std::string>& args) const override {
        return args.empty() || args.size() == 1;
    }

private:
    void initializeCommandDescriptions() {
        commandDescriptions = {
            {"help", "Zeigt diese Hilfemeldung an. Verwendung: help [Befehlsname]"},
            {"cls", "Loescht den Bildschirminhalt. Verwendung: cls"},
            {"echo", "Gibt einen Text aus. Verwendung: echo <Text>"},
            {"time", "Zeigt die aktuelle Systemzeit an. Verwendung: time"},
            {"calc", "Oeffnet den Windows-Taschenrechner. Verwendung: calc"},
            {"history", "Zeigt den Befehlsverlauf an. Verwendung: history"},
            {"alias", "Zeigt alle definierten Aliase an. Verwendung: alias"},
            {"setalias", "Erstellt ein neues Alias. Verwendung: setalias <Aliasname> <Befehl>"},
            {"create", "Erstellt eine neue Datei. Verwendung: create <Dateiname>"},
            {"delete", "Loescht eine Datei. Verwendung: delete <Dateiname>"},
            {"list", "Listet Dateien im aktuellen Verzeichnis auf. Verwendung: list"},
            {"ping", "Sendet ICMP-Echo-Anforderungen an einen Host. Verwendung: ping <Hostname oder IP>"},
            {"random", "Generiert eine Zufallszahl zwischen 1 und 100. Verwendung: random"},
            {"sleep", "Pausiert die Ausfuehrung fuer eine bestimmte Zeit. Verwendung: sleep <Sekunden>"},
            {"theme", "Aendert das Farbschema des Terminals. Verwendung: theme"},
            {"writefile", "Schreibt Text in eine Datei. Verwendung: writefile <Dateiname> <Text>"},
            {"readfile", "Liest den Inhalt einer Datei. Verwendung: readfile <Dateiname>"},
            {"encrypt", "Verschluesselt eine Datei. Verwendung: encrypt <Eingabedatei> <Ausgabedatei>"},
            {"decrypt", "Entschluesselt eine Datei. Verwendung: decrypt <Eingabedatei> <Ausgabedatei>"},
            {"compress", "Komprimiert eine Datei. Verwendung: compress <Eingabedatei> <Ausgabedatei>"},
            {"decompress", "Dekomprimiert eine Datei. Verwendung: decompress <Eingabedatei> <Ausgabedatei>"},
            {"search", "Sucht nach Dateien mit einem bestimmten Muster. Verwendung: search <Suchmuster>"},
            {"schedule", "Plant die Ausfuehrung eines Befehls. Verwendung: schedule <Verzoegerung in Sekunden> <Befehl>"},
            {"task", "Verwaltet Hintergrundaufgaben. Verwendung: task <start|stop|list> [Befehl]"},
            {"network", "Zeigt Netzwerkinformationen an. Verwendung: network"},
            {"sysinfo", "Zeigt Systeminformationen an. Verwendung: sysinfo"},
            {"weather", "Zeigt Wetterinformationen fuer eine Stadt an. Verwendung: weather <Stadt>"},
            {"math", "Fuehrt einfache mathematische Operationen durch. Verwendung: math <Zahl1> <Operator> <Zahl2>"},
            {"sort", "Sortiert eine Liste von Elementen. Verwendung: sort <Element1> <Element2> ..."},
            {"base64", "Kodiert oder dekodiert Text in Base64. Verwendung: base64 <encode|decode> <Text>"},
            {"hash", "Berechnet den Hash-Wert eines Textes. Verwendung: hash <Text>"},
            {"edit", "Oeffnet einen einfachen Texteditor. Verwendung: edit <dateiname>"},
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

    CommandResult execute(const std::vector<std::string>& args) override {
        if (!validateArgs(args)) {
            return CommandResult(CommandResult::Status::Error, "Invalid arguments");
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

            if (tokens[0] == "quit") break;

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
            }
        }
        return CommandResult(CommandResult::Status::Success, "File edited successfully");
    }

    std::string getHelp() const override { return "Oeffnet einen einfachen Texteditor"; }
    std::string getUsage() const override { return "edit <dateiname>"; }
    bool validateArgs(const std::vector<std::string>& args) const override {
        return !args.empty() && args.size() == 1;
    }
};

class ProcessManager {
public:
    static void listProcesses() {
        system("tasklist");
    }

    static void killProcess(const std::string& pid) {
        system(("taskkill /PID " + pid + " /F").c_str());
    }
};

class PerformanceMetrics {
private:
    static inline std::map<std::string, std::chrono::microseconds> metrics;
    static inline std::mutex metricsMutex;

public:
    static void record(const std::string& operation, std::chrono::microseconds duration) {
        std::lock_guard<std::mutex> lock(metricsMutex);
        metrics[operation] += duration;
    }

    static void displayMetrics() {
        std::lock_guard<std::mutex> lock(metricsMutex);
        for (const auto& [op, duration] : metrics) {
            std::cout << op << ": " << duration.count() / 1000.0 << "ms\n";
        }
    }
};

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    ThreadPool(size_t threads) : stop(false) {
        for(size_t i = 0; i < threads; ++i)
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        if(stop && tasks.empty())
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
    }

    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers)
            worker.join();
    }
};

class ConcreteCommand : public Command {
private:
    std::function<void(const std::vector<std::string>&)> executeFunc;
    std::string helpText;
    std::string usageText;

public:
    ConcreteCommand(
        std::function<void(const std::vector<std::string>&)> exec,
        std::string help,
        std::string usage
    ) : executeFunc(std::move(exec)),
        helpText(std::move(help)),
        usageText(std::move(usage)) {}

    CommandResult execute(const std::vector<std::string>& args) override {
        executeFunc(args);
        return CommandResult(CommandResult::Status::Success, "Command executed");
    }

    std::string getHelp() const override {
        return helpText;
    }

    std::string getUsage() const override {
        return usageText;
    }

    bool validateArgs(const std::vector<std::string>& args) const override {
        return true;
    }
};

class TerminalUI {
private:
    const int WINDOW_WIDTH = 120;
    const int WINDOW_HEIGHT = 30;
    
    struct Theme {
        int backgroundColor;
        int textColor;
        int highlightColor;
        int errorColor;
        int successColor;
        int promptColor;
        int headerColor;
        std::string promptSymbol;
        std::string headerBorder;
        std::string footerBorder;
        std::string separator;
    };
    
    Theme currentTheme = {
        0,      
        15,     
        14,     
        12,     
        10,     
        11,     
        13,     
        "λ",    
        "═",    
        "─",    
        "│"     
    };

public:
    void initializeWindow() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        
        std::string cmd = "mode con: cols=" + std::to_string(WINDOW_WIDTH) + 
                         " lines=" + std::to_string(WINDOW_HEIGHT);
        system(cmd.c_str());
        
        SetConsoleTitleA("Aether Terminal - Modern CLI");
        clearScreen();
    }

    void drawHeader() {
        ConsoleColor::set(currentTheme.headerColor, currentTheme.backgroundColor);
        
        
        std::cout << "╔" << std::string(WINDOW_WIDTH - 2, currentTheme.headerBorder[0]) << "╗\n";
        
        
        std::string title = " AETHER TERMINAL ";
        std::string subtitle = " Modern Command Line Interface ";
        
        drawCenteredText(title, true);
        drawCenteredText(subtitle, false);
        
        
        std::cout << "╚" << std::string(WINDOW_WIDTH - 2, currentTheme.headerBorder[0]) << "╝\n";
        
        
        drawStatusBar();
        
        resetColors();
    }

    void drawPrompt(const std::string& currentDir) {
        
        ConsoleColor::set(currentTheme.promptColor, currentTheme.backgroundColor);
        std::cout << "\n" << currentTheme.promptSymbol << " ";
        
        ConsoleColor::set(currentTheme.highlightColor, currentTheme.backgroundColor);
        std::cout << shortenPath(currentDir);
        
        
        std::string gitBranch = getGitBranch();
        if (!gitBranch.empty()) {
            ConsoleColor::set(currentTheme.successColor, currentTheme.backgroundColor);
            std::cout << " [" << gitBranch << "]";
        }
        
        ConsoleColor::set(currentTheme.textColor, currentTheme.backgroundColor);
        std::cout << " → ";
    }

    void drawError(const std::string& message) {
        std::cout << "\n";
        ConsoleColor::set(currentTheme.errorColor, currentTheme.backgroundColor);
        std::cout << "✘ ERROR: " << message << "\n";
        resetColors();
    }

    void drawSuccess(const std::string& message) {
        std::cout << "\n";
        ConsoleColor::set(currentTheme.successColor, currentTheme.backgroundColor);
        std::cout << "✓ SUCCESS: " << message << "\n";
        resetColors();
    }

    void drawInfo(const std::string& message) {
        std::cout << "\n";
        ConsoleColor::set(currentTheme.highlightColor, currentTheme.backgroundColor);
        std::cout << "ℹ INFO: " << message << "\n";
        resetColors();
    }

    void clearScreen() {
        system("cls");
    }

private:
    void drawStatusBar() {
        ConsoleColor::set(currentTheme.promptColor, currentTheme.backgroundColor);
        
        
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        int memoryUsage = static_cast<int>(memInfo.dwMemoryLoad);
        
        
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M:%S");
        
        
        std::string status = " RAM: " + std::to_string(memoryUsage) + "% | " + ss.str();
        std::cout << std::string(WINDOW_WIDTH - status.length() - 1, ' ') << status << "\n";
    }

    std::string getGitBranch() {
        return "";
    }

    void drawCenteredText(const std::string& text, bool isTitle) {
        int padding = (WINDOW_WIDTH - 2 - text.length()) / 2;
        std::cout << currentTheme.separator;
        
        if (isTitle) {
            ConsoleColor::set(currentTheme.highlightColor, currentTheme.backgroundColor);
        }
        
        std::cout << std::string(padding, ' ') << text 
                 << std::string(WINDOW_WIDTH - 2 - padding - text.length(), ' ');
        
        std::cout << currentTheme.separator << "\n";
    }

    std::string shortenPath(const std::string& path) {
        if (path.length() <= 40) return path;
        
        size_t pos = path.find_last_of("\\/", path.length() - 2);
        if (pos != std::string::npos) {
            return "..." + path.substr(pos);
        }
        return path;
    }

    void resetColors() {
        ConsoleColor::set(currentTheme.textColor, currentTheme.backgroundColor);
    }
};

class Terminal {
private:
    TerminalUI ui;
    std::map<std::string, std::unique_ptr<Command>> commands;

    void executeCommand(const std::string& command) {
        try {
            auto start = std::chrono::high_resolution_clock::now();
            
            std::vector<std::string> args = parseCommand(command);
            if (args.empty()) return;

            std::string cmd = args[0];
            args.erase(args.begin());

            
            ui.drawInfo("Führe aus: " + command);

            if (aliases.find(cmd) != aliases.end()) {
                cmd = aliases[cmd];
            }

            if (auto it = commands.find(cmd); it != commands.end()) {
                if (it->second->validateArgs(args)) {
                    it->second->execute(args);
                    
                    auto end = std::chrono::high_resolution_clock::now();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
                    ui.drawSuccess("Befehl in " + std::to_string(duration.count()) + " ms ausgeführt");
                } else {
                    throw std::runtime_error("Ungültige Argumente. Verwendung: " + it->second->getUsage());
                }
            } else {
                throw std::runtime_error("Unbekannter Befehl: " + cmd);
            }
        } catch (const std::exception& e) {
            ui.drawError(e.what());
        }
    }

    void handleEcho(const std::vector<std::string>& args) {
        std::string message = std::accumulate(
            args.begin(), args.end(),
            std::string{},
            [](std::string a, const std::string& b) {
                return std::move(a) + (a.empty() ? "" : " ") + b;
            }
        );
        std::cout << message << std::endl;
    }

    std::map<std::string, int> defaultTheme{
        {"promptTextColor", 10},
        {"promptBgColor", 0},
        {"defaultTextColor", 7},
        {"defaultBgColor", 0}
    };

    std::map<std::string, int> currentTheme{defaultTheme};
    std::vector<std::string> commandHistory;
    std::map<std::string, std::string> aliases;
    std::unique_ptr<TaskManager> taskManager;
    std::thread taskThread;
    int historyIndex{-1};

    std::atomic<bool> isRunning{true};
    std::vector<std::thread> workerThreads;
    ThreadPool threadPool{4};

    std::vector<std::string> parseCommand(const std::string& commandLine) {
        std::vector<std::string> args;
        std::string currentArg;
        bool inQuotes = false;
        
        for (char c : commandLine) {
            if (c == '"') {
                inQuotes = !inQuotes;
                continue;
            }
            
            if (c == ' ' && !inQuotes) {
                if (!currentArg.empty()) {
                    args.push_back(currentArg);
                    currentArg.clear();
                }
            } else {
                currentArg += c;
            }
        }
        
        if (!currentArg.empty()) {
            args.push_back(currentArg);
        }
        
        return args;
    }

    void initializeCommands() {
        commands["help"] = std::make_unique<HelpCommand>(commands);
        
        commands["cls"] = std::make_unique<ConcreteCommand>(
            [](const auto& args [[maybe_unused]]) { system("cls"); },
            "Löscht den Bildschirminhalt",
            "cls"
        );

        commands["echo"] = std::make_unique<ConcreteCommand>(
            [](const auto& args) { 
                for (const auto& arg : args) std::cout << arg << " ";
                std::cout << std::endl;
            },
            "Gibt Text aus",
            "echo <text>"
        );

        commands["time"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { displayCurrentTime(); },
            "Zeigt die aktuelle Zeit an",
            "time"
        );

        commands["history"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { printHistory(); },
            "Zeigt den Befehlsverlauf an",
            "history"
        );

        commands["writefile"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { writeToFile(args); },
            "Schreibt Text in eine Datei",
            "writefile <dateiname> <text>"
        );

        commands["readfile"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { readFromFile(args); },
            "Liest den Inhalt einer Datei",
            "readfile <dateiname>"
        );

        commands["create"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { createFile(args); },
            "Erstellt eine neue Datei",
            "create <dateiname>"
        );

        commands["delete"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { deleteFile(args); },
            "Löscht eine Datei",
            "delete <dateiname>"
        );

        commands["list"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { listFiles(); },
            "Listet Dateien im aktuellen Verzeichnis auf",
            "list"
        );

        commands["encrypt"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { encryptFile(args); },
            "Verschlüsselt eine Datei",
            "encrypt <eingabedatei> <ausgabedatei>"
        );

        commands["decrypt"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { decryptFile(args); },
            "Entschlüsselt eine Datei",
            "decrypt <eingabedatei> <ausgabedatei>"
        );

        commands["compress"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { compressFile(args); },
            "Komprimiert eine Datei",
            "compress <eingabedatei> <ausgabedatei>"
        );

        commands["decompress"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { decompressFile(args); },
            "Dekomprimiert eine Datei",
            "decompress <eingabedatei> <ausgabedatei>"
        );

        commands["search"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { searchFiles(args); },
            "Sucht nach Dateien mit einem bestimmten Muster",
            "search <suchmuster>"
        );

        commands["ps"] = std::make_unique<ConcreteCommand>(
            [](const auto&) { ProcessManager::listProcesses(); },
            "Zeigt laufende Prozesse an",
            "ps"
        );

        commands["kill"] = std::make_unique<ConcreteCommand>(
            [](const auto& args) { 
                if (!args.empty()) ProcessManager::killProcess(args[0]); 
            },
            "Beendet einen Prozess",
            "kill <pid>"
        );

        commands["benchmark"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { runBenchmark(); },
            "Führt einen Systemtest durch",
            "benchmark"
        );

        commands["stats"] = std::make_unique<ConcreteCommand>(
            [](const auto&) { PerformanceMetrics::displayMetrics(); },
            "Zeigt Leistungsmetriken an",
            "stats"
        );

        commands["edit"] = std::make_unique<EditCommand>(*this);
    }

public:
    Terminal() : 
        taskManager(std::make_unique<TaskManager>()),
        taskThread([this]() {
            taskManager->run();
        })
    {
        initializeCommands();
    }

    ~Terminal() {
        taskManager->stop();
        if (taskThread.joinable()) {
            taskThread.join();
        }
    }

    void run() {
        ui.initializeWindow();
        ui.drawHeader();
        printWelcomeMessage();
        
        loadCommandHistory();
        loadAliases();
        loadThemes();
        registerCommands();

        std::string command;
        while (true) {
            ui.drawPrompt(fs::current_path().string());
            command = getCommandInput();

            if (command.empty()) continue;

            if (command == "exit") {
                ui.drawInfo("Beende Terminal...");
                saveAliases();
                break;
            }

            addCommandToHistory(command);
            executeCommand(command);
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
    void registerCommands() {
        commands["help"] = std::make_unique<HelpCommand>(commands);
        
        commands["cls"] = std::make_unique<ConcreteCommand>(
            [](const auto& args [[maybe_unused]]) { system("cls"); },
            "Löscht den Bildschirminhalt",
            "cls"
        );

        commands["echo"] = std::make_unique<ConcreteCommand>(
            [](const auto& args) { 
                for (const auto& arg : args) std::cout << arg << " ";
                std::cout << std::endl;
            },
            "Gibt Text aus",
            "echo <text>"
        );

        commands["time"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { displayCurrentTime(); },
            "Zeigt die aktuelle Zeit an",
            "time"
        );

        commands["history"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { printHistory(); },
            "Zeigt den Befehlsverlauf an",
            "history"
        );

        commands["writefile"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { writeToFile(args); },
            "Schreibt Text in eine Datei",
            "writefile <dateiname> <text>"
        );

        commands["readfile"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { readFromFile(args); },
            "Liest den Inhalt einer Datei",
            "readfile <dateiname>"
        );

        commands["create"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { createFile(args); },
            "Erstellt eine neue Datei",
            "create <dateiname>"
        );

        commands["delete"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { deleteFile(args); },
            "Löscht eine Datei",
            "delete <dateiname>"
        );

        commands["list"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { listFiles(); },
            "Listet Dateien im aktuellen Verzeichnis auf",
            "list"
        );

        commands["encrypt"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { encryptFile(args); },
            "Verschlüsselt eine Datei",
            "encrypt <eingabedatei> <ausgabedatei>"
        );

        commands["decrypt"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { decryptFile(args); },
            "Entschlüsselt eine Datei",
            "decrypt <eingabedatei> <ausgabedatei>"
        );

        commands["compress"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { compressFile(args); },
            "Komprimiert eine Datei",
            "compress <eingabedatei> <ausgabedatei>"
        );

        commands["decompress"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { decompressFile(args); },
            "Dekomprimiert eine Datei",
            "decompress <eingabedatei> <ausgabedatei>"
        );

        commands["search"] = std::make_unique<ConcreteCommand>(
            [this](const auto& args) { searchFiles(args); },
            "Sucht nach Dateien mit einem bestimmten Muster",
            "search <suchmuster>"
        );

        commands["ps"] = std::make_unique<ConcreteCommand>(
            [](const auto&) { ProcessManager::listProcesses(); },
            "Zeigt laufende Prozesse an",
            "ps"
        );

        commands["kill"] = std::make_unique<ConcreteCommand>(
            [](const auto& args) { 
                if (!args.empty()) ProcessManager::killProcess(args[0]); 
            },
            "Beendet einen Prozess",
            "kill <pid>"
        );

        commands["benchmark"] = std::make_unique<ConcreteCommand>(
            [this](const auto&) { runBenchmark(); },
            "Führt einen Systemtest durch",
            "benchmark"
        );

        commands["stats"] = std::make_unique<ConcreteCommand>(
            [](const auto&) { PerformanceMetrics::displayMetrics(); },
            "Zeigt Leistungsmetriken an",
            "stats"
        );

        commands["edit"] = std::make_unique<EditCommand>(*this);
    }

    void printWelcomeMessage() {
        ConsoleColor::set(14, 0);
        std::cout << R"(
    _    _____ _____ _   _ _____ ____  
   / \  | ____|_   _| | | | ____|  _ \ 
  / _ \ |  _|   | | | |_| |  _| | |_) |
 / ___ \| |___  | | |  _  | |___|  _ < 
/_/   \_\_____| |_| |_| |_|_____|_| \_\
        )" << std::endl;
        
        ui.drawInfo("Willkommen im Aether Terminal v1.0");
        ui.drawInfo("Geben Sie 'help' ein für eine Liste der verfuegbaren Befehle");
        std::cout << std::endl;
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
        std::string input;
        int cursorPos = 0;
        
        while (true) {
            if (_kbhit()) {
                int ch = _getch();
                
                if (ch == 13) { 
                    std::cout << '\n';
                    return input;
                }
                else if (ch == 8) { 
                    if (!input.empty() && cursorPos > 0) {
                        input.erase(--cursorPos, 1);
                        
                        std::cout << "\b \b" << input.substr(cursorPos);
                    }
                }
                else if (ch == 224) { 
                    ch = _getch();
                    if (ch == 72) { 
                        if (!commandHistory.empty() && historyIndex < commandHistory.size() - 1) {
                            
                            std::cout << "\r" << std::string(input.length() + 3, ' ');
                            input = commandHistory[++historyIndex];
                            cursorPos = input.length();
                            ui.drawPrompt(fs::current_path().string());
                            std::cout << input;
                        }
                    }
                    else if (ch == 80) { 
                        if (historyIndex > 0) {
                            std::cout << "\r" << std::string(input.length() + 3, ' ');
                            input = commandHistory[--historyIndex];
                            cursorPos = input.length();
                            ui.drawPrompt(fs::current_path().string());
                            std::cout << input;
                        }
                    }
                }
                else if (ch >= 32 && ch <= 126) { 
                    input.insert(cursorPos++, 1, (char)ch);
                    std::cout << (char)ch;
                }
            }
        }
    }

    void addCommandToHistory(const std::string& command) {
        commandHistory.push_back(command);
        historyIndex = -1;
        saveCommandHistory();
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
            std::cout << "Datei '" << args[0] << "' geloescht.\n";
        } else {
            std::cerr << "Fehler: Datei '" << args[0] << "' konnte nicht geloescht werden.\n";
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
        std::cout << "Schlafe fuer " << seconds << " Sekunden...\n";
        std::this_thread::sleep_for(std::chrono::seconds(seconds));
        std::cout << "Aufgewacht!\n";
    }

    void switchTheme() {
        std::cout << "Waehlen Sie ein Theme: (1) Dunkel, (2) Hell, (3) Benutzerdefiniert\n";
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
            std::cerr << "Fehler: Konnte Dateien nicht oeffnen.\n";
            return;
        }
        char ch;
        while (inFile.get(ch)) {
            ch = ch ^ 0x5A;
            outFile.put(ch);
        }
        std::cout << "Datei erfolgreich verschluesselt.\n";
    }

    void decryptFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: decrypt <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht oeffnen.\n";
            return;
        }
        char ch;
        while (inFile.get(ch)) {
            ch = ch ^ 0x5A;
            outFile.put(ch);
        }
        std::cout << "Datei erfolgreich entschluesselt.\n";
    }

    void compressFile(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << "Verwendung: compress <eingabedatei> <ausgabedatei>\n";
            return;
        }
        std::ifstream inFile(args[0], std::ios::binary);
        std::ofstream outFile(args[1], std::ios::binary);
        if (!inFile || !outFile) {
            std::cerr << "Fehler: Konnte Dateien nicht oeffnen.\n";
            return;
        }
        
        char current;
        unsigned char count = 0;
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
            std::cerr << "Fehler: Konnte Dateien nicht oeffnen.\n";
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
            std::cout << "Verwendung: schedule <verzoegerung_in_sekunden> <befehl>\n";
            return;
        }
        int delay = std::stoi(args[0]);
        std::string cmd;
        for (size_t i = 1; i < args.size(); ++i) {
            cmd += args[i] + " ";
        }
        taskManager->addTask([this, delay, cmd]() {
            std::this_thread::sleep_for(std::chrono::seconds(delay));
            executeCommand(cmd);
        });
        std::cout << "Befehl geplant, wird in " << delay << " Sekunden ausgefuehrt.\n";
    }

    void manageTask(const std::vector<std::string>& args) {
        if (args.empty()) {
            std::cout << "Verwendung: task <start|stop|list> [Befehl]\n";
            return;
        }

        if (args[0] == "start" && args.size() > 1) {
            std::string taskCommand = args[1];
            taskManager->addTask([this, taskCommand]() {
                executeCommand(taskCommand);
            });
            std::cout << "Hintergrundaufgabe gestartet: " << taskCommand << std::endl;
        } else if (args[0] == "list") {
            std::cout << "Hintergrundaufgaben-Verwaltung noch nicht vollstaendig implementiert.\n";
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
        std::cout << "Wetterinformationen fuer " << city << " werden abgerufen...\n";
        std::cout << "Wetterabfrage-Funktionalitaet noch nicht vollstaendig implementiert.\n";
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
            std::cout << "Ungueltige Operation.\n";
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
            std::cout << "Ungueltige Operation. Verwenden Sie 'encode' oder 'decode'.\n";
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

    void runBenchmark() {
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<fs::path> files;
        std::mutex filesMutex;
        
        std::for_each(std::execution::par,
            fs::recursive_directory_iterator(fs::current_path()),
            fs::recursive_directory_iterator(),
            [&](const auto& entry) {
                if (entry.is_regular_file()) {
                    std::lock_guard<std::mutex> lock(filesMutex);
                    files.push_back(entry.path());
                }
            });

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "Benchmark Results:\n";
        std::cout << "Files scanned: " << files.size() << "\n";
        std::cout << "Time taken: " << duration.count() << "ms\n";
    }

    void executeCommandAsync(const std::string& command) {
        threadPool.enqueue([this, command]() {
            auto start = std::chrono::high_resolution_clock::now();
            executeCommand(command);
            auto end = std::chrono::high_resolution_clock::now();
            
            PerformanceMetrics::record(command, 
                std::chrono::duration_cast<std::chrono::microseconds>(end - start));
        });
    }

    class CommandHistory {
    public:
        void add(const std::string& command) {
            if (command.empty() || command == "exit") return;
            
            history.push_back(command);
            if (history.size() > maxHistory) {
                history.pop_front();
            }
            save();
        }
        
        std::string searchBackward(const std::string& prefix) {
            for (auto it = history.rbegin(); it != history.rend(); ++it) {
                if (it->find(prefix) == 0) {
                    return *it;
                }
            }
            return prefix;
        }
        
        void save() {
            std::ofstream file(historyFile);
            for (const auto& cmd : history) {
                file << cmd << '\n';
            }
        }
        
        void load() {
            std::ifstream file(historyFile);
            std::string line;
            while (std::getline(file, line)) {
                add(line);
            }
        }

    private:
        std::deque<std::string> history;
        const size_t maxHistory = 1000;
        const std::string historyFile = "command_history.txt";
    };

    class CommandInput {
    private:
        const std::map<std::string, std::unique_ptr<Command>>& commands;
        std::vector<std::string>& commandHistory;
        size_t& historyIndex;
        TerminalUI& ui;

        void handleBackspace(std::string& input, size_t& cursorPos) {
            if (!input.empty() && cursorPos > 0) {
                input.erase(--cursorPos, 1);
                std::cout << "\b \b";
            }
        }

        void handleAutoComplete(std::string& input, size_t& cursorPos) {
            std::string prefix = input.substr(0, cursorPos);
            std::vector<std::string> matches;
            
            
            for (const auto& [name, cmd] : commands) {
                if (name.find(prefix) == 0) {
                    matches.push_back(name);
                }
                
                auto cmdCompletions = cmd->getCompletions(prefix);
                matches.insert(matches.end(), cmdCompletions.begin(), cmdCompletions.end());
            }

            if (matches.empty()) {
                return;
            }

            if (matches.size() == 1) {
                
                input = matches[0];
                cursorPos = input.length();
                redrawLine(input);
            } else {
                
                std::string commonPrefix = findCommonPrefix(matches);
                if (commonPrefix.length() > prefix.length()) {
                    input = commonPrefix + input.substr(cursorPos);
                    cursorPos = commonPrefix.length();
                    redrawLine(input);
                }
                
                showCompletions(matches);
            }
        }

    public:
        CommandInput(
            const std::map<std::string, std::unique_ptr<Command>>& cmds,
            std::vector<std::string>& history,
            size_t& hIndex,
            TerminalUI& terminalUI
        ) : commands(cmds),
            commandHistory(history),
            historyIndex(hIndex),
            ui(terminalUI) {}
        
        std::string getInput() {
            std::string input;
            char ch;
            size_t cursorPos = 0;
            
            while ((ch = _getch()) != 13) {  
                if (ch == '\t') {  
                    handleAutoComplete(input, cursorPos);
                } else if (ch == 8) {  
                    handleBackspace(input, cursorPos);
                } else if (ch == 224) {  
                    ch = _getch();
                    if (ch == 72) {  
                        if (!commandHistory.empty() && historyIndex < commandHistory.size() - 1) {
                            std::cout << "\r" << std::string(input.length() + 3, ' ');
                            input = commandHistory[++historyIndex];
                            cursorPos = input.length();
                            ui.drawPrompt(fs::current_path().string());
                            std::cout << input;
                        }
                    }
                    else if (ch == 80) {  
                        if (historyIndex > 0) {
                            std::cout << "\r" << std::string(input.length() + 3, ' ');
                            input = commandHistory[--historyIndex];
                            cursorPos = input.length();
                            ui.drawPrompt(fs::current_path().string());
                            std::cout << input;
                        }
                    }
                }
                else if (ch >= 32 && ch <= 126) {  
                    input.insert(cursorPos++, 1, (char)ch);
                    std::cout << (char)ch;
                }
            }
            
            return input;
        }

    private:
        void showCompletions(const std::vector<std::string>& completions) {
            std::cout << "\n";
            size_t maxLength = 0;
            for (const auto& c : completions) {
                maxLength = std::max(maxLength, c.length());
            }

            size_t columns = std::max(size_t{1}, 80 / (maxLength + 2));
            size_t col = 0;

            for (const auto& completion : completions) {
                std::cout << std::left << std::setw(maxLength + 2) << completion;
                if (++col >= columns) {
                    std::cout << "\n";
                    col = 0;
                }
            }
            if (col != 0) std::cout << "\n";
        }

        void redrawLine(const std::string& input) {
            std::cout << "\r" << std::string(input.length() + 3, ' ') << "\r";
            ui.drawPrompt(fs::current_path().string());
            std::cout << input;
        }

        std::string findCommonPrefix(const std::vector<std::string>& strings) {
            if (strings.empty()) return "";
            
            std::string prefix = strings[0];
            for (const auto& s : strings) {
                while (s.find(prefix) != 0) {
                    prefix = prefix.substr(0, prefix.length() - 1);
                }
            }
            return prefix;
        }
    };

    class CommandCache {
    public:
        std::optional<std::string> get(const std::string& key) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            auto it = cache.find(key);
            if (it != cache.end() && !isExpired(it->second)) {
                return it->second.result;
            }
            return std::nullopt;
        }
        
        void put(const std::string& key, const std::string& result) {
            std::lock_guard<std::mutex> lock(cacheMutex);
            cache[key] = CacheEntry{result, std::chrono::system_clock::now()};
        }

    private:
        struct CacheEntry {
            std::string result;
            std::chrono::system_clock::time_point timestamp;
        };
        
        bool isExpired(const CacheEntry& entry) {
            auto now = std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::minutes>(
                now - entry.timestamp).count() > 5;  
        }
        
        std::mutex cacheMutex;
        std::map<std::string, CacheEntry> cache;
    };
};

int main() {
    Terminal terminal;
    terminal.run();
    return 0;
}