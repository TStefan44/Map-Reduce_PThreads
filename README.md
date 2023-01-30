    [ENG]
    Explanation data_thread:
    A structure to store the necessary information for a thread, from variables such as thread_id or output file name, to common variables such as the input file queue for mappers or the number of unopened files (these variables are allocated in main and saved in the structure as a pointer).

    ResultMap is a vector<vector<set<int>>> that can be considered as a matrix of sets, where:
    - number of rows = number of files => represents the file where the data was added from
    - number of columns = number of reducer threads => a column j is associated with power j + 2
    - the set at position (i, j) represents the perfect powers of exponent j found in file i.
    
    Implementation:
    - mapper + reducer threads all are created in the main functions. A barrier is used to force the mapper threads to work first, waiting for an equal number of threads, positioned after the mappers have finished working and before the reducers start working.
    - The distribution of input files is created dynamically. In a critical section, the logic to extract a file from the input file queue is implemented. If a map thread is not processing a file and there are still files in the queue, it will try to extract a new file and process it. Each map receives a unique id when choosing a file from the queue (= number of remaining files - 1), to directly add a line in the set<int> matrix and to avoid race condition.
    - the reducer threads go through the assigned column of the set<int> matrix. The sets are concatenated, and the final result represents the union size of the sets."
    
    [RO]
    Explicatie data_thread:
    structura in care salvez informatiile necesare pentru un thread, de la variabile propri precum thread_id sau denumirea fisierului de output, la variabile comune precum coada de fisiere de intrare pentru mappers sau numarul de fisiere nedeschise (aceste variabile sunt alocate in main, si salvate in structura ca pointer).
    
    ResultMap este un vector<vector<set<int>>>, care poate fi asemanat cu o matrice de set, unde:
    - nr de linii = nr de fisiere => reprezinta fisierul de unde au fost adaugate datele
    - nr de coloane = nr de threads reducer => o coloana j  este asociata cu puterea j + 2
    - set-ul de la pozitia (i, j) reprezinta puterile perfecte de exponent j, regasite in fisierul i.

    Implementare: 
    - sunt create mappers + reducers threads in aceiasi bucata de cod. Pentru a forta thread-urile mapper sa lucreze primele, se foloseste o bariera care asteapta un numar egal cu numarul de thread-uri, pozitionata dupa ce mappers au terminat de lucrat si inainte ca reducers sa inceapa sa lucreze.
    - distributia fisierelor de intrare este creata dinamic. Intr-o zona critica este implementata logica de extragere a unui fisier din coada de fisiere de intrare. Daca un thread map nu prelucreaza un fisier si mai sunt fisiere in coada, va incerca sa extraga un nou fisier si sa il prelucreze. Fiecare map primeste un id unic la alegerea unui fisier din coada (= nr de fisiere ramase - 1), pentru a adauga direct o linie in matricea de set<int> si pentru a evita race condition.
    - thread-urile reducers parcurg coloana atribuita din matricea de set<int>. Set-urile sunt concatenate, iar rezultatul final reprezinta dimensiunea uniunii de set-uri.
