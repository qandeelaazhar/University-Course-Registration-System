#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/time.h>

#define MAX_REQUESTS 10
#define MAX_COURSES 10

typedef struct {
    int id_num;   // Numeric index (0-based)
    char course_id[20];  // String ID e.g. "CS101"
    int capacity;   // Total seats configured
    int available_seats;  // Remaining seats (protected by mutex)
    int total_requests; // Total enrollment attempts
    pthread_mutex_t course_mutex; // Per-course mutual exclusion lock
} Course;

typedef struct {
    int student_id;   // Unique ID
    char name[50];  // Student name
    bool is_high_priority;   // Senior/graduating student flag
    int requested_courses[MAX_REQUESTS];  // Course indices requested
    bool request_status[MAX_REQUESTS]; // Outcome per request
    int num_requests; // Number of course requests
} Student;

typedef struct {
    char timestamp[20];  // HH:MM:SS of the attempt
    char status[15];  // "SUCCESS" or "FAILED"
    char student_name[50];
    char priority[10];  //"High" or "Low"
    char course_id[20];  // e.g. "CS101"
} JsonLogEntry;

Course *courses;
Student *students;
JsonLogEntry *json_logs;
int num_courses, num_students, json_log_count = 0;
int high_pri_success = 0, high_pri_failed = 0, low_pri_success = 0, low_pri_failed = 0;

pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t json_mutex = PTHREAD_MUTEX_INITIALIZER;

void get_timestamp(char* buffer) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm* tm_info = localtime(&tv.tv_sec);
    strftime(buffer, 20, "%H:%M:%S", tm_info);
}

void setup_from_web() {
    FILE *f = fopen("web_input.txt", "r");
    if (!f) exit(1);

    fscanf(f, "%d", &num_courses);
    fscanf(f, "%d", &num_students);

    courses = (Course*)malloc(num_courses * sizeof(Course));
    for(int i = 0; i < num_courses; i++) {
        courses[i].id_num = i;
        snprintf(courses[i].course_id, 20, "CS%d", 101 + i);
        fscanf(f, "%d", &courses[i].capacity);
        courses[i].available_seats = courses[i].capacity;
        courses[i].total_requests = 0;
        pthread_mutex_init(&courses[i].course_mutex, NULL);
    }
    students = (Student*)malloc(num_students * sizeof(Student));
    for(int i = 0; i < num_students; i++) {
        students[i].student_id = i + 1;
        // Cleanly read the name line
        fscanf(f, " %[^\n]", students[i].name);
        int pri, req_count;
        // Read the priority and number of requests
        if (fscanf(f, "%d %d", &pri, &req_count) != 2) break;
        students[i].is_high_priority = (pri == 1);
        students[i].num_requests = req_count;
        for(int r = 0; r < req_count; r++) {
            fscanf(f, "%d", &students[i].requested_courses[r]);
            students[i].request_status[r] = false;
        }
    }
    fclose(f);
    json_logs = (JsonLogEntry*)malloc(num_students * MAX_REQUESTS * sizeof(JsonLogEntry));
}

void* student_task(void* arg) {
    Student* student = (Student*)arg;
    for (int i = 0; i < student->num_requests; i++) {
        int idx = student->requested_courses[i];
        if (idx < 0 || idx >= num_courses) continue;

        pthread_mutex_lock(&courses[idx].course_mutex);
        courses[idx].total_requests++;
        if (courses[idx].available_seats > 0) {
            courses[idx].available_seats--;
            student->request_status[i] = true;
        }
        pthread_mutex_unlock(&courses[idx].course_mutex);

        pthread_mutex_lock(&json_mutex);
        get_timestamp(json_logs[json_log_count].timestamp);
        strcpy(json_logs[json_log_count].status, student->request_status[i] ? "SUCCESS" : "FAILED");
        strcpy(json_logs[json_log_count].student_name, student->name);
        strcpy(json_logs[json_log_count].priority, student->is_high_priority ? "High" : "Low");
        strcpy(json_logs[json_log_count].course_id, courses[idx].course_id);
        json_log_count++;
        pthread_mutex_unlock(&json_mutex);

        pthread_mutex_lock(&stats_mutex);
        if (student->is_high_priority) student->request_status[i] ? high_pri_success++ : high_pri_failed++;
        else student->request_status[i] ? low_pri_success++ : low_pri_failed++;
        pthread_mutex_unlock(&stats_mutex);
    }
    return NULL;
}

void export_json() {
    FILE *f = fopen("dashboard_data.json", "w");
    fprintf(f, "{\"summary\": {\"total_success\": %d, \"total_failed\": %d}, \"logs\": [",
            high_pri_success + low_pri_success, high_pri_failed + low_pri_failed);
    for (int i = 0; i < json_log_count; i++) {
        fprintf(f, "{\"timestamp\": \"%s\", \"name\": \"%s\", \"priority\": \"%s\", \"course_id\": \"%s\", \"status\": \"%s\"}%s",
            json_logs[i].timestamp, json_logs[i].student_name, json_logs[i].priority,
            json_logs[i].course_id, json_logs[i].status, (i == json_log_count - 1) ? "" : ",");
    }
    fprintf(f, "]}");
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc > 1 && strcmp(argv[1], "web") == 0) {
        setup_from_web();
        pthread_t *threads = malloc(num_students * sizeof(pthread_t));
        for (int i = 0; i < num_students; i++) pthread_create(&threads[i], NULL, student_task, &students[i]);
        for (int i = 0; i < num_students; i++) pthread_join(threads[i], NULL);
        export_json();
    }
    return 0;
}
