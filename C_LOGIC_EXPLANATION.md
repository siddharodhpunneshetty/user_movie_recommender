# C Logic Explanation - Movie Recommender System

## Complete Beginner-Friendly Guide for Viva Preparation

This document explains every structure, function, and logic block in the C-based movie recommendation engine in simple language.

---

# PART 1: movie.h (Header File)

The header file defines all the data structures and function declarations used in the system.

---

## Section 1: Constants

```c
#define MAX_TITLE_LEN 256
#define MAX_GENRE_LEN 64
#define MAX_DIRECTOR_LEN 128
#define HASH_TABLE_SIZE 211
#define MAX_MOVIES 500
#define MAX_RECOMMENDATIONS 20
```

**Explanation:**
These are fixed values used throughout the program:
- `MAX_TITLE_LEN 256` → Maximum length for movie title (256 characters)
- `MAX_GENRE_LEN 64` → Maximum length for genre name (64 characters)
- `MAX_DIRECTOR_LEN 128` → Maximum length for director name (128 characters)
- `HASH_TABLE_SIZE 211` → Size of hash table (211 is a prime number for better distribution)
- `MAX_MOVIES 500` → Maximum number of movies the system can handle
- `MAX_RECOMMENDATIONS 20` → Number of recommendations to return

---

## Section 2: Edge Types (Enum)

```c
typedef enum {
    GENRE_SIMILAR,
    RATING_SIMILAR,
    DIRECTOR_SIMILAR
} EdgeType;
```

**Explanation:**
This creates a custom data type called `EdgeType` with three possible values:
- `GENRE_SIMILAR` = 0 → Two movies have the same genre
- `RATING_SIMILAR` = 1 → Two movies have similar ratings (difference ≤ 0.5)
- `DIRECTOR_SIMILAR` = 2 → Two movies have the same director

These are used to label the connections (edges) between movies in the knowledge graph.

---

## Section 3: Movie Structure

```c
typedef struct {
    int id;
    char title[MAX_TITLE_LEN];
    char genre[MAX_GENRE_LEN];
    float rating;
    char director[MAX_DIRECTOR_LEN];
} Movie;
```

**Explanation:**
This structure stores information about ONE movie:
- `id` → Unique number to identify the movie (like 1, 2, 3...)
- `title` → Name of the movie (like "The Matrix")
- `genre` → Category of movie (like "Action", "Sci-Fi")
- `rating` → Movie rating from 0 to 10 (like 8.7)
- `director` → Name of the director (like "Christopher Nolan")

**Example:**
```
id = 1
title = "The Matrix"
genre = "Sci-Fi"
rating = 8.7
director = "Lana Wachowski"
```

---

## Section 4: Hash Table Structures

### Structure: HashNode

```c
typedef struct HashNode {
    Movie movie;
    struct HashNode* next;
} HashNode;
```

**Explanation:**
This is a node in the hash table's linked list:
- `movie` → Stores the actual movie data
- `next` → Pointer to the next node (used when multiple movies hash to same index)

Think of it like a box that holds one movie and has a string attached to the next box.

---

### Structure: HashTable

```c
typedef struct {
    HashNode* buckets[HASH_TABLE_SIZE];
    int count;
} HashTable;
```

**Explanation:**
This is the main hash table structure:
- `buckets[211]` → An array of 211 slots, each can hold a chain of movies
- `count` → Total number of movies stored in the table

Think of it like a filing cabinet with 211 drawers. Each drawer can hold multiple files (movies).

---

## Section 5: Knowledge Graph Structures

### Structure: GraphEdge

```c
typedef struct GraphEdge {
    int targetMovieId;
    EdgeType edgeType;
    struct GraphEdge* next;
} GraphEdge;
```

**Explanation:**
This represents a connection (edge) between two movies:
- `targetMovieId` → The ID of the movie this edge connects to
- `edgeType` → Type of relationship (GENRE_SIMILAR, RATING_SIMILAR, or DIRECTOR_SIMILAR)
- `next` → Pointer to the next edge (since one movie can connect to many others)

**Example:** If "The Matrix" connects to "Inception" because they have similar genres:
```
targetMovieId = 4 (Inception's ID)
edgeType = GENRE_SIMILAR
```

---

### Structure: GraphNode

