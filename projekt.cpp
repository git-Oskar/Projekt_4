#include <windows.h>
#include <vector>

enum Ksztalt { BRAK_KSZTALTU, KWADRAT, TROJKAT, KOLKO };
enum TrybDzwigu { BRAK, KWADRATY_ONLY };

struct Element {
    Ksztalt ksztalt;
    int x, y; // Pozycja do rysowania
};

std::vector<Element> wiezaKwadratow;
TrybDzwigu trybDzwigu = BRAK;
Element aktualnyElement = { BRAK_KSZTALTU, -100, -100 };
bool pokazElementy = false;
bool pokazPrzenoszenie = false;
bool przesuniety = false;

// Animacja:
enum AnimState {
    ANIM_NONE,
    ANIM_ZNIŻANIE_DO_KLOCKA,
    ANIM_PODNOSZENIE_Z_KLOCKIEM,
    ANIM_PRZESUWANIE_W_PRAWO,
    ANIM_OPUSZCZANIE,
    ANIM_PODNOSZENIE,
    ANIM_WRACANIE
};
AnimState animStan = ANIM_NONE;

int animX = 50;   // Pozycja X haka
int animY = 200;  // Pozycja Y haka (w pionie)
const int hakStartY = 200;
int hakDoceloweY = 200; // docelowa wysokość haka przy opuszczaniu

// Pozycja klocka na ziemi (lewy dół ekranu)
int klocekStartX = 50;
int klocekStartY = 400;

bool pokazElementNaHak = false;  // czy rysować klocek na haku (podczas podnoszenia i przenoszenia)
bool klocekNaZiemi = true;       // czy klocek jest na ziemi (na start)

void RysujElement(HDC hdc, const Element& e) {
    switch (e.ksztalt) {
    case KWADRAT:
        Rectangle(hdc, e.x, e.y, e.x + 50, e.y + 50);
        break;
    case TROJKAT:
    {
        POINT p[3] = { {e.x + 25, e.y}, {e.x, e.y + 50}, {e.x + 50, e.y + 50} };
        Polygon(hdc, p, 3);
        break;
    }
    case KOLKO:
        Ellipse(hdc, e.x, e.y, e.x + 50, e.y + 50);
        break;
    default:
        break;
    }
}

void RysujDzwig(HDC hdc) {
    // Ramię dźwigu - pozioma linia od x=50 do x=450 na wysokości y=150
    MoveToEx(hdc, 50, 150, NULL);
    LineTo(hdc, 450, 150);

    // Pionowa linka - z ramienia w dół na pozycję y=animY
    MoveToEx(hdc, animX + 25, 150, NULL);
    LineTo(hdc, animX + 25, animY);

    // Hak - prostokąt pod linką
    Rectangle(hdc, animX + 10, animY, animX + 40, animY + 10);

    // Noga dźwigu - pionowa linia od (50, 150) do (50, 400)
    MoveToEx(hdc, 50, 150, NULL);
    LineTo(hdc, 50, 400);
}

void PokazPrzyciskiZadan(HWND hwnd) {
    CreateWindowW(L"BUTTON", L"Zad 4.1 (Kwadraty)", WS_VISIBLE | WS_CHILD, 10, 10, 180, 30, hwnd, (HMENU)(100), NULL, NULL);
    CreateWindowW(L"BUTTON", L"Zad 4.1 (trojkaty)", WS_VISIBLE | WS_CHILD, 200, 10, 180, 30, hwnd, (HMENU)(101), NULL, NULL);

}

