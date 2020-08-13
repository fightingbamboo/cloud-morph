#include <iostream>
#include <windows.h>
// #include <psapi.h>
#include <vector>
#include <sstream>
using namespace std;

int screenWidth, screenHeight;

int clientConnect()
{
    WSADATA wsa_data;
    SOCKADDR_IN addr;

    WSAStartup(MAKEWORD(2, 0), &wsa_data);
    int server = socket(AF_INET, SOCK_STREAM, 0);

    //inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);

    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = INADDR_ANY;

    connect(server, reinterpret_cast<SOCKADDR *>(&addr), sizeof(addr));
    cout << "Connected to server!" << endl;
    return server;
}

// get the horizontal and vertical screen sizes in pixel
void getDesktopResolution(int &width, int &height)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (width, height)
    width = desktop.right;
    height = desktop.bottom;
}

HWND getWindowByTitle(char *pattern)
{
    HWND hwnd = NULL;

    do
    {
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
        DWORD dwPID = 0;
        GetWindowThreadProcessId(hwnd, &dwPID);
        cout << "searching" << hwnd << endl;
        int len = GetWindowTextLength(hwnd) + 1;

        char title[len];
        GetWindowText(hwnd, title, len);
        string st(title);
        cout << "len " << len << " Title : " << st << endl;

        if (st.find(pattern) != string::npos)
        {
            cout << "Found " << hwnd << endl;
            return hwnd;
        }
    } while (hwnd != 0);
    cout << "Searching Not found";
    return hwnd; //Ignore that
}

HWND getWindow(DWORD dwProcessID)
{
    HWND hwnd = NULL;

    do
    {
        hwnd = FindWindowEx(NULL, hwnd, NULL, NULL);
        DWORD dwPID = 0;
        GetWindowThreadProcessId(hwnd, &dwPID);
        cout << "searching" << hwnd << endl;
        if (dwPID == dwProcessID)
        {
            cout << "yay: " << hwnd << " :pid: " << dwPID << endl; //debug
            int len = GetWindowTextLength(hwnd) + 1;
            //std::string s;

            //vector<char> list(len);
            char title[len];
            //s.reserve(len);
            //GetWindowText(hwnd, const_cast<char*>(s.c_str()), len - 1);
            GetWindowText(hwnd, title, len - 1);
            cout << "len " << len << " Title : " << title << endl;
        }
    } while (hwnd != 0);
    cout << "Searching done"
         << " " << hwnd << endl;
    return hwnd; //Ignore that
}

HWND sendIt(HWND hwnd, int key)
{
    INPUT ip;

    // Set up a generic keyboard event.
    ip.type = INPUT_KEYBOARD;
    ip.ki.wScan = 0; // hardware scan code for key
    ip.ki.time = 0;
    ip.ki.dwExtraInfo = 0;
    ip.ki.wVk = key;   // virtual-key code for the "a" key
    ip.ki.dwFlags = 0; // 0 for key press

    cout << "Sending key " << ' ' << key << endl;
    HWND temp = SetActiveWindow(hwnd);
    ShowWindow(hwnd, SW_RESTORE);
    SetFocus(hwnd);
    BringWindowToTop(hwnd);

    //cout << temp << endl;
    SendInput(1, &ip, sizeof(INPUT));

    // PostMessage(hwnd, WM_KEYDOWN, key, 0x001E0001); //send
    // PostMessage(hwnd, WM_KEYUP, key, 0x001E0001);   //send
    cout << "sended key " << ' ' << key << endl;
}

int MakeLParam(float x, float y)
{
    return int(y) << 16 | (int(x) & 0xFFFF);
}
// MouseMove function
void MouseMove(int x, int y)
{
    double fScreenWidth = GetSystemMetrics(SM_CXSCREEN) - 1;
    double fScreenHeight = GetSystemMetrics(SM_CYSCREEN) - 1;
    double fx = x * (65535.0f / fScreenWidth);
    double fy = y * (65535.0f / fScreenHeight);
    INPUT Input = {0};
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    Input.mi.dx = fx;
    Input.mi.dy = fy;
    SendInput(1, &Input, sizeof(INPUT));
}

void sendMouseDown(HWND hwnd, float x, float y)
{
    cout << x << ' ' << y << endl;
    INPUT Input = {0};
    MouseMove(int(x), int(y));

    // left down
    ZeroMemory(&Input, sizeof(INPUT));
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &Input, sizeof(INPUT));
    cout << "left mouse Down";

    // left up
    ZeroMemory(&Input, sizeof(INPUT));
    Input.type = INPUT_MOUSE;
    Input.mi.dwFlags = MOUSEEVENTF_LEFTUP | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &Input, sizeof(INPUT));
    cout << "left mouse up";

    // PostMessage(hwnd, WM_LBUTTONDOWN, 1, MakeLParam(x, y));
    // PostMessage(hwnd, WM_LBUTTONUP, 0, MakeLParam(x, y));
}

struct Mouse
{
    float x;
    float y;
    float relwidth;
    float relheight;
};

Mouse parseMousePos(string stPos)
{
    stringstream ss(stPos);

    string substr;
    getline(ss, substr, ',');
    float x = stof(substr);

    getline(ss, substr, ',');
    float y = stof(substr);

    getline(ss, substr, ',');
    float w = stof(substr);

    getline(ss, substr, ',');
    float h = stof(substr);

    return Mouse{x, y, w, h};
}

int main(int argc, char *argv[])
{
    int server = clientConnect();

    char *winTitle = "Road";
    if (argc > 1)
    {
        winTitle = argv[1];
    }
    cout << "Finding title " << winTitle << endl;
    HWND hwnd = getWindowByTitle(winTitle);

    cout << "Connected " << server << endl;
    cout << '\n'
         << "Press a key to continue..." << endl;
    getDesktopResolution(screenWidth, screenHeight);
    cout << "width " << screenWidth << "height " << screenHeight;
    do
    {

        int recv_size;
        char buf[2000];
        //Receive a reply from the server
        if ((recv_size = recv(server, buf, 1024, 0)) == SOCKET_ERROR)
        {
            puts("recv failed");
            continue;
        }
        if (recv_size == 0)
        {
            //cout << "Rejected: " << buffer << ' ' << strlen(buffer) << endl;
            continue;
        }
        char *buffer = new char[recv_size];
        memcpy(buffer, buf, recv_size);
        cout << "Got: " << buf << " Parsed: " << buffer << "recv size " << recv_size << " len " << strlen(buffer) << endl;
        if (recv_size > 1)
        {
            string st(buffer);
            cout << "Mouse: " << st << endl;
            Mouse pos = parseMousePos(st);
            float x = pos.x * screenWidth / pos.relwidth;
            float y = pos.y * screenHeight / pos.relheight;
            cout << "pos: " << x << ' ' << y << endl;
            sendMouseDown(hwnd, x, y);
        }
        else
        {
            // use key
            if (buffer[0] == 'p')
            {
                break;
            }
            sendIt(hwnd, buffer[0]); //notepad ID
            cout << '\n'
                 << "Press a key to continue...";
        }
    } while (true);
    closesocket(server);
    cout << "Socket closed." << endl
         << endl;

    WSACleanup();

    return 0;
}