```c
typedef struct GraphNode {
    int movieId;
    GraphEdge* edges;
    struct GraphNode* next;
} GraphNode;
```

**Explanation:**
This represents a movie in the knowledge graph:
- `movieId` → The ID of this movie
- `edges` → Pointer to the first edge (connection to other movies)
- `next` → Pointer to next node (for hash-based storage)

Think of each movie as a person, and edges are their friendships with other people.

---

### Structure: KnowledgeGraph

```c
typedef struct {
    GraphNode* nodes[HASH_TABLE_SIZE];
    int nodeCount;
} KnowledgeGraph;
```

**Explanation:**
This is the complete knowledge graph:
- `nodes[211]` → Array of 211 slots to store graph nodes (same principle as hash table)
- `nodeCount` → Total number of movies in the graph

---

## Section 6: Queue Structures

### Structure: QueueNode

```c
typedef struct QueueNode {
    int movieId;
    struct QueueNode* next;
} QueueNode;
```

**Explanation:**
A node in the queue:
- `movieId` → The movie ID stored in this queue position
- `next` → Pointer to the next item in queue

---

### Structure: Queue

```c
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int size;
} Queue;
```

**Explanation:**
First-In-First-Out (FIFO) data structure:
- `front` → Pointer to the first item (where we remove from)
- `rear` → Pointer to the last item (where we add new items)
- `size` → Number of items currently in queue

Think of it like a line at a ticket counter - first person in line gets served first.

---

## Section 7: Candidate Structure

```c
typedef struct {
    int movieId;
    int score;
    float rating;
} Candidate;
```

**Explanation:**
Used to store potential recommendation movies:
- `movieId` → The ID of the candidate movie
- `score` → Total weighted score (higher = better match)
- `rating` → Movie rating (used to break ties)

---

# PART 2: recommender.c (Implementation File)

This file contains all the actual code that makes the system work.

---

## Section 1: Hash Table Functions

### Function: initHashTable()

```c
void initHashTable(HashTable *ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    ht->count = 0;
}
```

**Line-by-line explanation:**
- `for (int i = 0; i < HASH_TABLE_SIZE; i++)` → Loop through all 211 buckets
- `ht->buckets[i] = NULL` → Set each bucket to empty (NULL means nothing there)
- `ht->count = 0` → Set movie count to zero

**Purpose:** Prepares an empty hash table before we start adding movies.

---

### Function: hashFunction()

```c
unsigned int hashFunction(int movieId) {
    return (unsigned int)(movieId % HASH_TABLE_SIZE);
}
```

**Explanation:**
- Takes a movie ID as input
- Returns `movieId % 211` (remainder when divided by 211)
- This gives a number between 0 and 210 (the bucket index)

**Example:**
- Movie ID = 5 → `5 % 211 = 5` → Goes in bucket 5
- Movie ID = 212 → `212 % 211 = 1` → Goes in bucket 1

**Purpose:** Converts any movie ID into a valid bucket index.

---

### Function: insertMovie()

```c
void insertMovie(HashTable *ht, Movie movie) {
    unsigned int index = hashFunction(movie.id);
    HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
    if (newNode == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        return;
    }
    newNode->movie = movie;
    newNode->next = ht->buckets[index];
    ht->buckets[index] = newNode;
    ht->count++;
}
```

**Line-by-line explanation:**
1. `unsigned int index = hashFunction(movie.id)` → Calculate which bucket this movie goes into
2. `HashNode *newNode = malloc(...)` → Allocate memory for a new node
3. `if (newNode == NULL)` → Check if memory allocation failed
4. `newNode->movie = movie` → Copy the movie data into the new node
5. `newNode->next = ht->buckets[index]` → Point new node to whatever was already in this bucket
6. `ht->buckets[index] = newNode` → Make the new node the head of the bucket's chain
7. `ht->count++` → Increment total movie count

**Purpose:** Adds a movie to the hash table using separate chaining for collision handling.

---

### Function: findMovie()

```c
Movie *findMovie(HashTable *ht, int movieId) {
    unsigned int index = hashFunction(movieId);
    HashNode *current = ht->buckets[index];
    
    while (current != NULL) {
        if (current->movie.id == movieId) {
            return &(current->movie);
        }
        current = current->next;
    }
    return NULL;
}
```

