/* toggle_gpio_pi.cpp  ── cross‑platform (Windows+Linux) GPIO toggler
 * Build on Windows (Visual Studio Developer Prompt):
 *   cl /std:c++17 /O2 /EHsc toggle_gpio_pi.cpp
 *
 * Build on Linux/macOS:
 *   g++ -std=c++17 -O2 -Wall -o toggle_gpio_pi toggle_gpio_pi.cpp
 *
 * Run:
 *   toggle_gpio_pi.exe        ← interactive (q / 1 / 0)
 *   toggle_gpio_pi.exe 1      ← set HIGH
 *   toggle_gpio_pi.exe 0      ← set LOW
 */
#include <cstdlib>
#include <iostream>
#include <string>

 /// ──────  EDIT THESE FOUR LINES  ────────────────────────────────────────
const std::string PI_USER = "craft";        // SSH user on the Pi
const std::string PI_HOST = "10.0.0.41";    // Pi’s IP
const std::string PWD = "abc";          // SSH password
// libgpiod commands on the Pi (Ubuntu‑Server 24.04)
const std::string CMD_HIGH = "gpioset gpiochip0 2=1";
const std::string CMD_LOW = "gpioset gpiochip0 2=0";
/// ───────────────────────────────────────────────────────────────────────

/* Decide at compile‑time which SSH wrapper to use. */
#ifdef _WIN32
// Windows → use PuTTY’s plink.exe. Expect it next to toggle_gpio_pi.exe
const std::string SSH_PREFIX =
".\\plink.exe -batch -pw \"" + PWD + "\" ";
#else
// POSIX → use sshpass (install once: sudo apt install sshpass)
const std::string SSH_PREFIX =
"sshpass -p '" + PWD + "' ssh -o StrictHostKeyChecking=no ";
#endif

int exec_remote(bool high)
{
    const std::string remoteCmd = high ? CMD_HIGH : CMD_LOW;
    const std::string full =
        SSH_PREFIX + PI_USER + "@" + PI_HOST + " \"" + remoteCmd + "\"";
    return std::system(full.c_str());
}
/*
int main(int argc, char* argv[])
{
    auto ask = []() {
        std::cout << "\n[q]uit | [1] HIGH | [0] LOW : ";
        char c; std::cin >> c; return c;
        };

    while (true) {
        char sel = (argc == 2) ? argv[1][0] : ask();
        if (sel == 'q' || sel == 'Q') break;
        if (sel != '1' && sel != '0') {
            std::cerr << "Invalid input – use 1, 0, or q\n";
            if (argc == 2) return 1;
            continue;
        }

        bool toHigh = (sel == '1');
        int rc = exec_remote(toHigh);

        if (rc == 0)
            std::cout << "GPIO 2 set " << (toHigh ? "HIGH\n" : "LOW\n");
        else
            std::cerr << "Remote command failed (exit " << rc << ")\n";

        if (argc == 2) break;
    }
    return 0;
}
*/