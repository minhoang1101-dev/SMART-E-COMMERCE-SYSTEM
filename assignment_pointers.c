#include <stdio.h>
#include <stdlib.h>

#define NAME_SIZE 100

typedef struct {
    int id;
    char fullName[NAME_SIZE];
    int age;
    float salary;
} Worker;

void Swap(int *x, int *y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

void swapWorker(Worker *a, Worker *b) {
    Worker temp = *a;
    *a = *b;
    *b = temp;
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
}

void printIntArray(int *arr, int size) {
    int *p;

    for (p = arr; p < arr + size; p++) {
        printf("%d ", *p);
    }
    printf("\n");
}

int sum1DArray(int *arr, int size) {
    int sum = 0;
    int *p;

    for (p = arr; p < arr + size; p++) {
        sum += *p;
    }

    return sum;
}

void sortAscending(int *arr, int size) {
    int *i;
    int *j;

    for (i = arr; i < arr + size - 1; i++) {
        for (j = i + 1; j < arr + size; j++) {
            if (*i > *j) {
                Swap(i, j);
            }
        }
    }
}

int maxSubarraySum(int *arr, int size) {
    int best = *arr;
    int current = *arr;
    int *p;

    for (p = arr + 1; p < arr + size; p++) {
        if (current + *p < *p) {
            current = *p;
        } else {
            current += *p;
        }

        if (best < current) {
            best = current;
        }
    }

    return best;
}

int sum2DArray(int *matrix, int rows, int cols) {
    int sum = 0;
    int *p;

    for (p = matrix; p < matrix + rows * cols; p++) {
        sum += *p;
    }

    return sum;
}

float sumSalary(Worker *workers, int size) {
    float sum = 0;
    Worker *p;

    for (p = workers; p < workers + size; p++) {
        sum += p->salary;
    }

    return sum;
}

void sortWorkersBySalaryDescending(Worker *workers, int size) {
    Worker *i;
    Worker *j;

    for (i = workers; i < workers + size - 1; i++) {
        for (j = i + 1; j < workers + size; j++) {
            if (i->salary < j->salary) {
                swapWorker(i, j);
            }
        }
    }
}

void assignment1(void) {
    int a;
    int b;

    printf("\n========== Assignment 1: Swap two integers ==========\n");
    printf("Enter first integer: ");
    scanf("%d", &a);
    printf("Enter second integer: ");
    scanf("%d", &b);

    printf("Before swap: a = %d, b = %d\n", a, b);
    Swap(&a, &b);
    printf("After swap:  a = %d, b = %d\n", a, b);
}

void assignment2(void) {
    int size;
    int *arr;
    int i;

    printf("\n========== Assignment 2: 1D array ==========\n");
    printf("Enter size of array: ");
    scanf("%d", &size);

    if (size <= 0) {
        printf("Array size must be positive.\n");
        return;
    }

    arr = (int *)malloc(size * sizeof(int));
    if (arr == NULL) {
        printf("Cannot allocate memory.\n");
        return;
    }

    for (i = 0; i < size; i++) {
        printf("A[%d] = ", i);
        scanf("%d", arr + i);
    }

    printf("Original array: ");
    printIntArray(arr, size);

    printf("Sum of this array: %d\n", sum1DArray(arr, size));
    printf("Max value of sum of a subarray: %d\n", maxSubarraySum(arr, size));

    sortAscending(arr, size);
    printf("Array after sorting ascending: ");
    printIntArray(arr, size);

    free(arr);
}

void assignment3(void) {
    int rows;
    int cols;
    int *matrix;
    int i;
    int j;

    printf("\n========== Assignment 3: 2D array / matrix ==========\n");
    printf("Enter number of rows: ");
    scanf("%d", &rows);
    printf("Enter number of columns: ");
    scanf("%d", &cols);

    if (rows <= 0 || cols <= 0) {
        printf("Rows and columns must be positive.\n");
        return;
    }

    matrix = (int *)malloc(rows * cols * sizeof(int));
    if (matrix == NULL) {
        printf("Cannot allocate memory.\n");
        return;
    }

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            printf("A[%d][%d] = ", i, j);
            scanf("%d", matrix + i * cols + j);
        }
    }

    printf("Sum of this 2D array: %d\n", sum2DArray(matrix, rows, cols));

    printf("Transpose matrix:\n");
    for (j = 0; j < cols; j++) {
        for (i = 0; i < rows; i++) {
            printf("%d ", *(matrix + i * cols + j));
        }
        printf("\n");
    }

    free(matrix);
}

void assignment4(void) {
    int size;
    Worker *workers;
    int i;

    printf("\n========== Assignment 4: Workers ==========\n");
    printf("Enter number of workers: ");
    scanf("%d", &size);
    clearInputBuffer();

    if (size <= 0) {
        printf("Number of workers must be positive.\n");
        return;
    }

    workers = (Worker *)malloc(size * sizeof(Worker));
    if (workers == NULL) {
        printf("Cannot allocate memory.\n");
        return;
    }

    for (i = 0; i < size; i++) {
        Worker *p = workers + i;

        printf("\nWorker %d\n", i + 1);
        printf("ID: ");
        scanf("%d", &p->id);
        clearInputBuffer();

        printf("Full name: ");
        fgets(p->fullName, NAME_SIZE, stdin);
        for (char *ch = p->fullName; *ch != '\0'; ch++) {
            if (*ch == '\n') {
                *ch = '\0';
                break;
            }
        }

        printf("Age: ");
        scanf("%d", &p->age);

        printf("Salary: ");
        scanf("%f", &p->salary);
        clearInputBuffer();
    }

    printf("\nSum of salary paid for all workers: %.2f\n", sumSalary(workers, size));

    sortWorkersBySalaryDescending(workers, size);

    printf("\nWorkers sorted by salary descending:\n");
    printf("%-10s %-30s %-10s %-10s\n", "ID", "Full name", "Age", "Salary");
    for (i = 0; i < size; i++) {
        Worker *p = workers + i;
        printf("%-10d %-30s %-10d %-10.2f\n", p->id, p->fullName, p->age, p->salary);
    }

    free(workers);
}

int main(void) {
    assignment1();
    assignment2();
    assignment3();
    assignment4();

    printf("\nAll assignments finished. Code vibes: clean pointers, no panic.\n");
    return 0;
}
