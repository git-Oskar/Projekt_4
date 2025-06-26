#include <windows.h>
#include <vector>
#include <iostream>

enum Ksztalt { BRAK_KSZTALTU, KWADRAT, TROJKAT, KOLKO };
enum TrybDzwigu {
    BRAK,
    KWADRATY_ONLY,
    TROJKATY_ONLY,
    KOLKA_ONLY,
    OGRANICZENIE_MASY,
    AUTO_WIEZA_1,
    AUTO_WIEZA_2
};
struct Element {
    Ksztalt ksztalt;
    int x, y; // Pozycja do rysowania
    int waga;
};

std::vector<Element> wiezaElementow;
std::vector<std::vector<Element>> pole(7, std::vector<Element>(0)); // tablica zapisujaca ksztalty znajdujace sie miedzy ziemia a dzwigiem, 7 kolumn po elementy
TrybDzwigu trybDzwigu = BRAK;
std::vector<Element> naHaku;
Element aktualnyElement = { BRAK_KSZTALTU, -100, -100, 0 };
bool pokazElementy = false;
bool pokazPrzenoszenie = false;
bool przesuniety = false;

// Animacja:
enum AnimState {
    ANIM_NONE,
    ANIM_ZNIZANIE_DO_KLOCKA,        // Poprawiono: usunięto polski znak "Ż"
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

void RysujSkale(HDC hdc) {
        int r[] = {255, 230, 200, 180, 150, 120 ,100 ,70 ,30,0} , g [] = {0,70,150,200,255,230,180,120,70,0}, b[] = {0,30,70,100,120,150,180,200,230,255};
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // czarny kontur
    SelectObject(hdc, hPen);
    TextOutW(hdc, 680, 10, L"Skala masy", 10);
    for(int i = 0; i < 10; ++i) {
         HBRUSH hBrush = CreateSolidBrush(RGB(r[i], g[i], b[i]));
         SelectObject(hdc, hBrush);
        Rectangle(hdc, 700 , 30 + i * 30, 730, 60 + i * 30);
        wchar_t num[2] = {};
        num [0] = L'1' + i; // zamiana liczby na znak
        num[1] = L'\0';
        if(i == 9)
        {
        num[0] = L'1';
        num[1] = L'0';
    
        }
        TextOutW(hdc, 735, 30 + i * 30, &num[0], 2);
         DeleteObject(hBrush);
    }
   
    DeleteObject(hPen);
}

void RysujElement(HDC hdc, const Element& e) {
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); // czarny kontur
    SelectObject(hdc, hPen);
    int masa = e.waga - 1; // masa elementu 
    int r[] = {255, 230, 200, 180, 150, 120 ,100 ,70 ,30,0} , g [] = {0,70,150,200,255,230,180,120,70,0}, b[] = {0,30,70,100,120,150,180,200,230,255};
    HBRUSH hBrush = CreateSolidBrush(RGB(r[masa], g[masa], b[masa])); 
    SelectObject(hdc, hBrush);



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
    klocekNaZiemi = false;
    DeleteObject(hBrush);
    DeleteObject(hPen);
}