**Line-by-line explanation:**
1. `unsigned int index = hashFunction(movieId)` → Find which bucket to search
2. `HashNode *current = ht->buckets[index]` → Start at the beginning of that bucket's chain
3. `while (current != NULL)` → Loop through all nodes in the chain
4. `if (current->movie.id == movieId)` → Check if we found the movie
5. `return &(current->movie)` → Found it! Return pointer to the movie
6. `current = current->next` → Not found yet, move to next node
7. `return NULL` → Movie not found

**Time Complexity:** O(1) average case, O(n) worst case (if all movies hash to same bucket)

---

### Function: freeHashTable()

```c
void freeHashTable(HashTable *ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL) {
            HashNode *temp = current;
            current = current->next;
            free(temp);
        }
        ht->buckets[i] = NULL;
    }
    ht->count = 0;
}
```

**Purpose:** Frees all memory used by the hash table to prevent memory leaks.

**How it works:**
1. Loop through each bucket
2. For each bucket, traverse the chain and free each node
3. Set bucket to NULL
4. Reset count to 0

---

## Section 2: Knowledge Graph Functions

### Function: initKnowledgeGraph()

```c
void initKnowledgeGraph(KnowledgeGraph *kg) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        kg->nodes[i] = NULL;
    }
    kg->nodeCount = 0;
}
```

**Purpose:** Initializes an empty knowledge graph (similar to initHashTable).

---

### Function: getGraphNode()

```c
GraphNode *getGraphNode(KnowledgeGraph *kg, int movieId) {
    unsigned int index = hashFunction(movieId);
    GraphNode *current = kg->nodes[index];
    
    while (current != NULL) {
        if (current->movieId == movieId) {
            return current;
        }
        current = current->next;
    }
    
    // Create new node if not found
    GraphNode *newNode = (GraphNode *)malloc(sizeof(GraphNode));
    newNode->movieId = movieId;
    newNode->edges = NULL;
    newNode->next = kg->nodes[index];
    kg->nodes[index] = newNode;
    kg->nodeCount++;
    
    return newNode;
}
```

**Purpose:** Gets or creates a graph node for a movie.

**Logic:**
1. First, search if node already exists
2. If found, return it
3. If not found, create a new node and add it to the graph

---

### Function: addDirectedEdge() (Static/Helper)

```c
static void addDirectedEdge(KnowledgeGraph *kg, int sourceId, int targetId, EdgeType type) {
    GraphNode *sourceNode = getGraphNode(kg, sourceId);
    
    // Check if edge already exists
    GraphEdge *current = sourceNode->edges;
    while (current != NULL) {
        if (current->targetMovieId == targetId && current->edgeType == type) {
            return;  // Already exists
        }
        current = current->next;
    }
    
    // Create new edge
    GraphEdge *newEdge = (GraphEdge *)malloc(sizeof(GraphEdge));
    newEdge->targetMovieId = targetId;
    newEdge->edgeType = type;
    newEdge->next = sourceNode->edges;
    sourceNode->edges = newEdge;
}
```

**Purpose:** Creates a one-way connection from source movie to target movie.

---

### Function: addEdge()

```c
void addEdge(KnowledgeGraph *kg, int movieId1, int movieId2, EdgeType type) {
    addDirectedEdge(kg, movieId1, movieId2, type);
    addDirectedEdge(kg, movieId2, movieId1, type);
}
```

**Purpose:** Creates a TWO-WAY (bidirectional) connection between two movies.

**Example:** If Matrix and Inception are genre-similar:
- Matrix → Inception (GENRE_SIMILAR)
- Inception → Matrix (GENRE_SIMILAR)

---

### Function: strcasecmp_custom() (Helper)

```c
static int strcasecmp_custom(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}
```

**Purpose:** Compares two strings ignoring uppercase/lowercase differences.

**Example:** "Action" and "action" would be considered equal.

---

### Function: buildKnowledgeGraph()

