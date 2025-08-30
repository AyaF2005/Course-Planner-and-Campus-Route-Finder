#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define MAX_BUILDINGS 100
#define MAX_NAME_LEN 50

struct MinHeapNode {
    int v;
    int dist;
};

struct MinHeap {
    int size;
    int capacity;
    int *pos;
    struct MinHeapNode *array;
};

int graph[MAX_BUILDINGS][MAX_BUILDINGS];
char buildings[MAX_BUILDINGS][MAX_NAME_LEN];
int num_buildings = 0;

struct MinHeapNode newMinHeapNode(int v, int dist) {
    struct MinHeapNode minHeapNode;
    minHeapNode.v = v;
    minHeapNode.dist = dist;
    return minHeapNode;
}

struct MinHeap createMinHeap(int capacity) {
    struct MinHeap minHeap;
    minHeap.pos = (int *)malloc(capacity * sizeof(int));
    minHeap.size = 0;
    minHeap.capacity = capacity;
    minHeap.array = (struct MinHeapNode *)malloc(capacity * sizeof(struct MinHeapNode));
    return minHeap;
}

void swapMinHeapNode(struct MinHeapNode* a, struct MinHeapNode* b) {
    struct MinHeapNode t = *a;
    *a = *b;
    *b = t;
}

void minHeapify(struct MinHeap* minHeap, int idx) {
    int smallest, left, right;
    smallest = idx;
    left = 2 * idx + 1;
    right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->array[left].dist < minHeap->array[smallest].dist)
        smallest = left;

    if (right < minHeap->size && minHeap->array[right].dist < minHeap->array[smallest].dist)
        smallest = right;

    if (smallest != idx) {
        struct MinHeapNode smallestNode = minHeap->array[smallest];
        struct MinHeapNode idxNode = minHeap->array[idx];

        minHeap->pos[smallestNode.v] = idx;
        minHeap->pos[idxNode.v] = smallest;

        swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

        minHeapify(minHeap, smallest);
    }
}

int isEmpty(struct MinHeap* minHeap) {
    return minHeap->size == 0;
}

struct MinHeapNode extractMin(struct MinHeap* minHeap) {
    if (isEmpty(minHeap))
        return (struct MinHeapNode) {0};

    struct MinHeapNode root = minHeap->array[0];

    struct MinHeapNode lastNode = minHeap->array[minHeap->size - 1];
    minHeap->array[0] = lastNode;

    minHeap->pos[root.v] = minHeap->size-1;
    minHeap->pos[lastNode.v] = 0;

    --minHeap->size;
    minHeapify(minHeap, 0);

    return root;
}

void decreaseKey(struct MinHeap* minHeap, int v, int dist) {
    int i = minHeap->pos[v];

    minHeap->array[i].dist = dist;

    while (i && minHeap->array[i].dist < minHeap->array[(i - 1) / 2].dist) {
        minHeap->pos[minHeap->array[i].v] = (i-1)/2;
        minHeap->pos[minHeap->array[(i-1)/2].v] = i;
        swapMinHeapNode(&minHeap->array[i],  &minHeap->array[(i - 1) / 2]);

        i = (i - 1) / 2;
    }
}

int isInMinHeap(struct MinHeap *minHeap, int v) {
    if (minHeap->pos[v] < minHeap->size)
        return 1;
    return 0;
}

void insert(struct MinHeap* minHeap, int v, int dist) {
    if (isInMinHeap(minHeap, v)) {
        decreaseKey(minHeap, v, dist);
    } else {
        minHeap->pos[v] = minHeap->size;
        minHeap->array[minHeap->size] = newMinHeapNode(v, dist);
        minHeap->size++;
    }
}

