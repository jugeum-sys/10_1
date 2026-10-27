#define _CRT_SECURE_NO_WARNINGS   
#include <stdio.h>

#define N            10000     
#define TRIALS         100     
#define MAX_KEY    1000000     
#define STABLE_N      1000     
#define STABLE_KEYS     20     
#define SWAP_RATIO       5     
#define DEFAULT_SEED 12345


static unsigned long rngState = DEFAULT_SEED;
static void rng_seed(unsigned long seed) { rngState = seed; }
static int rng_next(int bound) {
    rngState = rngState * 1103515245UL + 12345UL;
    return (int)((rngState >> 16) % (unsigned long)bound);
}


typedef struct {
    int key1;     
    int key2;     
    int index;    
} Data;

typedef struct {
    long long cmp;   
    long long mov;   
} Counter;

static Data original[N], work[N];
static Data sortedData[N], reverseData[N], partialData[N];
static Data tmpArr[N];

static void copy_array(const Data src[], Data dst[], int n) {
    int i;
    for (i = 0; i < n; i++) dst[i] = src[i];
}

static void make_random(Data a[], int n) {
    int i;
    for (i = 0; i < n; i++) {
        a[i].key1 = rng_next(MAX_KEY + 1);
        a[i].key2 = rng_next(MAX_KEY + 1);
        a[i].index = i;                      
    }
}


static void selection_sort(Data a[], int n, Counter* c) {
    long long cmp = 0, mov = 0;
    int i, j, min;
    Data temp;

    for (i = 0; i < n - 1; i++) {
        min = i;
        for (j = i + 1; j < n; j++) {
            cmp++;                               
            if (a[j].key1 < a[min].key1) min = j;
        }
        if (min != i) {
            temp = a[i];  a[i] = a[min];  a[min] = temp;
            mov += 3;                            
        }
    }
    c->cmp = cmp; c->mov = mov;
}


static void bubble_sort(Data a[], int n, Counter* c) {
    long long cmp = 0, mov = 0;
    int i, j;
    Data temp;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                temp = a[j];  a[j] = a[j + 1];  a[j + 1] = temp;
                mov += 3;
            }
        }
    }
    c->cmp = cmp; c->mov = mov;
}


static void bubble_sort_early(Data a[], int n, Counter* c, int* passes) {
    long long cmp = 0, mov = 0;
    int i, j, swapped, p = 0;
    Data temp;

    for (i = 0; i < n - 1; i++) {
        swapped = 0;
        p++;
        for (j = 0; j < n - 1 - i; j++) {
            cmp++;
            if (a[j].key1 > a[j + 1].key1) {
                temp = a[j];  a[j] = a[j + 1];  a[j + 1] = temp;
                mov += 3;
                swapped = 1;
            }
        }
        if (swapped == 0) break;                 
    }
    c->cmp = cmp; c->mov = mov;
    if (passes != NULL) *passes = p;
}


static void insertion_sort(Data a[], int n, Counter* c, int useGE) {
    long long cmp = 0, mov = 0;
    int i, j;
    Data cur;

    for (i = 1; i < n; i++) {
        cur = a[i];  mov++;                      
        j = i - 1;
        while (j >= 0) {
            cmp++;                               
            if (useGE ? (a[j].key1 >= cur.key1) : (a[j].key1 > cur.key1)) {
                a[j + 1] = a[j];  mov++;         
                j--;
            }
            else {
                break;
            }
        }
        a[j + 1] = cur;  mov++;                  
    }
    c->cmp = cmp; c->mov = mov;
}


static int is_sorted(const Data a[], int n) {
    int i;
    for (i = 1; i < n; i++) if (a[i - 1].key1 > a[i].key1) return 0;
    return 1;
}


static int is_stable(const Data a[], int n) {
    int i;
    for (i = 1; i < n; i++)
        if (a[i - 1].key1 == a[i].key1 && a[i - 1].index > a[i].index) return 0;
    return 1;
}

