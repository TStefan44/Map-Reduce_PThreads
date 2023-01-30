#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <queue>
#include <vector>
#include <set>
#include <fstream>
#include <math.h>
using namespace std;

#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

typedef std::vector<std::set<int>> map_res;

/**
 * @brief struct used to parse need data for threads
 */
struct thread_data {
    unsigned int thread_id;
    unsigned int mappers;  // num of maper threads
    unsigned int reducers; // num of reducer threads
    char outfile[10];
    int *files_num;
    std::queue<std::string> *files; // queue with files
    std::vector<map_res> *resultMap; // matrix of perfect power list
    pthread_barrier_t *barrier;
    pthread_mutex_t *mutex;

    thread_data(unsigned int thread_id, unsigned int mappers, unsigned int reducers, int *files_num,
                std::queue<std::string> *files, std::vector<map_res> *resultMap,
                pthread_barrier_t *barrier, pthread_mutex_t *mutex) :
        thread_id(thread_id), mappers(mappers), reducers(reducers), files_num(files_num),
        files(files), resultMap(resultMap), barrier(barrier), mutex(mutex) {
            sprintf(outfile, "out%d.txt", thread_id - mappers + 2);
        }
};

/**
 * @brief Create a queue wich contains names of files, to be 
 * used by mappers, from main input file
 */
std::queue<std::string> read_input(const char *input_name, int &files_num) {
    std::ifstream fin(input_name);
    DIE(fin.fail(), "open failed");
    
    char file_name[50];
    std::queue<std::string> my_queue;

    fin >> files_num;

    for (int i = 0; i < files_num; i++) {
        fin >> file_name;
        my_queue.push(std::string(file_name));
    }

    fin.close();
    return my_queue;
}

/**
 * @brief Calculate n-rooth using Newton Raphson Method:
 * x(K+1) = x(K) – f(x) / f’(x), where 
 * f(x)  = x^(power) – base and
 * f'(x) = power * x^(power-1)
 */
inline double nRooth(int base, int power) {
    double pre = 1 + rand() % 9; // previous value
    double eps = 1e-3; // precision
    double del = INT32_MAX; // diference between 2 roots
    double xk; // current value

    while (del > eps) {
        xk = ((power - 1.0) * pre + (double)base / pow(pre, power - 1)) / (double)power;
        del = abs(xk - pre);
        pre = xk;
    }

    return xk;
}

/**
 * @brief Map function. Open given file, and for each number y, calculate
 * x = floor(y ^ 1 / j), verify if y = pow(x, j), and save y in a set if
 * true 
 */
inline void map_func(const char *file, thread_data *data, int id) {

    std::ifstream fin(file);
    DIE(fin.fail(), "open failed");

    int numbers;
    (*data->resultMap)[id] = map_res(data->reducers, std::set<int>());

    fin >> numbers;
    for (int i = 0; i < numbers; i++) {
        int a;
        fin >> a;

        // ignore case a = 0.
        if (a == 0)
            continue;

        // verify if a is a power with exponent = j
        for (unsigned int j = 2; j <= data->reducers + 1; j++) {
            float resRooth = nRooth(a, j);
            if (a == pow (floor(resRooth), j)) {
                (*data->resultMap)[id][j - 2].insert(a);
            }
        }
    }

    fin.close();
}

/**
 * @brief Thread function. Dynamically distribute filles to mapper:
 * when a mapper thread is free, it tries to work on another unopened file.
 */
void *f(void *arg) {
    thread_data data = *(thread_data *)arg;
    std::set<int> uniqueNum;
    std::string file;
    int id;

    // Mapper
    if (data.thread_id < data.mappers) {
        while(data.files->empty() == false) {
            // Critic code zone. Take an unopened file from queue
            pthread_mutex_lock(data.mutex);

            if(data.files->empty() == true) {
                pthread_mutex_unlock(data.mutex);
                break;
            }

            // Extract file from queue
            file = data.files->front();
            data.files->pop();
            *(data.files_num) = *(data.files_num) - 1;
            id = *(data.files_num);

            pthread_mutex_unlock(data.mutex);

            // Mapper has a file to open.
            map_func(file.c_str(), &data, id);

        }
    }
    
    // Barrier for syncronisation
    pthread_barrier_wait(data.barrier);

    // Reducer
    if (data.thread_id >= data.mappers) {

        for (unsigned int i = 0 ; i < (*data.resultMap).size(); i++) {
            // Choose set which repres. the same exponent as the thread 
            std::set<int> pows = (*data.resultMap)[i][data.thread_id - data.mappers];

            // Put values in a set, to get rid of duplicates
            uniqueNum.insert(pows.begin(), pows.end());
        }

        // Print thread result
        ofstream fout(data.outfile);
        fout << uniqueNum.size();

        // Close file
        fout.close();
    }

    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    srand(time(0));

    // Input variables
    int mapers_num, reducers_num;
    char input_name[50];
    int files_num;
    std::queue<std::string> files;

    // thread variables
    std::vector<map_res> resultMap;
    std::vector<pthread_t> threads;
    std::vector<thread_data*> datas;
    pthread_barrier_t barrier;
    pthread_mutex_t mutex;
    void *status;
    int error;

    if (argc != 4) {
        fprintf(stderr, "Wrong input parameters. Given %d parameters! Need 3 parameters\n", argc - 1);
    }

    // Parse input data from command line
    mapers_num = atoi(argv[1]);
    reducers_num = atoi(argv[2]);
    strcpy(input_name, argv[3]);
    files = read_input(input_name, files_num);

    // Create and join threads
    threads = std::vector<pthread_t>(mapers_num + reducers_num, 0);
    datas = std::vector<thread_data*>(mapers_num + reducers_num, NULL);
    resultMap = std::vector<map_res>(files_num, map_res());

    // Init barrier and mutex
    pthread_barrier_init(&barrier, NULL, mapers_num + reducers_num);
    pthread_mutex_init(&mutex, NULL);

    for (int i = 0; i < mapers_num + reducers_num; i++) {
        // Create thread and it's data
        datas[i] = new thread_data(i, mapers_num, reducers_num, &files_num, &files, &resultMap, &barrier, &mutex);
		error = pthread_create(&threads[i], NULL, f, datas[i]);

		if (error) {
            fprintf(stderr, "Error when creating thread %d\n", i);
			exit(-1);
		}
	}

	for (int i = 0; i < mapers_num + reducers_num; i++) {
		error = pthread_join(threads[i], &status);

		if (error) {
			fprintf(stderr, "Error when waiting for thread %d\n", i);
			exit(-1);
		}
	}

    // Destroy barrier and mutex
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);

    // Delete datas thread
    for (int i = 0; i < mapers_num + reducers_num; i++) {
        delete datas[i];
    }

    return 0;
}