```c
void buildKnowledgeGraph(KnowledgeGraph *kg, HashTable *ht) {
    // Step 1: Collect all movies from hash table
    Movie *movies[MAX_MOVIES];
    int movieCount = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *current = ht->buckets[i];
        while (current != NULL && movieCount < MAX_MOVIES) {
            movies[movieCount++] = &(current->movie);
            current = current->next;
        }
    }
    
    // Step 2: Compare every pair of movies
    for (int i = 0; i < movieCount; i++) {
        for (int j = i + 1; j < movieCount; j++) {
            Movie *m1 = movies[i];
            Movie *m2 = movies[j];
            
            // Check genre similarity
            if (strcasecmp_custom(m1->genre, m2->genre) == 0) {
                addEdge(kg, m1->id, m2->id, GENRE_SIMILAR);
            }
            
            // Check rating similarity (difference <= 0.5)
            if (fabs(m1->rating - m2->rating) <= 0.5) {
                addEdge(kg, m1->id, m2->id, RATING_SIMILAR);
            }
            
            // Check director similarity
            if (strcasecmp_custom(m1->director, m2->director) == 0) {
                addEdge(kg, m1->id, m2->id, DIRECTOR_SIMILAR);
            }
        }
    }
}
```

**Step-by-step explanation:**

1. **Collect all movies** from hash table into an array
2. **Compare every pair** using nested loops
3. For each pair, check three possible relationships:
   - Same genre? Add GENRE_SIMILAR edge
   - Ratings differ by ≤ 0.5? Add RATING_SIMILAR edge
   - Same director? Add DIRECTOR_SIMILAR edge

**Example:** Comparing "The Matrix" and "Inception":
- Both are "Sci-Fi" → Add GENRE_SIMILAR edge
- Matrix: 8.7, Inception: 8.8 (difference = 0.1 ≤ 0.5) → Add RATING_SIMILAR edge
- Different directors → No DIRECTOR_SIMILAR edge

---

## Section 3: Queue Functions

### Function: initQueue()

```c
void initQueue(Queue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}
```

**Purpose:** Creates an empty queue.

---

### Function: isQueueEmpty()

```c
int isQueueEmpty(Queue *q) {
    return q->front == NULL;
}
```

**Purpose:** Returns 1 (true) if queue is empty, 0 (false) otherwise.

---

### Function: enqueue()

```c
void enqueue(Queue *q, int movieId) {
    QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
    newNode->movieId = movieId;
    newNode->next = NULL;
    
    if (q->rear == NULL) {
        q->front = q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}
```

**Purpose:** Adds a movie ID to the end of the queue.

---

### Function: dequeue()

```c
int dequeue(Queue *q) {
    if (isQueueEmpty(q)) {
        return -1;
    }
    
    QueueNode *temp = q->front;
    int movieId = temp->movieId;
    
    q->front = q->front->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }
    
    free(temp);
    q->size--;
    
    return movieId;
}
```

**Purpose:** Removes and returns the movie ID from the front of the queue.

---

## Section 4: Weighted Recommendation Algorithm

### Function: compareCandidates()

```c
int compareCandidates(const void *a, const void *b) {
    const Candidate *ca = (const Candidate *)a;
    const Candidate *cb = (const Candidate *)b;
    
    // First compare by score (descending)
    if (cb->score != ca->score) {
        return cb->score - ca->score;
    }
    
    // Tie-breaker: compare by rating (descending)
    if (cb->rating > ca->rating) return 1;
    if (cb->rating < ca->rating) return -1;
    return 0;
}
```

**Purpose:** Comparison function for sorting. Used by `qsort()`.

**Sorting order:**
1. Higher score comes first
2. If scores are equal, higher rating comes first

---

### Function: recommendMoviesWeighted() - THE MAIN ALGORITHM

```c
int recommendMoviesWeighted(
    KnowledgeGraph *kg,
    HashTable *ht,
    int baseMovieId,
    int genreWeight,
    int ratingWeight,
    int directorWeight,
    Candidate *results,
    int maxResults
)
```

**Parameters:**
- `kg` → The knowledge graph
- `ht` → The hash table with movie data
- `baseMovieId` → The movie user selected
- `genreWeight` → User's weight for genre (0-10)
- `ratingWeight` → User's weight for rating (0-10)
- `directorWeight` → User's weight for director (0-10)
- `results` → Array to store recommendations
- `maxResults` → Maximum number of recommendations (20)

**Algorithm Step-by-Step:**

**STEP 1: Initialize score tracking**
```c
int scores[MAX_MOVIES];
int movieIds[MAX_MOVIES];
int candidateCount = 0;

for (int i = 0; i < MAX_MOVIES; i++) {
    scores[i] = -1;
    movieIds[i] = -1;
}
```
Creates arrays to track movie IDs and their scores.