static void merge_sort(Data a[], Data tmp[], int left, int right) {
    int mid, i, j, k;
    if (left >= right) return;
    mid = (left + right) / 2;
    merge_sort(a, tmp, left, mid);
    merge_sort(a, tmp, mid + 1, right);
    i = left; j = mid + 1; k = left;
    while (i <= mid && j <= right)
        tmp[k++] = (a[i].key1 <= a[j].key1) ? a[i++] : a[j++];
    while (i <= mid)   tmp[k++] = a[i++];
    while (j <= right) tmp[k++] = a[j++];
    for (i = left; i <= right; i++) a[i] = tmp[i];
}


static void run_input_case(const char* title, const Data base[], int n) {
    Counter c;
    int passes;

    printf("\n%s\n", title);

    copy_array(base, work, n);
    selection_sort(work, n, &c);
    printf("\nSelection Sort\n  Comparisons : %lld\n  Movements   : %lld\n  정렬 확인   : %s\n",
        c.cmp, c.mov, is_sorted(work, n) ? "오름차순 정렬됨" : "실패");

    copy_array(base, work, n);
    bubble_sort(work, n, &c);
    printf("\nBubble Sort\n  Comparisons : %lld\n  Movements   : %lld\n  정렬 확인   : %s\n",
        c.cmp, c.mov, is_sorted(work, n) ? "오름차순 정렬됨" : "실패");

    copy_array(base, work, n);
    bubble_sort_early(work, n, &c, &passes);
    printf("\nBubble Sort (Early Termination)\n  Comparisons : %lld\n  Movements   : %lld\n"
        "  수행한 순회 : %d회\n  정렬 확인   : %s\n",
        c.cmp, c.mov, passes, is_sorted(work, n) ? "오름차순 정렬됨" : "실패");

    copy_array(base, work, n);
    insertion_sort(work, n, &c, 0);
    printf("\nInsertion Sort\n  Comparisons : %lld\n  Movements   : %lld\n  정렬 확인   : %s\n",
        c.cmp, c.mov, is_sorted(work, n) ? "오름차순 정렬됨" : "실패");

    printf("\n------------------------------------\n");
}


static unsigned long ask_seed(void) {
    char line[64];
    unsigned long value = 0;
    int i = 0, digits = 0;

    printf("난수 시드를 입력하세요 (그냥 Enter를 누르면 %d): ", DEFAULT_SEED);
    if (fgets(line, sizeof(line), stdin) == NULL) return DEFAULT_SEED;
    while (line[i] == ' ' || line[i] == '\t') i++;
    while (line[i] >= '0' && line[i] <= '9') {
        value = value * 10 + (unsigned long)(line[i] - '0');
        digits++; i++;
    }
    return (digits > 0) ? value : DEFAULT_SEED;
}


