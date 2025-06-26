# TECHNIKI PROGRAMOWANIA - projekt 4

### Oskar Belczewski 203182
### Kacper Fydrych 204012

Wykorzystane biblioteki:
- Windows GDI+ (c++)

# Mechanizm działania oraz wizualizacja dźwigu
Hak dźwigu można poruszać się poziomo oraz można podnosić i odkładać różne elementy.
Pojemność haka jest tylko na 1 element, a takowy można pojawić poprzez wybranie okienka ze strzałką skierowaną w dół, wybranie elementu oraz naciśnięcie przycisku z “+”. To spowoduje pojawienie się elementu w konkretnej kolumnie.
Nad samym dźwigiem są przyciski do kontroli dźwigiem, które pozwalają użytkownikowi poruszanie się hakiem, podnoszenie oraz opuszczanie elementów na haku.
Oczywiście w przypadku próby podniesienia, gdy nie ma elementu pod hakiem  do podniesienia to akcja zostanie przerwana i pojawi się komunikat o braku elementu do podniesienia.

![dzwig.png](attachment:dzwig.png) ![kierunki_ruchu.png](attachment:kierunki_ruchu.png) ![wybor.png](attachment:wybor.png)

# Mechanizm tylko 1 typu elementu

W celu możliwości ustawienia możliwego typu elementu do podnoszenia  dodaliśmy opcję wyboru kształtu przyjmowanego, przez hak. Można wybrać między trybami  “kwadraty”,”trojkaty” lub “kolka”, bądź “wszystkie kształty”. Pod nimi znajduje się przycisk “RESET”, który usuwa wszystkie pojawione elementy.
Przy wybraniu elementu spoza ustawionego trybu wyskoczy komunikat o niemożności podniesienia elementu przez hak.

![tryby.png](attachment:tryby.png)

# Mechanizm sprawdzający masę elementu
W celu możliwości ustawienia maksymalnej, dopuszczalnej masy elementu dodaliśmy opcję ustawienia go, poprzez zaznaczenie na suwaku, który znajduje się po prawej stronie ekranu.
Pod każdą kolumną dodaliśmy okienko z napisem “Waga”, gdzie można wpisać wagę elementu w zakresie 1-10. Jeżeli pojawimy element, z wcześniej ustaloną wagą, to zmieni ten element kolor w zależności od ustalonej wagi. 
Legenda barw jest zawarta na suwaku.

![skala.png](attachment:skala.png) ![kolory_wagowe.png](attachment:kolory_wagowe.png)
