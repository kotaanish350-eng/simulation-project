#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NAME 64
#define QUEUE_CAPACITY 5
#define TOTAL_TO_SERVE 100

typedef struct {
    char name[MAX_NAME];
    double cash;
    int arrival_time;
    int service_time;
} Customer;

typedef struct {
    Customer data[QUEUE_CAPACITY];
    int head;
    int tail;
    int count;
} Queue;

void q_init(Queue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int q_is_empty(const Queue *q) {
    return q->count == 0;
}

int q_is_full(const Queue *q) {
    return q->count == QUEUE_CAPACITY;
}

int q_size(const Queue *q) {
    return q->count;
}

int q_enqueue(Queue *q, const Customer *c) {
    if (q_is_full(q)) return 0;
    q->data[q->tail] = *c;
    q->tail = (q->tail + 1) % QUEUE_CAPACITY;
    q->count++;
    return 1;
}

int q_dequeue(Queue *q, Customer *out) {
    if (q_is_empty(q)) return 0;
    if (out) *out = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_CAPACITY;
    q->count--;
    return 1;
}

Customer* q_front(Queue *q) {
    if (q_is_empty(q)) return NULL;
    return &q->data[q->head];
}

static void read_line(const char *prompt, char *buf, size_t n) {
    if (prompt) { printf("%s", prompt); fflush(stdout); }
    if (!fgets(buf, (int)n, stdin)) { buf[0] = '\0'; return; }
    size_t len = strlen(buf);
    if (len && buf[len-1] == '\n') buf[len-1] = '\0';
}

static int read_int(const char *prompt) {
    char buf[64];
    read_line(prompt, buf, sizeof(buf));
    return atoi(buf);
}

static double read_double(const char *prompt) {
    char buf[64];
    read_line(prompt, buf, sizeof(buf));
    return atof(buf);
}

static int rand_range(int a, int b) {
    if (b < a) return a;
    return a + (rand() % (b - a + 1));
}

static void gen_random_name(char *buf, size_t n) {
    const char *names[] = {
        "Alex","Ben","Cara","Dina","Eve","Fred","Gina","Hank","Isha","Jack",
        "Kyle","Lina","Mona","Ned","Omar","Pam","Quin","Rita","Sam","Tina"
    };
    size_t m = sizeof(names)/sizeof(names[0]);
    snprintf(buf, n, "%s%d", names[rand()%m], rand()%1000);
}

int main(void) {
    srand((unsigned)time(NULL));

    printf("Single-Cashier Checkout Simulator\n");
    printf("Queue capacity: %d, simulate serving %d customers then report average waiting time.\n\n", QUEUE_CAPACITY, TOTAL_TO_SERVE);

    printf("Choose input mode:\n1) Manual\n2) Automatic\n");
    int mode = read_int("Enter 1 or 2: ");
    if (mode != 1 && mode != 2) mode = 1;

    const int ARRIVAL_MIN = 0, ARRIVAL_MAX = 3;
    const int SERVICE_MIN = 1, SERVICE_MAX = 5;

    Queue q;
    q_init(&q);

    int served_count = 0;
    int customers_generated = 0;
    double total_wait_time = 0.0;

    int current_time = 0;
    int next_arrival_time = 0;
    int cashier_free_time = 0;
    int cashier_busy = 0;

    while (served_count < TOTAL_TO_SERVE) {
        if (customers_generated == 0) {
            next_arrival_time = current_time + rand_range(ARRIVAL_MIN, ARRIVAL_MAX);
        }

        int next_event_time = next_arrival_time;
        if (cashier_busy && cashier_free_time < next_event_time) next_event_time = cashier_free_time;
        if (!cashier_busy && next_arrival_time <= current_time) next_event_time = next_arrival_time;
        current_time = next_event_time;

        if (cashier_busy && current_time >= cashier_free_time) {
            cashier_busy = 0;
        }

        if (current_time >= next_arrival_time) {
            Customer c;
            if (mode == 1) {
                char tmp[MAX_NAME];
                read_line("\n--- New arrival ---\nEnter name: ", tmp, sizeof(tmp));
                if (strlen(tmp) == 0)
                    snprintf(tmp, sizeof(tmp), "Guest%d", customers_generated+1);
                strncpy(c.name, tmp, MAX_NAME-1);
                c.name[MAX_NAME-1] = '\0';
                c.cash = read_double("Enter cash amount: ");
            } else {
                gen_random_name(c.name, sizeof(c.name));
                c.cash = (double)rand_range(1, 200);
            }
            c.arrival_time = current_time;
            c.service_time = rand_range(SERVICE_MIN, SERVICE_MAX);

            customers_generated++;

            if (q_is_full(&q)) {
                printf("At time %d: Arrival '%s' - QUEUE IS FULL.\n", current_time, c.name);
            } else {
                q_enqueue(&q, &c);
                printf("At time %d: Arrival '%s' (cash=%.2f, service=%ds) Queue size: %d\n",
                       current_time, c.name, c.cash, c.service_time, q_size(&q));
            }
            next_arrival_time = current_time + rand_range(ARRIVAL_MIN, ARRIVAL_MAX);
        }

        if (!cashier_busy && !q_is_empty(&q)) {
            Customer front;
            q_dequeue(&q, &front);
            int start_service_time = current_time;
            double wait = (double)(start_service_time - front.arrival_time);
            total_wait_time += wait;
            served_count++;

            int members_left = q_size(&q);
            if (members_left == 0)
                printf("At time %d: Served '%s' (wait=%.0f s). No persons in queue.\n", current_time, front.name, wait);
            else
                printf("At time %d: Served '%s' (wait=%.0f s). Members left: %d\n", current_time, front.name, wait, members_left);

            cashier_busy = 1;
            cashier_free_time = current_time + front.service_time;
        }

        if (!cashier_busy && q_is_empty(&q)) {
            if (next_arrival_time > current_time) {
                printf("At time %d: No persons in queue. Next arrival at t=%d\n", current_time, next_arrival_time);
            }
        }
    }

    double avg_wait = total_wait_time / served_count;
    printf("\nSimulation finished: Served %d customers.\n", served_count);
    printf("Total attempted arrivals: %d\n", customers_generated);
    printf("Average waiting time: %.3f seconds\n", avg_wait);

    return 0;
}