int main(void) {
    Counter c;
    long long selCmp = 0, selMov = 0, bubCmp = 0, bubMov = 0;
    long long earCmp = 0, earMov = 0, insCmp = 0, insMov = 0;
    int t, i, j, tmpIdx, allSorted = 1;
    Data stableData[STABLE_N], stableWork[STABLE_N], temp;

    rng_seed(ask_seed());

    printf("\nData Count : %d\n", N);

    printf("\nRandom Data\nNumber of Trials : %d\n", TRIALS);
    printf("(10,000개 데이터를 네 가지 방법으로 100회 정렬합니다. 잠시 걸립니다...)\n");

    for (t = 0; t < TRIALS; t++) {
        make_random(original, N);                

        copy_array(original, work, N);
        selection_sort(work, N, &c);
        selCmp += c.cmp; selMov += c.mov;
        if (!is_sorted(work, N)) allSorted = 0;

        copy_array(original, work, N);
        bubble_sort(work, N, &c);
        bubCmp += c.cmp; bubMov += c.mov;
        if (!is_sorted(work, N)) allSorted = 0;

        copy_array(original, work, N);
        bubble_sort_early(work, N, &c, NULL);
        earCmp += c.cmp; earMov += c.mov;
        if (!is_sorted(work, N)) allSorted = 0;

        copy_array(original, work, N);
        insertion_sort(work, N, &c, 0);
        insCmp += c.cmp; insMov += c.mov;
        if (!is_sorted(work, N)) allSorted = 0;
    }

    printf("\nSelection Sort\n  Total Comparisons   : %lld\n  Average Comparisons : %.1f\n"
        "  Average Movements   : %.1f\n", selCmp, (double)selCmp / TRIALS, (double)selMov / TRIALS);
    printf("\nBubble Sort\n  Total Comparisons   : %lld\n  Average Comparisons : %.1f\n"
        "  Average Movements   : %.1f\n", bubCmp, (double)bubCmp / TRIALS, (double)bubMov / TRIALS);
    printf("\nBubble Sort (Early Termination)\n  Total Comparisons   : %lld\n  Average Comparisons : %.1f\n"
        "  Average Movements   : %.1f\n", earCmp, (double)earCmp / TRIALS, (double)earMov / TRIALS);
    printf("\nInsertion Sort\n  Total Comparisons   : %lld\n  Average Comparisons : %.1f\n"
        "  Average Movements   : %.1f\n", insCmp, (double)insCmp / TRIALS, (double)insMov / TRIALS);
    printf("\n정렬 결과 확인 : %s\n", allSorted ? "100회 모두 오름차순 정렬 성공" : "실패한 경우 있음");
    printf("\n------------------------------------\n");


    make_random(original, N);

    copy_array(original, sortedData, N);
    merge_sort(sortedData, tmpArr, 0, N - 1);            

    for (i = 0; i < N; i++) reverseData[i] = sortedData[N - 1 - i];   

    copy_array(sortedData, partialData, N);
    for (i = 0; i < N * SWAP_RATIO / 100; i++) {
        j = rng_next(N);
        tmpIdx = rng_next(N);
        temp = partialData[j]; partialData[j] = partialData[tmpIdx]; partialData[tmpIdx] = temp;
    }

    run_input_case("Sorted Data", sortedData, N);
    run_input_case("Reverse-Sorted Data", reverseData, N);
    run_input_case("Partially Sorted Data (정렬 후 무작위 교환 500회)", partialData, N);


    for (i = 0; i < STABLE_N; i++) {
        stableData[i].key1 = rng_next(STABLE_KEYS);
        stableData[i].key2 = i;
        stableData[i].index = i;
    }

    printf("\nStability Verification\n");
    printf("(데이터 %d개, 키1은 0~%d만 사용해 중복을 많이 만든 뒤\n"
        " 키1이 같은 데이터끼리 원래 순서가 유지되는지 확인)\n\n",
        STABLE_N, STABLE_KEYS - 1);

    copy_array(stableData, stableWork, STABLE_N);
    selection_sort(stableWork, STABLE_N, &c);
    printf("  Selection Sort       : %s\n", is_stable(stableWork, STABLE_N) ? "Stable" : "Unstable");

    copy_array(stableData, stableWork, STABLE_N);
    bubble_sort(stableWork, STABLE_N, &c);
    printf("  Bubble Sort          : %s\n", is_stable(stableWork, STABLE_N) ? "Stable" : "Unstable");

    copy_array(stableData, stableWork, STABLE_N);
    insertion_sort(stableWork, STABLE_N, &c, 0);
    printf("  Insertion Sort (>)   : %s\n", is_stable(stableWork, STABLE_N) ? "Stable" : "Unstable");

    copy_array(stableData, stableWork, STABLE_N);
    insertion_sort(stableWork, STABLE_N, &c, 1);
    printf("  Insertion Sort (>=)  : %s\n", is_stable(stableWork, STABLE_N) ? "Stable" : "Unstable");

    {
        Data demo[4] = { {527,100,0}, {527,200,1}, {125,300,2}, {892,400,3} };
        Data d[4];
        printf("\n  [작은 예시로 확인] 원본 : ");
        for (i = 0; i < 4; i++) printf("(%d,%d) ", demo[i].key1, demo[i].key2);

        copy_array(demo, d, 4);
        insertion_sort(d, 4, &c, 0);
        printf("\n  삽입 정렬 (>)  : ");
        for (i = 0; i < 4; i++) printf("(%d,%d) ", d[i].key1, d[i].key2);

        copy_array(demo, d, 4);
        insertion_sort(d, 4, &c, 1);
        printf("\n  삽입 정렬 (>=) : ");
        for (i = 0; i < 4; i++) printf("(%d,%d) ", d[i].key1, d[i].key2);
        printf("\n");
    }

    return 0;
}