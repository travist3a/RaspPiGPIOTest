#include "SSHGPIO.h"
#include <cstdlib>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

// ─────── EDIT THESE FOUR LINES ONLY ────────────────────────────────────
static const std::string PI_USER = "craft";
static const std::string PI_HOST = "10.0.0.41";
static const std::string PWD = "abc";            // SSH password
static const std::string CMD_BASE = "gpioset gpiochip0 "; // "<gpio>=<0|1>"
// ────────────────────────────────────────────────────────────────────────


#ifdef _WIN32
// ─────────────────────────  Windows implementation  ────────────────────
namespace {
    PROCESS_INFORMATION g_pi{};      // plink process info
    HANDLE              g_stdinW = nullptr; // write‑end we own
    bool                g_connected = false;

    // find plink.exe next to our EXE; else rely on PATH
    std::string find_plink()
    {
        char buf[MAX_PATH] = {};
        GetModuleFileNameA(nullptr, buf, MAX_PATH);
        std::string path(buf);
        size_t cut = path.find_last_of("\\/");
        if (cut != std::string::npos)
            path.assign(path.substr(0, cut + 1)).append("plink.exe");

        DWORD a = GetFileAttributesA(path.c_str());
        if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY))
            return '"' + path + '"';      // quote spaces

        return "plink";                  // fallback to PATH
    }
}

bool gpio_open_session()
{
    if (g_connected) return true;

    SECURITY_ATTRIBUTES sa{ sizeof sa };
    sa.bInheritHandle = TRUE;

    HANDLE stdinR = nullptr;               // child’s STDIN (read end)
    if (!CreatePipe(&stdinR, &g_stdinW, &sa, 0)) return false;
    SetHandleInformation(g_stdinW, HANDLE_FLAG_INHERIT, 0);

    std::string cmdStr =
        find_plink() + " -batch -pw \"" + PWD + "\" "
        + PI_USER + '@' + PI_HOST + " -T";          // -T = no pseudo‑tty

    // mutable, NUL‑terminated buffer for CreateProcessA
    std::vector<char> cmd(cmdStr.begin(), cmdStr.end());
    cmd.push_back('\0');

    STARTUPINFOA si{ sizeof si };
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = stdinR;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = si.hStdOutput;

    if (!CreateProcessA(nullptr, cmd.data(),     // <- mutable buffer!
        nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr,
        &si, &g_pi))
    {
        CloseHandle(stdinR);
        CloseHandle(g_stdinW);
        g_stdinW = nullptr;
        return false;
    }

    CloseHandle(stdinR);          // parent no longer needs read end
    g_connected = true;
    return true;
}

void gpio_close_session()
{
    if (!g_connected) return;

    const char quit[] = "exit\n";
    DWORD dummy;
    WriteFile(g_stdinW, quit, sizeof quit - 1, &dummy, nullptr);

    CloseHandle(g_stdinW);
    WaitForSingleObject(g_pi.hProcess, 3000);

    CloseHandle(g_pi.hProcess);
    CloseHandle(g_pi.hThread);

    g_stdinW = nullptr;
    g_connected = false;
}

bool gpio_set_remote(int gpio, bool high)
{
    if (!g_connected && !gpio_open_session()) return false;

    std::string line =
        CMD_BASE + std::to_string(gpio) + '=' + (high ? '1' : '0') + '\n';

    DWORD sent = 0;
    BOOL ok = WriteFile(g_stdinW, line.data(),
        static_cast<DWORD>(line.size()), &sent, nullptr);
    return ok && sent == line.size();
}

#else   // ───────── Linux / macOS fallback (one‑shot sshpass) ───────────
bool gpio_open_session() { return true; }
void gpio_close_session() {}

bool gpio_set_remote(int gpio, bool high)
{
    std::string remote =
        CMD_BASE + std::to_string(gpio) + '=' + (high ? '1' : '0');

    std::string full =
        "sshpass -p '" + PWD + "' ssh -o StrictHostKeyChecking=no "
        + PI_USER + '@' + PI_HOST + " \"" + remote + '"';

    return std::system(full.c_str()) == 0;
}
#endif