---

**STEP 2: Get the base movie's node from graph**
```c
GraphNode *baseNode = getGraphNode(kg, baseMovieId);
if (baseNode == NULL) {
    return 0;
}
```
Find the selected movie in the knowledge graph.

---

**STEP 3: Traverse all edges (connections) from base movie**
```c
GraphEdge *edge = baseNode->edges;
while (edge != NULL) {
    int targetId = edge->targetMovieId;
    
    // Skip the base movie itself
    if (targetId == baseMovieId) {
        edge = edge->next;
        continue;
    }
    
    // Find or create entry for this candidate
    int candidateIdx = -1;
    for (int i = 0; i < candidateCount; i++) {
        if (movieIds[i] == targetId) {
            candidateIdx = i;
            break;
        }
    }
    
    if (candidateIdx == -1) {
        candidateIdx = candidateCount++;
        movieIds[candidateIdx] = targetId;
        scores[candidateIdx] = 0;
    }
    
    // Add weight based on edge type
    switch (edge->edgeType) {
        case GENRE_SIMILAR:
            scores[candidateIdx] += genreWeight;
            break;
        case RATING_SIMILAR:
            scores[candidateIdx] += ratingWeight;
            break;
        case DIRECTOR_SIMILAR:
            scores[candidateIdx] += directorWeight;
            break;
    }
    
    edge = edge->next;
}
```

**Explanation:**
- Go through each connected movie
- For each connection, add the appropriate weight to that movie's score
- If a movie has multiple connections (e.g., same genre AND same director), scores add up!

**Example:**
If user selects "The Matrix" with weights: genre=5, rating=3, director=10

Movie "Matrix Reloaded" has edges:
- GENRE_SIMILAR → Add 5 points
- RATING_SIMILAR → Add 3 points
- DIRECTOR_SIMILAR → Add 10 points

Total score for Matrix Reloaded = 5 + 3 + 10 = **18 points**

---

**STEP 4: Build candidate array with scores**
```c
Candidate candidates[MAX_MOVIES];
int validCount = 0;

for (int i = 0; i < candidateCount; i++) {
    Movie *movie = findMovie(ht, movieIds[i]);
    if (movie != NULL && scores[i] > 0) {
        candidates[validCount].movieId = movieIds[i];
        candidates[validCount].score = scores[i];
        candidates[validCount].rating = movie->rating;
        validCount++;
    }
}
```
Create proper Candidate structures with movieId, score, and rating.

---

**STEP 5: Sort candidates**
```c
qsort(candidates, validCount, sizeof(Candidate), compareCandidates);
```
Sort all candidates by score (highest first), then by rating (highest first for ties).

---

**STEP 6: Return top results**
```c
int resultCount = (validCount < maxResults) ? validCount : maxResults;
for (int i = 0; i < resultCount; i++) {
    results[i] = candidates[i];
}
return resultCount;
```
Copy the top 20 (or fewer) recommendations to the results array.

---

## Section 5: File I/O Functions

### Function: loadMovies()

```c
int loadMovies(const char *filename, HashTable *ht)
```

**Purpose:** Reads movies from CSV file and inserts them into hash table.

**File format expected:**
```
id,title,genre,rating,director
1,The Matrix,Sci-Fi,8.7,Lana Wachowski
2,Inception,Sci-Fi,8.8,Christopher Nolan
```

**Process:**
1. Open file
2. Skip header line
3. For each line:
   - Parse ID, title, genre, rating, director using `strtok()`
   - Create Movie struct
   - Insert into hash table
4. Close file
5. Return count of movies loaded

---

### Function: printRecommendation()

```c
void printRecommendation(Movie *movie) {
    printf("%d,%s,%s,%.1f,%s\n", 
           movie->id, movie->title, movie->genre, 
           movie->rating, movie->director);
}
```

**Purpose:** Prints a movie in CSV format for output.

---

## Section 6: Main Function

```c
int main(int argc, char *argv[])
```

**Command line arguments:**
```
./recommender <movie_id> <genre_weight> <rating_weight> <director_weight>
```

**Example:**
```
./recommender 1 5 3 10
```
This means: Recommend movies similar to movie ID 1, with genre weight=5, rating weight=3, director weight=10.

