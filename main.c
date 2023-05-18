#include <my_utils.h>
#include <my_memory.h>
#include <my_dir.h>
#include <my_thread.h>
#include <my_socket.h>
#include <thread_safe_queue.h>

#include <sys/wait.h>

#define forever for (;;)
#define MAX_PATH_LEN 255
#define BUF_SIZE 300
#define ADDRESS NULL
#define PORT 4444
#define MAX_CONNECTION 10

char path[MAX_PATH_LEN];    // argv[1]
int W;                      // argv[2]

Thread_Safe_Queue queue;    // coda di path
char **all_paths;           // array contenente tutti i path utilizzati
size_t all_paths_len;

Thread *workers;            // client
Thread *handlers;           // gestori client

int *client_fds;            // array contenente tutti i file descriptor dei client

void fetch_args(const char **);
void init_main_workspace(void);
int collector(void);
void init_collector_workspace(void);
void explore(char *);

void *worker(void *);
void *handler(void *);

int main(int argc, char const *argv[])
{
    // Controllo il numero di argomenti: ./prog path W
    if (argc < 3)
    {
        errno = EINVAL;
        Perror("Argomenti insufficienti");
    }

    // Ottengo i dati da riga di comando
    fetch_args(argv);

    // Inizializzo le variabili
    init_main_workspace();

    // Lancio il collector
    if (Fork() == 0)
        return collector();

    // Visito la cartella
    explore(path);

    // Segnale di terminazione worker
    TSQPush(&queue, NULL);

    // Attendo i worker
    for (size_t i = 0; i < W; i++)
        Join(workers[i], NULL);

    // Libero la memoria
    Free(workers);
    TSQDestroy(&queue);

    for (size_t i = 0; i < all_paths_len; i++)
        Free(all_paths[i]);
    Free(all_paths);

    // Attendo il server (per avere un output "pulito")
    wait(NULL);

    return 0;
}

void fetch_args(const char **args)
{
    // Ottengo il path
    strncpy(path, args[1], MAX_PATH_LEN);

    // Ottengo il numero di worker
    W = abs(atoi(args[2]));
}

void init_main_workspace(void)
{
    // Inizializzo la coda
    TSQInit(&queue);

    // Inizializzo l'array di file_name
    all_paths = (char **)Malloc(sizeof(char *));
    all_paths_len = 0;

    // Inizializzo l'array di worker
    workers = (Thread *)Malloc(sizeof(Thread) * W);

    // Creo i thread worker
    for (size_t i = 0; i < W; i++)
        TCreate(&workers[i], worker, NULL);
}

void init_collector_workspace(void)
{
    // Inizializzo l'array di gestori
    handlers = (Thread *)Malloc(sizeof(Thread) * W);

    // Inizializzo l'array di file descriptor dei client
    client_fds = (int *)Malloc(sizeof(int) * W);
}

void explore(char *dirname)
{
    // Apro la cartella
    DIR *dirptr = Dopen(dirname);

    // Caso base
    if (Dempty(dirptr))
    {
        Dclose(dirptr);
        return;
    }

    // Sposto il puntatore all'inzio della directory
    Drewind(dirptr);

    struct dirent *entry;

    while ((entry = Dread(dirptr)) != NULL)
    {
        // Trovata la cartella . o ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Aggiorno l'array di tutti i path
        all_paths_len++;
        all_paths = (char **)Realloc(all_paths, sizeof(char *) * all_paths_len);
        all_paths[all_paths_len - 1] = (char *)Malloc(sizeof(char) * MAX_PATH_LEN);

        // Aggiungo all'array il nome dell'ultima entry trovata
        sprintf(all_paths[all_paths_len - 1], "%s/%s", dirname, entry->d_name);

        // Trovata cartella diversa da . e ..
        if (is_dir(all_paths[all_paths_len - 1]))
            explore(all_paths[all_paths_len - 1]);

        // Trovato file .dat
        if (is_reg(all_paths[all_paths_len - 1]) && strstr(entry->d_name, ".dat") != NULL)
            TSQPush(&queue, all_paths[all_paths_len - 1]);
    }

    // Chiudo la cartella
    Dclose(dirptr);
}