void WypelnijPole(HDC hdc) {
    for (int i = 0; i < 7; ++i) {
        int y = 400 - static_cast<int>(pole[i].size()) * 50;
        for (const auto& e : pole[i]) {
            RysujElement(hdc, e);
        }
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

    if(!naHaku.empty())
    {
        naHaku.back().x = animX;
        naHaku.back().y = animY; // Ustawiamy pozycję X ostatniego elementu na haku
        RysujElement(hdc, naHaku.back()); // Rysujemy ostatni element na haku
    }
}


void PokazWaga(HWND hwnd)
{
    for(int i = 0 ; i < 6; ++i){
       HWND hEdit = CreateWindowW(
    L"EDIT",           // Klasa kontrolki
    L"Waga",               // Tekst początkowy
    WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
    50 + 60 * i, 460, 50, 20,  // Pozycja i rozmiar
    hwnd,              // Okno nadrzędne
    (HMENU)(500 + i),       // ID kontrolki
    NULL, NULL
);
   HFONT hFont = CreateFontW(
        15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );

    SendMessage(hEdit, WM_SETFONT, WPARAM(hFont), TRUE);
}
}

void PokazListy(HWND hwnd)
{
    for(int i = 0 ; i < 6 ; ++ i)
    {
        HWND hList = CreateWindowW(
            L"COMBOBOX",  // Klasa kontrolki
            L"KSZTAŁT",   // <-- poprawka tutaj
            WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
            50 + 60 * i, 490, 50, 120,  // Pozycja i rozmiar
            hwnd,
            (HMENU)(600 + i),  // ID kontrolki
            NULL, NULL
        );
        SendMessageW(hList, CB_ADDSTRING, 0, (LPARAM)L"Kwadrat");
        SendMessageW(hList, CB_ADDSTRING, 0, (LPARAM)L"Trojkat");
        SendMessageW(hList, CB_ADDSTRING, 0, (LPARAM)L"Kolo");
        SendMessageW(hList, CB_SETCURSEL, 0, 0);
    }
}

void PokazPlus(HWND hwnd)
{
    for(int i = 0 ; i < 6 ; i ++)
    {
        CreateWindowW(L"BUTTON", L"+", WS_VISIBLE | WS_CHILD, 50 + 60 * i, 520, 50, 20, hwnd, (HMENU)(700 + i), NULL, NULL);
    }
}

void PokazPrzyciskiZadan(HWND hwnd) {
    CreateWindowW(L"BUTTON", L"Wybierz tryb klocka:", WS_VISIBLE | WS_CHILD | BS_GROUPBOX,
        300, 10, 220, 100, hwnd, NULL, NULL, NULL);

    CreateWindowW(L"BUTTON", L"Kwadraty", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        300, 30, 200, 20, hwnd, (HMENU)100, NULL, NULL);
    CreateWindowW(L"BUTTON", L"Trojkaty", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        300, 50, 200, 20, hwnd, (HMENU)101, NULL, NULL);
    CreateWindowW(L"BUTTON", L"Kolka", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        300, 70, 200, 20, hwnd, (HMENU)102, NULL, NULL);
    CreateWindowW(L"BUTTON", L"Wszystkie ksztalty", WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,
        300, 90, 200, 20, hwnd, (HMENU)103, NULL, NULL);
    CreateWindowW(L"BUTTON", L"RESET", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 300, 110, 200, 20, hwnd, (HMENU)106, NULL, NULL);
}

// void PokazKsztalty(HWND hwnd) {
//     CreateWindowW(L"BUTTON", L"Kwadrat", WS_VISIBLE | WS_CHILD, 10, 50, 100, 30, hwnd, (HMENU)(200), NULL, NULL);
//     CreateWindowW(L"BUTTON", L"Trojkat", WS_VISIBLE | WS_CHILD, 120, 50, 100, 30, hwnd, (HMENU)(201), NULL, NULL);
//     CreateWindowW(L"BUTTON", L"Kolo", WS_VISIBLE | WS_CHILD, 230, 50, 100, 30, hwnd, (HMENU)(202), NULL, NULL);
//     CreateWindowW(L"BUTTON", L"Przenoszenie", WS_VISIBLE | WS_CHILD, 10, 100, 150, 30, hwnd, (HMENU)(300), NULL, NULL);
// }
void PokazSterowanie(HWND hwnd) {
    CreateWindowW(L"STATIC", L"Sterowanie hakiem:", WS_VISIBLE | WS_CHILD, 10, 70, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindowW(L"BUTTON", L"<-", WS_VISIBLE | WS_CHILD, 10, 100, 50, 30, hwnd, (HMENU)(400), NULL, NULL);
    CreateWindowW(L"BUTTON", L"->", WS_VISIBLE | WS_CHILD, 70, 100, 50, 30, hwnd, (HMENU)(401), NULL, NULL);
    CreateWindowW(L"BUTTON", L"^", WS_VISIBLE | WS_CHILD, 130, 100, 50, 30, hwnd, (HMENU)(402), NULL, NULL);
    CreateWindowW(L"BUTTON", L"v", WS_VISIBLE | WS_CHILD, 190, 100, 50, 30, hwnd, (HMENU)(403), NULL, NULL);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    
    if(msg == WM_COMMAND && 700 <= LOWORD(wParam) &&  706 >= LOWORD(wParam)) // po wcisnieciu plusa dodaje sie nowa figura w okreslonym przez indeks miejscu
    {
        int index = LOWORD(wParam) - 700; 
        if (index >= 0 && index <= 6) 
        {
            Element element;
            HWND hList = GetDlgItem(hwnd, 600 + index);
            int sel = (int)SendMessageW(hList, CB_GETCURSEL, 0, 0);
            if (sel == CB_ERR) {
                MessageBoxW(hwnd, L"Nie wybrano ksztaltu!", L"Uwaga", MB_OK | MB_ICONWARNING);
                return 0;
            }
            // Map selection index to Ksztalt enum
            switch (sel) {
                case 0: element.ksztalt = KWADRAT; break;
                case 1: element.ksztalt = TROJKAT; break;
                case 2: element.ksztalt = KOLKO; break;
                default: element.ksztalt = BRAK_KSZTALTU; break;
            }
            HWND hWaga = GetDlgItem(hwnd, 500 + index);
            wchar_t buf[16] = {0};
            GetWindowTextW(hWaga, buf, 15);
            int waga = _wtoi(buf);
            element.waga = waga;
            element.x = 50 + index * 60; // Pozycja X w zaleznosci od indeksu
            element.y = 400 - static_cast<int>(pole[index].size()) * 50; // Pozycja Y w zaleznosci od liczby elementów w kolumnie
            pole[index].push_back(element);
            if(pole[index].size() > 3)
            {
                MessageBoxW(hwnd, L"Przekroczono maksymalna wysokosc wiezy w tej kolumnie!", L"Uwaga", MB_OK | MB_ICONWARNING);
                pole[index].pop_back(); // Usuwamy ostatni element
                return 0;
            }
            aktualnyElement = element;
        }
        klocekNaZiemi = true; // Klocek jest na ziemi
        msg = WM_PAINT; // Wymuszenie odświeżenia okna
        InvalidateRect(hwnd, NULL, TRUE);

    }
    
    switch (msg) {
    case WM_CREATE:
      //  PokazPrzyciskiZadan(hwnd);
        PokazPrzyciskiZadan(hwnd);
        PokazSterowanie(hwnd);
        PokazWaga(hwnd);
        PokazListy(hwnd);
        PokazPlus(hwnd);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 100: trybDzwigu = KWADRATY_ONLY; break;
        case 101: trybDzwigu = TROJKATY_ONLY; break;
        case 102: trybDzwigu = KOLKA_ONLY; break;
        case 103: trybDzwigu = OGRANICZENIE_MASY; break;
        case 104: trybDzwigu = AUTO_WIEZA_1; break;
        case 105: trybDzwigu = AUTO_WIEZA_2; break;
        case 106:
            wiezaElementow.clear();
            pole = std::vector<std::vector<Element>>(7); 
            naHaku.clear();
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        default: break;
        }



        switch (LOWORD(wParam)) {
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
      
        case 400: // <-
            if (animStan == ANIM_NONE && animX > 50) {
                animX -= 10;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case 401: // ->
            if (animStan == ANIM_NONE && animX < 450) {
                animX += 10;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        case 402: // ^

        if(!naHaku.empty())
            {
                MessageBoxW(hwnd, L"Najpierw opusc zawartość haka!", L"Uwaga", MB_OK | MB_ICONWARNING);
            break;
            }
             if(pole[(animX - 20) / 60].empty())
        {
            MessageBoxW(hwnd, L"Najpierw wybierz ksztalt!", L"Uwaga", MB_OK | MB_ICONWARNING);
            break;
        }
        else
            aktualnyElement = pole[(animX - 20) / 60].back();
            if (aktualnyElement.ksztalt == BRAK_KSZTALTU) {
                MessageBoxW(hwnd, L"Najpierw wybierz ksztalt!", L"Uwaga", MB_OK | MB_ICONWARNING);
                break;
            }

            // Walidacja trybu
            if ((trybDzwigu == KWADRATY_ONLY && aktualnyElement.ksztalt != KWADRAT) ||
                (trybDzwigu == TROJKATY_ONLY && aktualnyElement.ksztalt != TROJKAT) ||
                (trybDzwigu == KOLKA_ONLY && aktualnyElement.ksztalt != KOLKO)) {
                MessageBoxW(hwnd, L"Ten ksztalt nie moze byc przeniesiony w tym trybie!", L"Blad", MB_OK | MB_ICONERROR);
                aktualnyElement = { BRAK_KSZTALTU, -100, -100 ,0};
                InvalidateRect(hwnd, NULL, TRUE);
                break;
            }

            if (trybDzwigu == OGRANICZENIE_MASY || trybDzwigu == AUTO_WIEZA_2) {
                int masa = aktualnyElement.waga;//MasaKsztaltu(aktualnyElement.ksztalt);
                if (masa > 6) {
                    MessageBoxW(hwnd, L"Klocek za ciezki! (masa > 6)", L"Blad", MB_OK | MB_ICONERROR);
                    aktualnyElement = { BRAK_KSZTALTU, -100, -100 ,0};
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                }
            }

            if (wiezaElementow.size() >= 3) {
                MessageBoxW(hwnd, L"Maksymalna wysokosc wiezy to 3!", L"Info", MB_OK | MB_ICONINFORMATION);
                break;
            }
             
            klocekStartY = 440 - static_cast<int>(pole[(animX - 20) / 60].size()) * 50; 
            animStan = ANIM_ZNIZANIE_DO_KLOCKA; // Poprawiono: usunięto polski znak "Ż"
            pokazElementNaHak = false;
            klocekNaZiemi = true;
            
            SetTimer(hwnd, 1, 30, NULL);
            break;
            break;

          case 403:

            if(naHaku.empty())
            {
                MessageBoxW(hwnd, L"Masz pusty hak!", L"Uwaga", MB_OK | MB_ICONWARNING);
            break;
            }
            if(pole[(animX - 20) / 60].size()>3)
        {
            MessageBoxW(hwnd, L"Przekroczono maksymalna wysokosc wierzy w tej kolumnie!", L"Uwaga", MB_OK | MB_ICONWARNING);
            break;
        }
        else
            aktualnyElement = naHaku.back();
            if (aktualnyElement.ksztalt == BRAK_KSZTALTU) {
                MessageBoxW(hwnd, L"Najpierw wybierz ksztalt!", L"Uwaga", MB_OK | MB_ICONWARNING);
                break;
            }

            // Walidacja trybu
            int baseY = 400 - static_cast<int>(pole[(animX - 20) / 60].size()) * 50;
            hakDoceloweY = baseY;
            animStan = ANIM_OPUSZCZANIE; // Poprawiono: usunięto polski znak "Ż"
            pokazElementNaHak = false;
            klocekNaZiemi = true;
            SetTimer(hwnd, 1, 30, NULL);
            break;
        
        }
        break;

    case WM_TIMER:
        switch (animStan) {
        case ANIM_ZNIZANIE_DO_KLOCKA: // Poprawiono: usunięto polski znak "Ż"
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
                pole[(animX - 20) / 60].pop_back();
                 naHaku.push_back(aktualnyElement);
            }
            break;

        case ANIM_PODNOSZENIE_Z_KLOCKIEM:
                
            if (animY > hakStartY) {
                
                animY -= 5; // podnosimy hak z klockiem
                if (animY < hakStartY) 
                animY = hakStartY;
            InvalidateRect(hwnd, NULL, TRUE);
            }
            else
            {
                animStan = ANIM_NONE;
                KillTimer(hwnd, 1);
            }
            break;

        case ANIM_PRZESUWANIE_W_PRAWO:
            if (animX < 400) {
                animX += 10;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                // Oblicz docelową wysokość haka - na podstawie wielkości wieży
                int baseY = 400 - static_cast<int>(pole[(animX - 20) / 60].size()) * 50;
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
                aktualnyElement.x = animX; 
                aktualnyElement.y = hakDoceloweY; 
                pole[(animX - 20) / 60].push_back(aktualnyElement);
                aktualnyElement.x = animX; // Ustawiamy pozycję X klocka na wieży
                naHaku.pop_back(); // usuwamy klocek z haka
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
                InvalidateRect(hwnd, NULL, TRUE);
                KillTimer(hwnd, 1);
                animStan = ANIM_NONE;


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

        // Utwórz bufor w pamięci (double buffering)
        RECT rc;
        GetClientRect(hwnd, &rc);
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBM = CreateCompatibleBitmap(hdc, rc.right - rc.left, rc.bottom - rc.top);
        HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

        FillRect(memDC, &rc, (HBRUSH)(COLOR_WINDOW + 1));

        RysujDzwig(memDC);
        RysujSkale(memDC);

        if (klocekNaZiemi && aktualnyElement.ksztalt != BRAK_KSZTALTU) {
            RysujElement(memDC, aktualnyElement);
        }

        if ((animStan == ANIM_PODNOSZENIE_Z_KLOCKIEM || animStan == ANIM_PRZESUWANIE_W_PRAWO || animStan == ANIM_OPUSZCZANIE) && pokazElementNaHak) {
            Element animElem = { aktualnyElement.ksztalt, animX, animY , aktualnyElement.waga };
            RysujElement(memDC, animElem);
        }

        for (const auto& e : wiezaElementow) {
            RysujElement(memDC, e);
        }

        WypelnijPole(memDC);

        // Przenieś bufor na ekran
        BitBlt(hdc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, memDC, 0, 0, SRCCOPY);

        // Sprzątanie
        SelectObject(memDC, oldBM);
        DeleteObject(memBM);
        DeleteDC(memDC);

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