int get_building_index(char* name) {
    for (int i = 0; i < num_buildings; i++) {
        if (strcmp(buildings[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_edge(char* src, char* dest, int dist) {
    int src_index = get_building_index(src);
    int dest_index = get_building_index(dest);
    if (src_index == -1 || dest_index == -1) {
        printf("Error: Invalid edge between %s and %s\n", src, dest);
        return;
    }
    graph[src_index][dest_index] = dist;  // Only add edge from source to destination for directed graph
}

void read_graph_from_file() {
    char* filename = "Buildings.txt";
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char src[MAX_NAME_LEN], dest[MAX_NAME_LEN];
        int dist;
        sscanf(line, "%[^#]#%[^#]#%d", src, dest, &dist);

        // Add the source and destination buildings to the buildings array
        if (get_building_index(src) == -1) {
            strcpy(buildings[num_buildings++], src);
        }
        if (get_building_index(dest) == -1) {
            strcpy(buildings[num_buildings++], dest);
        }

        add_edge(src, dest, dist);
    }
    fclose(file);
}

void dijkstra(int src_index, int* dist, int* prev) {
    struct MinHeap minHeap = createMinHeap(num_buildings);

    for (int v = 0; v < num_buildings; ++v) {
        dist[v] = INT_MAX;;
        prev[v] = -1;
        minHeap.array[v] = newMinHeapNode(v, dist[v]);
        minHeap.pos[v] = v;
    }

    minHeap.array[src_index] = newMinHeapNode(src_index, dist[src_index]);
    minHeap.pos[src_index] = src_index;
    dist[src_index] = 0;
    decreaseKey(&minHeap, src_index, dist[src_index]);

    minHeap.size = num_buildings;

    while (!isEmpty(&minHeap)) {
        struct MinHeapNode minHeapNode = extractMin(&minHeap);
        int u = minHeapNode.v;

        for (int v = 0; v < num_buildings; v++) {
            if (graph[u][v] && isInMinHeap(&minHeap, v) && dist[u] != INT_MAX && dist[u] + graph[u][v] < dist[v]) {
                dist[v] = dist[u] + graph[u][v];
                prev[v] = u;
                decreaseKey(&minHeap, v, dist[v]);
            }
        }
    }
}

void print_path(int* prev, int src_index, int dest_index) {
    if (dest_index == src_index) {
        printf("%s ", buildings[src_index]);
    } else if (prev[dest_index] == -1) {
        printf("No path from %s to %s\n", buildings[src_index], buildings[dest_index]);
    } else {
        print_path(prev, src_index, prev[dest_index]);
        printf("-> %s ", buildings[dest_index]);
    }
}

void find_shortest_path(char* src, char* dest, int* dist, int* prev) {
    int src_index = get_building_index(src);
    int dest_index = get_building_index(dest);
    if (src_index == -1 || dest_index == -1) {
        printf("Error: Invalid source or destination\n");
        return;
    }
    dijkstra(src_index, dist, prev);
}

///////////////////////////////// PART TWO ///////////////////////////////////////////
#include <stdio.h>
#include <string.h>

#define MAX_COURSES 100
#define MAX_NAME_LEN 50

int graph_courses[MAX_COURSES][MAX_COURSES];
char courses[MAX_COURSES][MAX_NAME_LEN];
int num_courses = 0;


int get_course_index(char* name) {
    for (int i = 0; i < num_courses; i++) {
        if (strcmp(courses[i], name) == 0) {
            return i;
        }
    }
    return -1;
}

void add_course_if_new(char* name) {
    if (get_course_index(name) == -1) {
        strcpy(courses[num_courses++], name);
    }
}

void add_edge_course(int src_index, int dest_index) {
    if (src_index == -1 || dest_index == -1) {
        printf("Error: Invalid edge between %s and %s\n", courses[src_index], courses[dest_index]);
        return;
    }
    graph_courses[dest_index][src_index] = 1; // Edge points from prerequisite to course
}

void read_courses_from_file(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error: Could not open file %s\n", filename);
        return;
    }

    memset(graph_courses, 0, sizeof(graph_courses));
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char src[MAX_NAME_LEN], dest1[MAX_NAME_LEN], dest2[MAX_NAME_LEN];
        int count = sscanf(line, "%[^#]#%[^#\n]", src, dest1);
        add_course_if_new(src);
        add_course_if_new(dest1);

        char* token = strtok(dest1, " and ");
        while (token != NULL) {
            add_edge_course(get_course_index(token), get_course_index(src));
            token = strtok(NULL, " and ");
        }
    }
    fclose(file);
}

typedef struct {
    int course_index;
    int in_degree;
} Node;

int compare(const void* a, const void* b) {
    return ((Node*)a)->in_degree - ((Node*)b)->in_degree;
}

void topological_sort() {
    int in_degree[MAX_COURSES] = {0};
    Node queue[MAX_COURSES];
    int front = 0, rear = -1;
    int top_order[MAX_COURSES], counter = 0;

    // Calculate in-degree for each vertex
    for (int i = 0; i < num_courses; i++) {
        for (int j = 0; j < num_courses; j++) {
            in_degree[i] += graph_courses[j][i];
        }
    }

    // Print initial in-degree and adjacency list
    printf("Step 1: Initialize In-degrees and Adjacency List\n");
    printf("In-degree:\n");
    for (int i = 0; i < num_courses; i++) {
        printf("%s: %d\n", courses[i], in_degree[i]);
    }
    printf("Adjacency List:\n");
    for (int i = 0; i < num_courses; i++) {
        printf("%s: ", courses[i]);
        for (int j = 0; j < num_courses; j++) {
            if (graph_courses[i][j]) {
                printf("%s, ", courses[j]);
            }
        }
        printf("\n");
    }

    // Enqueue courses with in-degree 0
    printf("\nStep 2: Enqueue Courses with In-degree 0\n");
    for (int i = 0; i < num_courses; i++) {
        if (in_degree[i] == 0) {
            printf("Queue: [%s] ", courses[i]);
            queue[++rear].course_index = i;
            queue[rear].in_degree = in_degree[i];
        }
    }

    // Process the queue
    printf("\nStep 3: Process the Queue\n");
    while (front <= rear) {
        int u = queue[front++].course_index;
        top_order[counter++] = u;

        // Decrease in-degree by 1 for all its neighboring nodes
        printf("\nDequeue %s:\n", courses[u]);
        printf("Topological Order: ");
        for (int i = 0; i < counter; i++) {
            printf("%s, ", courses[top_order[i]]);
        }
        printf("\nUpdate in-degree of its dependents:\n");
        for (int v = 0; v < num_courses; v++) {
            if (graph_courses[u][v]) {
                if (--in_degree[v] == 0) {
                    printf("%s: 0 (enqueue)\n", courses[v]);
                    queue[++rear].course_index = v;
                    queue[rear].in_degree = in_degree[v];
                }
            }
        }
    }

    // Print final topological order
    printf("\nFinal Topological Order\n");
    printf("The final topological order is:\n");
    for (int i = 0; i < num_courses; i++) {
        printf("%s, ", courses[top_order[i]]);
    }
    printf("\n");
}

void print_sorted_courses() {
    for (int i = 0; i < num_courses; i++) {
        printf("%s\n", courses[i]);
    }
}

////////////////////////// MAIN FUNCTION //////////////////////////////////////////////////////////////////////


int main() {
    int choice;
    char src[MAX_NAME_LEN], dest[MAX_NAME_LEN];
    int dist[MAX_BUILDINGS], prev[MAX_BUILDINGS];

    while (1) {
        printf("\nMenu:\n");
        printf("1. Load the two files.\n");
        printf("2. Calculate the shortest distance between two buildings.\n");
        printf("3. Print the shortest route between two buildings and the total distance.\n");
        printf("4. Sort the courses using the topological sort.\n");
        printf("5. Print the sorted courses.\n");
        printf("6. Exit.\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                read_graph_from_file();
                read_courses_from_file("Courses.txt");
                printf("Files loaded successfully.\n");

                break;
            case 2:
                printf("Enter source building: ");
                scanf("%s", src);
                printf("Enter destination building: ");
                scanf("%s", dest);
                find_shortest_path(src, dest, dist, prev);
                break;
            case 3:
                printf("Shortest path from %s to %s: ", src, dest);
                print_path(prev, get_building_index(src), get_building_index(dest));
                printf("\nTotal distance: %d\n", dist[get_building_index(dest)]);
                break;
            case 4:
                topological_sort();
                printf("Courses sorted successfully.\n");

                break;
            case 5:
                print_sorted_courses();
                break;
            case 6:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice. Please enter a number between 1 and 6.\n");
        }
    }
    return 0;
}