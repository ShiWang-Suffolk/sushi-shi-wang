#include <string>
#include <vector>

class Sushi {
private:
    std::vector<std::string> history;  // ❶ 使用 vector<string> 代替 int
    static const size_t HISTORY_LENGTH;
    static const size_t MAX_INPUT;

public:
    Sushi() : history() {};  // ❷ 初始化 history 为空 vector<string>
    std::string read_line(std::istream &in);
    bool read_config(const char *fname, bool ok_if_missing);
    void store_to_history(std::string line);
    void show_history();

    static const std::string DEFAULT_PROMPT;
};

#define UNUSED(expr) do {(void)(expr);} while (0)