---

### Main Function Flow:

**STEP 1: Parse and validate arguments**
```c
int baseMovieId = atoi(argv[1]);
int genreWeight = atoi(argv[2]);
int ratingWeight = atoi(argv[3]);
int directorWeight = atoi(argv[4]);
```

---

**STEP 2: Initialize data structures**
```c
HashTable hashTable;
KnowledgeGraph knowledgeGraph;

initHashTable(&hashTable);
initKnowledgeGraph(&knowledgeGraph);
```

---

**STEP 3: Load movies from file**
```c
int movieCount = loadMovies("movies.txt", &hashTable);
```

---

**STEP 4: Verify base movie exists**
```c
Movie *baseMovie = findMovie(&hashTable, baseMovieId);
if (baseMovie == NULL) {
    // Error handling
}
```

---

**STEP 5: Build knowledge graph**
```c
buildKnowledgeGraph(&knowledgeGraph, &hashTable);
```

---

**STEP 6: Generate recommendations**
```c
Candidate recommendations[MAX_RECOMMENDATIONS];
int recCount = recommendMoviesWeighted(
    &knowledgeGraph, &hashTable, baseMovieId,
    genreWeight, ratingWeight, directorWeight,
    recommendations, MAX_RECOMMENDATIONS
);
```

---

**STEP 7: Print results**
```c
for (int i = 0; i < recCount; i++) {
    Movie *movie = findMovie(&hashTable, recommendations[i].movieId);
    printRecommendation(movie);
}
```

---

**STEP 8: Cleanup memory**
```c
freeKnowledgeGraph(&knowledgeGraph);
freeHashTable(&hashTable);
```

---

# PART 3: Complete System Flow

## Summary: How Everything Works Together

```
┌─────────────────────────────────────────────────────────────┐
│                    COMPLETE FLOW DIAGRAM                     │
└─────────────────────────────────────────────────────────────┘

1. LOAD MOVIES
   movies.txt → loadMovies() → Hash Table
   
2. BUILD KNOWLEDGE GRAPH
   Hash Table → buildKnowledgeGraph() → Knowledge Graph
   (Compare all pairs, create edges for similar movies)

3. USER INPUT
   Command line: movie_id, genre_weight, rating_weight, director_weight

4. FIND BASE MOVIE
   movie_id → findMovie() → Movie struct

5. GENERATE RECOMMENDATIONS
   recommendMoviesWeighted():
   │
   ├── Get base movie's graph node
   ├── Traverse all edges
   ├── For each connected movie:
   │   └── Add weights based on edge type
   ├── Sort by score (then rating)
   └── Return top 20

6. OUTPUT
   Print recommendations in CSV format
```

---

## Data Structures Summary Table

| Structure | Purpose | Key Operations |
|-----------|---------|----------------|
| Movie | Store one movie's data | - |
| HashNode | Node in hash table chain | - |
| HashTable | Fast movie lookup O(1) | init, insert, find, free |
| GraphEdge | Connection between movies | - |
| GraphNode | Movie in knowledge graph | - |
| KnowledgeGraph | All movie relationships | init, addEdge, build, free |
| QueueNode | Node in BFS queue | - |
| Queue | FIFO for BFS traversal | init, enqueue, dequeue, free |
| Candidate | Recommendation with score | - |

---

## Algorithm Complexity

| Operation | Time Complexity | Explanation |
|-----------|-----------------|-------------|
| Hash lookup | O(1) average | Direct bucket access |
| Insert movie | O(1) | Add to head of chain |
| Build graph | O(n²) | Compare all pairs |
| Generate recommendations | O(m + n log n) | Traverse edges + sort |

Where:
- n = number of movies
- m = number of edges (connections)

---

## Key Concepts for Viva

1. **Hash Table with Separate Chaining**
   - Uses linked lists to handle collisions
   - Prime number size (211) for better distribution

2. **Knowledge Graph**
   - Adjacency list representation
   - Bidirectional edges (if A→B, then B→A)
   - Three edge types: genre, rating, director

3. **Weighted Scoring**
   - User defines importance of each similarity type
   - Scores accumulate for multiple relationships
   - Higher score = more similar movie

4. **Sorting**
   - Primary: Score (descending)
   - Secondary: Rating (descending, tie-breaker)

---

**End of Documentation**