void *worker(void *args)
{
    char **buffer;                  // Buffer per la pop dalla coda
    FILE *file;                     

    char msg[BUF_SIZE];             // Buffer per la send
    memset(msg, 0, BUF_SIZE);       

    size_t n = 0;                   // Numero di float trovati
    float avg = 0;                  // Media dei float trovati
    float std = 0;                  // Deviazione standard dei float trovati

    float number;                   // Buffer per il numero corrente

    int client_fd;                  // File descriptor del client corrente
    Address server_address;         // Indirizzo del server

    // Inizializzo l'indirizzo
    SetAddr(&server_address, ADDRESS, PORT);

    // Creo il socket del client
    Socket(&client_fd);

    // Connetto il client al server
    Connect(client_fd, &server_address);

    // Inizializzo il buffer per i path
    buffer = (char **)Malloc(sizeof(char *));

    forever
    {
        TSQPop(&queue, (void **)buffer);

        // Gestisco la terminazione (fine visita)
        if (*buffer == NULL)
        {
            // Propago il segnale di terminazione agli altri thread client attivi
            TSQPush(&queue, NULL);

            // Termino il thread del server che gestisce la connessione e chiudo la connessione
            Send(client_fd, "close", 6);
            Close(client_fd);

            // Libero la memoria e termino il thread client
            Free(buffer);
            return NULL;
        }

        // Apro il file
        Fopen(*buffer, "r", &file);

        // Calcolo n, avg
        Frewind(file);

        while (fscanf(file, "%f", &number) == 1)
        {
            n++;
            avg += number;
        }

        if (n)
            avg /= n;

        // Calcolo std
        Frewind(file);

        if (n)
        {
            while (fscanf(file, "%f", &number) == 1)
                std += pow(number - avg, 2);

            std = sqrt(std / (float)n);
        }

        // Chiudo il file
        Fclose(file);

        // Creo il messaggio da mandare al server
        snprintf(msg, BUF_SIZE, "%-10ld%-10.2f%-10.2f%s", n, avg, std, *buffer);

        // Mando il messaggio al server
        Send(client_fd, msg, BUF_SIZE);

        // Resetto i valori
        n = 0;
        avg = 0;
        std = 0;
    }

    return NULL;
}

int collector()
{
    int server_fd;              // File descriptor del server
    Address server_address;     // Indirizzo del server

    // Creo il server
    Socket(&server_fd);

    // Binding
    ReuseAddr(server_fd);
    SetAddr(&server_address, ADDRESS, PORT);
    Bind(server_fd, &server_address);

    // Server in ascolto
    Listen(server_fd, MAX_CONNECTION);

    // Stampo l'intestazione della tabella
    printf("%-10s%-10s%-10s%-10s\n", "n", "avg", "std", "file");
    printf("----------------------------------------\n");

    // Inizializzo le variabili
    init_collector_workspace();

    // Accetto le richieste dei client e le gestisco nei thread handler
    for (size_t i = 0; i < W; i++)
    {
        Accept(&client_fds[i], server_fd);

        TCreate(&handlers[i], handler, (void *)(client_fds + i));
    }

    // Aspetto che tutti i client abbiano interrotto la connessione
    for (size_t i = 0; i < W; i++)
        Join(handlers[i], NULL);

    // Libero la memoria
    for (size_t i = 0; i < all_paths_len; i++)
        Free(all_paths[i]);
    Free(all_paths);

    Free(workers);
    Free(client_fds);
    Free(handlers);
    Close(server_fd);

    return 0;
}

void *handler(void *args)
{
    // L'indirizzo del file descriptor del client viene passato come argomento al thread
    int client_fd = *(int *)args;

    // Buffer del messaggio ricevuto dal client
    char buffer[BUF_SIZE];      

    forever
    {
        Receive(client_fd, buffer, BUF_SIZE);

        // Gestisco terminazione 
        if (strncmp(buffer, "close", BUF_SIZE) == 0)
        {
            Close(client_fd);
            return NULL;
        }

        // Stampo la riga della tabella e svuoto il buffer
        printf("%s\n", buffer);
        memset(buffer, 0, BUF_SIZE);
    }

    return NULL;
}