void PokazKsztalty(HWND hwnd) {
    CreateWindowW(L"BUTTON", L"Kwadrat", WS_VISIBLE | WS_CHILD, 10, 50, 100, 30, hwnd, (HMENU)(200), NULL, NULL);
    CreateWindowW(L"BUTTON", L"Trojkat", WS_VISIBLE | WS_CHILD, 120, 50, 100, 30, hwnd, (HMENU)(201), NULL, NULL);
    CreateWindowW(L"BUTTON", L"Kolo", WS_VISIBLE | WS_CHILD, 230, 50, 100, 30, hwnd, (HMENU)(202), NULL, NULL);
    CreateWindowW(L"BUTTON", L"Przenoszenie", WS_VISIBLE | WS_CHILD, 10, 100, 150, 30, hwnd, (HMENU)(300), NULL, NULL);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        PokazPrzyciskiZadan(hwnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 100:
            trybDzwigu = KWADRATY_ONLY;
            pokazElementy = true;
            pokazPrzenoszenie = true;
            for (int i = 100; i <= 102; i++) DestroyWindow(GetDlgItem(hwnd, i));
            PokazKsztalty(hwnd);
            aktualnyElement = { BRAK_KSZTALTU, -100, -100 };
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case 200:
            aktualnyElement = { KWADRAT, klocekStartX, klocekStartY };
            przesuniety = false;
            klocekNaZiemi = true;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 201:
            aktualnyElement = { TROJKAT, klocekStartX, klocekStartY };
            przesuniety = false;
            klocekNaZiemi = true;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 202:
            aktualnyElement = { KOLKO, klocekStartX, klocekStartY };
            przesuniety = false;
            klocekNaZiemi = true;
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        case 300:
            if (aktualnyElement.ksztalt == BRAK_KSZTALTU) {
                MessageBoxW(hwnd, L"Najpierw wybierz ksztalt!", L"Uwaga", MB_OK | MB_ICONWARNING);
                break;
            }
            if (trybDzwigu == KWADRATY_ONLY && aktualnyElement.ksztalt != KWADRAT) {
                MessageBoxW(hwnd, L"Tylko kwadrat moze byc przeniesiony!", L"Blad", MB_OK | MB_ICONERROR);
                aktualnyElement = { BRAK_KSZTALTU, -100, -100 };
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }
            if (wiezaKwadratow.size() >= 3) {
                MessageBoxW(hwnd, L"Maksymalna wysokosc wiezy to 3!", L"Info", MB_OK | MB_ICONINFORMATION);
                break;
            }

            // Start animacji: najpierw zniżanie haka do klocka na ziemi
            animX = klocekStartX;
            animY = hakStartY;
            animStan = ANIM_ZNIŻANIE_DO_KLOCKA;
            pokazElementNaHak = false;
            SetTimer(hwnd, 1, 30, NULL);
            break;
        }
        break;

    case WM_TIMER:
        switch (animStan) {
        case ANIM_ZNIŻANIE_DO_KLOCKA:
            if (animY < klocekStartY) {
                animY += 5;  // zniżamy hak do klocka
                if (animY > klocekStartY) animY = klocekStartY;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                animStan = ANIM_PODNOSZENIE_Z_KLOCKIEM;
                pokazElementNaHak = true;
                klocekNaZiemi = false;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_PODNOSZENIE_Z_KLOCKIEM:
            if (animY > hakStartY) {
                animY -= 5; // podnosimy hak z klockiem
                if (animY < hakStartY) animY = hakStartY;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                animStan = ANIM_PRZESUWANIE_W_PRAWO;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_PRZESUWANIE_W_PRAWO:
            if (animX < 400) {
                animX += 10;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                // Oblicz docelową wysokość haka - na podstawie wielkości wieży
                int baseY = 400 - static_cast<int>(wiezaKwadratow.size()) * 60;
                hakDoceloweY = baseY;
                animStan = ANIM_OPUSZCZANIE;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_OPUSZCZANIE:
            if (animY < hakDoceloweY) {
                animY += 5;  // opuszczamy hak (w dół)
                if (animY > hakDoceloweY) animY = hakDoceloweY;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                // Odłóż klocek na wieżę (pozycja animX=400, y=hakDoceloweY)
                wiezaKwadratow.push_back({ KWADRAT, 400, hakDoceloweY });
                pokazElementNaHak = false;  // klocek już jest odłożony
                aktualnyElement = { BRAK_KSZTALTU, -100, -100 };
                animStan = ANIM_PODNOSZENIE;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_PODNOSZENIE:
            if (animY > hakStartY) {
                animY -= 5; // podnosimy hak (w górę)
                if (animY < hakStartY) animY = hakStartY;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                animStan = ANIM_WRACANIE;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_WRACANIE:
            if (animX > 50) {
                animX -= 10; // cofamy się w lewo
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                animStan = ANIM_NONE;
                KillTimer(hwnd, 1);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ANIM_NONE:
        default:
            break;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        RysujDzwig(hdc);

        // Rysuj klocek na ziemi, jeśli jest i nie został podniesiony
        if (klocekNaZiemi && aktualnyElement.ksztalt != BRAK_KSZTALTU) {
            RysujElement(hdc, aktualnyElement);
        }

        // Rysuj klocek na haku podczas animacji podnoszenia/przenoszenia
        if ((animStan == ANIM_PODNOSZENIE_Z_KLOCKIEM || animStan == ANIM_PRZESUWANIE_W_PRAWO || animStan == ANIM_OPUSZCZANIE) && pokazElementNaHak) {
            Element animElem = { KWADRAT, animX, animY };
            RysujElement(hdc, animElem);
        }

        for (const auto& e : wiezaKwadratow) {
            RysujElement(hdc, e);
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"DzwigSim";
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(wc.lpszClassName, L"Symulator Dzwigu - Zadanie 4.1", WS_OVERLAPPEDWINDOW, 100, 100, 800, 600, NULL, NULL, hInstance, NULL);
    ShowWindow(hwnd, nCmdShow);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
