#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 100
#define STUDENT_ID_SIZE 10
#define PARSE_FIELD 4

typedef enum {
    GRADE_EXCELLENT,
    GRADE_GOOD,
    GRADE_AVERAGE,
    GRADE_POOR,
} e_grade_t;

typedef struct {
    char    id[7];
    char    name[50];
    char    class[8];
    float   gpa;
} Student;

char g_buf[BUF_SIZE] = {0};

char *get_grade(float gpa);
Student parse_student(char *strdata);

char search_result_buf [] =
"========== SEARCH RESULT ==========    \n\
  ID      : %s                          \n\
  Name    : %s                          \n\
  Class   : %s                          \n\
  GPA     : %.2f                        \n\
  Grade   : %s                          \n\
====================================    \n\
";

void search_result(Student student)
{
    printf(search_result_buf, student.id, student.name, student.class, student.gpa, get_grade(student.gpa));
}

int main(int argc, char *argv[])
{
    int status = 0;
    FILE *f;
    printf("[SEARCHER] PID: %d | PPID: %d\n", getpid(), getppid());
    printf("[SEARCHER] Searching for \"%s\" in %s...\n", argv[1], argv[2]);
    char student_id[STUDENT_ID_SIZE]= {0};
    if(argc == 3) {
        strncpy(student_id, argv[1], sizeof(student_id));
        f = fopen(argv[2], "r");
        if (NULL == f) {
            perror("Error: Open file failed");
            exit(2);
        }
        while(1) {
            char *readline = fgets(g_buf, sizeof(g_buf), f);
            if(readline != NULL) {
                if(strstr(readline, student_id)) {
                    search_result(parse_student(readline));
                    status = 0;
                    goto CLOSE_FILE;
                }
            }
            else {
                printf("No student found with ID: %s\n", student_id);
                status = 1;
                goto CLOSE_FILE;
            }
        }
    }
    else {
        perror("Error: Argument error\n");
        exit(2);

    }
CLOSE_FILE:
    if(fclose(f) == -1) {
        perror("Error: Cannot close file\n");
        exit(2);
    }
    else {
        printf("Close file successfully, exit code: %d\n", status);
    }
    exit(status);
}

void parse_string(void *param_out, size_t param_out_size, void *pvParameters, size_t pv_param_size)
{
    (void)pv_param_size;
    strncpy((char*)param_out,(char*)pvParameters, param_out_size);
}

void parse_float(void *param_out, size_t param_out_size, void *pvParameters, size_t pv_param_size)
{
    (void)param_out_size;
    char *endptr;
    float fvalue = strtof((const char*)pvParameters, &endptr);
    if((pvParameters + pv_param_size - 1) == endptr) {
        memcpy(param_out, (const void*)&fvalue, sizeof(float));
    }
}

Student parse_student(char *strdata)
{
    typedef struct {
        int token_idx;
        void (*pfunc_parse)(void *, size_t, void *, size_t);
        void *arg1;
        size_t sizearg1;
    } st_token_pair_t;
    Student student = {0};
    st_token_pair_t student_token_pair[] = {
        {0, parse_string, (void*)&student.id,       sizeof(student.id),   },
        {1, parse_string, (void*)&student.name,     sizeof(student.name), },
        {2, parse_string, (void*)&student.class,    sizeof(student.class),},
        {3, parse_float , (void*)&student.gpa,      0,                    },
    };
    char delim[] = "|";
    int i = 0;
    size_t token_length;
    char *token = strtok(strdata, delim);
    student_token_pair[0].pfunc_parse(
            student_token_pair[0].arg1,
            student_token_pair[0].sizearg1,
            token,
            0);
    i++;

    while (token != NULL) {
        token = strtok(NULL, delim);
        /* Only get the length of token for GPA token */
        if(student_token_pair[i].token_idx == 3) {
            token_length = strlen(token);
        }
        else { /* The rest of token is unused */
            token_length = 0;
        }
        student_token_pair[i].pfunc_parse(
                student_token_pair[i].arg1,
                student_token_pair[i].sizearg1,
                token,
                token_length);
        i++;
        if(i >= PARSE_FIELD)
            break;
    }
    return student;
}

char *get_grade(float gpa)
{
    e_grade_t  egrade = GRADE_EXCELLENT;
    if(gpa >= 8.5 && gpa < 10.0) {
        egrade = GRADE_EXCELLENT;
    }
    else if(gpa >= 7.0) {
        egrade = GRADE_GOOD;
    }
    else if(gpa >= 5.0) {
        egrade = GRADE_AVERAGE;
    }
    else if(gpa >= 0.0 && gpa < 5.0) {
        egrade = GRADE_POOR;
    }
    else {
        egrade = 1000;
    }
    switch(egrade) {
        case GRADE_EXCELLENT: return "Excellent";
        case GRADE_GOOD: return "Good";
        case GRADE_AVERAGE: return "Average";
        case GRADE_POOR: return "Poor";
        default: return "GPA Error";
    }
    return "";

}
