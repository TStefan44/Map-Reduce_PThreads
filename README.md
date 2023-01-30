    Explicatie data_thread:
    structura in care salvez informatiile necesare pentru
un thread, de la variabile prorpi precum thread_id sau denumirea fisierului de
output, la variabile comune precum coada de fisiere de intrare pentru mappers sau
numarul de fisiere nedeschise (aceste variabile sunt alocate in main, si salvate
in structura ca pointer).
    resultMap este un vector<vector<set<int>>>, care poate fi asemanat cu o matrice
de set, unde:
    - nr de linii = nr de fisiere => reprezinta fisierul de unde au fost adaugate
datele
    - nr de coloane = nr de threads reducer => o coloana j  este asociata cu puterea
j + 2
    - set-ul de la pozitia (i, j) reprezinta puterile perfecte de exponent j, regasite
in fisierul i.

    Implementare:
    - sunt create mappers + reducers threads in aceiasi bucata de cod. Pentru a forta
thread-urile mapper sa lucreze primele, se foloseste o bariera care asteapta un numar
egal cu numarul de thread-uri, pozitionata dupa ce mappers au terminat de lucrat si
inainte ca reducers sa inceapa sa lucreze.
    - distributia fisierelor de intrare este creata dinamic. Intr-o zona critica
este implementata logica de extragere a unui fisier din coada de fisiere de intrare.
Daca un thread map nu prelucreaza un fisier si mai sunt fisiere in coada, va incerca
sa extraga un nou fisier si sa il prelucreze. Fiecare map primeste un id unic la alegerea
unui fisier din coada (= nr de fisiere ramase - 1), pentru a adauga direct o linie in matricea
de set<int> si pentru a evita race condition.
    - thread-urile reducers parcurg coloana atribuita din matricea de set<int>. Set-urile
sunt concatenate, iar rezultatul final reprezinta dimensiunea uniunii de set-uri.
