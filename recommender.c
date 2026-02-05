/*
 * recommender.c - Movie Recommender System Core Engine
 *
 * Implements:
 * - Hash Table for O(1) movie lookup
 * - Knowledge Graph with genre, rating, and director similarity edges
 * - Weighted scoring recommendation algorithm
 *
 * Usage: ./recommender <movie_id> <genre_weight> <rating_weight>
 * <director_weight>
 */

#include "movie.h"

/* =====================================================
 * HASH TABLE IMPLEMENTATION
 * ===================================================== */

/*
 * Initialize hash table - set all buckets to NULL
 */
void initHashTable(HashTable *ht) {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    ht->buckets[i] = NULL;
  }
  ht->count = 0;
}

/*
 * Hash function using modulo prime
 * Provides good distribution for integer keys
 */
unsigned int hashFunction(int movieId) {
  return (unsigned int)(movieId % HASH_TABLE_SIZE);
}

/*
 * Insert movie into hash table
 * Uses separate chaining for collision handling
 */
void insertMovie(HashTable *ht, Movie movie) {
  unsigned int index = hashFunction(movie.id);

  /* Create new hash node */
  HashNode *newNode = (HashNode *)malloc(sizeof(HashNode));
  if (newNode == NULL) {
    fprintf(stderr, "Error: Memory allocation failed for hash node\n");
    return;
  }

  newNode->movie = movie;
  newNode->next = ht->buckets[index]; /* Insert at head of chain */
  ht->buckets[index] = newNode;
  ht->count++;
}

/*
 * Find movie by ID
 * Returns pointer to movie or NULL if not found
 * Average time complexity: O(1)
 */
Movie *findMovie(HashTable *ht, int movieId) {
  unsigned int index = hashFunction(movieId);
  HashNode *current = ht->buckets[index];

  while (current != NULL) {
    if (current->movie.id == movieId) {
      return &(current->movie);
    }
    current = current->next;
  }

  return NULL; /* Movie not found */
}

/*
 * Free all memory used by hash table
 */
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

/* =====================================================
 * KNOWLEDGE GRAPH IMPLEMENTATION
 * ===================================================== */

/*
 * Initialize knowledge graph - set all node slots to NULL
 */
void initKnowledgeGraph(KnowledgeGraph *kg) {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    kg->nodes[i] = NULL;
  }
  kg->nodeCount = 0;
}

/*
 * Get or create graph node for a movie ID
 */
GraphNode *getGraphNode(KnowledgeGraph *kg, int movieId) {
  unsigned int index = hashFunction(movieId);
  GraphNode *current = kg->nodes[index];

  /* Search for existing node */
  while (current != NULL) {
    if (current->movieId == movieId) {
      return current;
    }
    current = current->next;
  }

  /* Create new node if not found */
  GraphNode *newNode = (GraphNode *)malloc(sizeof(GraphNode));
  if (newNode == NULL) {
    fprintf(stderr, "Error: Memory allocation failed for graph node\n");
    return NULL;
  }

  newNode->movieId = movieId;
  newNode->edges = NULL;
  newNode->next = kg->nodes[index];
  kg->nodes[index] = newNode;
  kg->nodeCount++;

  return newNode;
}

/*
 * Add a single directed edge from source to target
 */
static void addDirectedEdge(KnowledgeGraph *kg, int sourceId, int targetId,
                            EdgeType type) {
  GraphNode *sourceNode = getGraphNode(kg, sourceId);
  if (sourceNode == NULL)
    return;

  /* Check if edge already exists */
  GraphEdge *current = sourceNode->edges;
  while (current != NULL) {
    if (current->targetMovieId == targetId && current->edgeType == type) {
      return; /* Edge already exists */
    }
    current = current->next;
  }

  /* Create new edge */
  GraphEdge *newEdge = (GraphEdge *)malloc(sizeof(GraphEdge));
  if (newEdge == NULL) {
    fprintf(stderr, "Error: Memory allocation failed for graph edge\n");
    return;
  }

  newEdge->targetMovieId = targetId;
  newEdge->edgeType = type;
  newEdge->next = sourceNode->edges;
  sourceNode->edges = newEdge;
}

/*
 * Add bidirectional edge between two movies
 */
void addEdge(KnowledgeGraph *kg, int movieId1, int movieId2, EdgeType type) {
  addDirectedEdge(kg, movieId1, movieId2, type);
  addDirectedEdge(kg, movieId2, movieId1, type);
}

/*
 * Case-insensitive string comparison helper
 */
static int strcasecmp_custom(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
    char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
    if (c1 != c2)
      return c1 - c2;
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

/*
 * Build knowledge graph by comparing all movie pairs
 * Creates edges based on genre, rating, and director similarity
 */
void buildKnowledgeGraph(KnowledgeGraph *kg, HashTable *ht) {
  /* Collect all movies into an array for pairwise comparison */
  Movie *movies[MAX_MOVIES];
  int movieCount = 0;

  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    HashNode *current = ht->buckets[i];
    while (current != NULL && movieCount < MAX_MOVIES) {
      movies[movieCount++] = &(current->movie);
      current = current->next;
    }
  }

  /* Compare all pairs of movies */
  for (int i = 0; i < movieCount; i++) {
    for (int j = i + 1; j < movieCount; j++) {
      Movie *m1 = movies[i];
      Movie *m2 = movies[j];

      /* Check genre similarity */
      if (strcasecmp_custom(m1->genre, m2->genre) == 0) {
        addEdge(kg, m1->id, m2->id, GENRE_SIMILAR);
      }

      /* Check rating similarity (difference <= 0.5) */
      if (fabs(m1->rating - m2->rating) <= 0.5) {
        addEdge(kg, m1->id, m2->id, RATING_SIMILAR);
      }

      /* Check director similarity */
      if (strcasecmp_custom(m1->director, m2->director) == 0) {
        addEdge(kg, m1->id, m2->id, DIRECTOR_SIMILAR);
      }
    }
  }
}

/*
 * Free all memory used by knowledge graph
 */
void freeKnowledgeGraph(KnowledgeGraph *kg) {
  for (int i = 0; i < HASH_TABLE_SIZE; i++) {
    GraphNode *node = kg->nodes[i];
    while (node != NULL) {
      /* Free all edges for this node */
      GraphEdge *edge = node->edges;
      while (edge != NULL) {
        GraphEdge *tempEdge = edge;
        edge = edge->next;
        free(tempEdge);
      }

      GraphNode *tempNode = node;
      node = node->next;
      free(tempNode);
    }
    kg->nodes[i] = NULL;
  }
  kg->nodeCount = 0;
}

/* =====================================================
 * QUEUE IMPLEMENTATION (For BFS)
 * ===================================================== */

/*
 * Initialize empty queue
 */
void initQueue(Queue *q) {
  q->front = NULL;
  q->rear = NULL;
  q->size = 0;
}

/*
 * Check if queue is empty
 */
int isQueueEmpty(Queue *q) { return q->front == NULL; }

/*
 * Add movie ID to rear of queue
 */
void enqueue(Queue *q, int movieId) {
  QueueNode *newNode = (QueueNode *)malloc(sizeof(QueueNode));
  if (newNode == NULL) {
    fprintf(stderr, "Error: Memory allocation failed for queue node\n");
    return;
  }

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

/*
 * Remove and return movie ID from front of queue
 * Returns -1 if queue is empty
 */
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

/*
 * Free all memory used by queue
 */
void freeQueue(Queue *q) {
  while (!isQueueEmpty(q)) {
    dequeue(q);
  }
}

/* =====================================================
 * WEIGHTED RECOMMENDATION ALGORITHM
 * ===================================================== */

/*
 * Comparison function for sorting candidates
 * Sort by: 1. Total score (descending), 2. Rating (descending)
 */
int compareCandidates(const void *a, const void *b) {
  const Candidate *ca = (const Candidate *)a;
  const Candidate *cb = (const Candidate *)b;

  /* First compare by score (descending) */
  if (cb->score != ca->score) {
    return cb->score - ca->score;
  }

  /* Tie-breaker: compare by rating (descending) */
  if (cb->rating > ca->rating)
    return 1;
  if (cb->rating < ca->rating)
    return -1;
  return 0;
}

/*
 * Generate weighted recommendations based on knowledge graph
 *
 * Algorithm:
 * 1. Get all neighbors of base movie from knowledge graph
 * 2. For each neighbor, calculate weighted score based on edge types
 * 3. Accumulate scores for movies with multiple edge types
 * 4. Sort by score (descending), then rating (descending)
 * 5. Return top N unique recommendations
 */
int recommendMoviesWeighted(KnowledgeGraph *kg, HashTable *ht, int baseMovieId,
                            int genreWeight, int ratingWeight,
                            int directorWeight, Candidate *results,
                            int maxResults) {
  /* Array to track scores for all movies */
  int scores[MAX_MOVIES];
  int movieIds[MAX_MOVIES];
  int candidateCount = 0;

  /* Initialize scores to -1 (not visited) */
  for (int i = 0; i < MAX_MOVIES; i++) {
    scores[i] = -1;
    movieIds[i] = -1;
  }

  /* Get the graph node for base movie */
  GraphNode *baseNode = getGraphNode(kg, baseMovieId);
  if (baseNode == NULL) {
    return 0; /* Base movie not in graph */
  }

  /* Traverse all edges from base movie */
  GraphEdge *edge = baseNode->edges;
  while (edge != NULL) {
    int targetId = edge->targetMovieId;

    /* Skip the base movie itself */
    if (targetId == baseMovieId) {
      edge = edge->next;
      continue;
    }

    /* Find or create candidate entry */
    int candidateIdx = -1;
    for (int i = 0; i < candidateCount; i++) {
      if (movieIds[i] == targetId) {
        candidateIdx = i;
        break;
      }
    }

    if (candidateIdx == -1) {
      /* New candidate */
      candidateIdx = candidateCount++;
      movieIds[candidateIdx] = targetId;
      scores[candidateIdx] = 0;
    }

    /* Add weight based on edge type */
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

  /* Build candidate array with movie info */
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

  /* Sort candidates by score (desc), then rating (desc) */
  qsort(candidates, validCount, sizeof(Candidate), compareCandidates);

  /* Copy top results */
  int resultCount = (validCount < maxResults) ? validCount : maxResults;
  for (int i = 0; i < resultCount; i++) {
    results[i] = candidates[i];
  }

  return resultCount;
}

/* =====================================================
 * FILE I/O
 * ===================================================== */

/*
 * Load movies from CSV file
 * Format: id,title,genre,rating,director
 */
int loadMovies(const char *filename, HashTable *ht) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error: Cannot open file %s\n", filename);
    return 0;
  }

  char line[1024];
  int count = 0;

  /* Skip header line */
  if (fgets(line, sizeof(line), file) == NULL) {
    fclose(file);
    return 0;
  }

  /* Read each movie */
  while (fgets(line, sizeof(line), file) != NULL) {
    Movie movie;
    char *token;
    char *saveptr;

    /* Remove trailing newline */
    size_t len = strlen(line);
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
      len--;
    }
    if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
      line[len - 1] = '\0';
    }

    /* Parse ID */
    token = strtok(line, ",");
    if (token == NULL)
      continue;
    movie.id = atoi(token);

    /* Parse Title */
    token = strtok(NULL, ",");
    if (token == NULL)
      continue;
    strncpy(movie.title, token, MAX_TITLE_LEN - 1);
    movie.title[MAX_TITLE_LEN - 1] = '\0';

    /* Parse Genre */
    token = strtok(NULL, ",");
    if (token == NULL)
      continue;
    strncpy(movie.genre, token, MAX_GENRE_LEN - 1);
    movie.genre[MAX_GENRE_LEN - 1] = '\0';

    /* Parse Rating */
    token = strtok(NULL, ",");
    if (token == NULL)
      continue;
    movie.rating = (float)atof(token);

    /* Parse Director */
    token = strtok(NULL, ",\r\n");
    if (token == NULL)
      continue;
    strncpy(movie.director, token, MAX_DIRECTOR_LEN - 1);
    movie.director[MAX_DIRECTOR_LEN - 1] = '\0';

    insertMovie(ht, movie);
    count++;
  }

  fclose(file);
  return count;
}

/*
 * Print movie recommendation in CSV format
 */
void printRecommendation(Movie *movie) {
  printf("%d,%s,%s,%.1f,%s\n", movie->id, movie->title, movie->genre,
         movie->rating, movie->director);
}

/* =====================================================
 * MAIN FUNCTION
 * ===================================================== */

int main(int argc, char *argv[]) {
  /* Validate command line arguments */
  if (argc != 5) {
    fprintf(stderr,
            "Usage: %s <movie_id> <genre_weight> <rating_weight> "
            "<director_weight>\n",
            argv[0]);
    fprintf(stderr, "  movie_id: ID of base movie (integer)\n");
    fprintf(stderr, "  genre_weight: Weight for genre similarity (0-10)\n");
    fprintf(stderr, "  rating_weight: Weight for rating similarity (0-10)\n");
    fprintf(stderr,
            "  director_weight: Weight for director similarity (0-10)\n");
    return 1;
  }

  int baseMovieId = atoi(argv[1]);
  int genreWeight = atoi(argv[2]);
  int ratingWeight = atoi(argv[3]);
  int directorWeight = atoi(argv[4]);

  /* Validate weights are in range 0-10 */
  if (genreWeight < 0 || genreWeight > 10 || ratingWeight < 0 ||
      ratingWeight > 10 || directorWeight < 0 || directorWeight > 10) {
    fprintf(stderr, "Error: Weights must be between 0 and 10\n");
    return 1;
  }

  /* Initialize data structures */
  HashTable hashTable;
  KnowledgeGraph knowledgeGraph;

  initHashTable(&hashTable);
  initKnowledgeGraph(&knowledgeGraph);

  /* Load movies from file */
  int movieCount = loadMovies("movies.txt", &hashTable);
  if (movieCount == 0) {
    fprintf(stderr, "Error: No movies loaded from file\n");
    freeHashTable(&hashTable);
    return 1;
  }

  /* Verify base movie exists */
  Movie *baseMovie = findMovie(&hashTable, baseMovieId);
  if (baseMovie == NULL) {
    fprintf(stderr, "Error: Movie with ID %d not found\n", baseMovieId);
    freeHashTable(&hashTable);
    return 1;
  }

  /* Build knowledge graph */
  buildKnowledgeGraph(&knowledgeGraph, &hashTable);

  /* Generate recommendations */
  Candidate recommendations[MAX_RECOMMENDATIONS];
  int recCount = recommendMoviesWeighted(
      &knowledgeGraph, &hashTable, baseMovieId, genreWeight, ratingWeight,
      directorWeight, recommendations, MAX_RECOMMENDATIONS);

  /* Print recommendations in CSV format */
  for (int i = 0; i < recCount; i++) {
    Movie *movie = findMovie(&hashTable, recommendations[i].movieId);
    if (movie != NULL) {
      printRecommendation(movie);
    }
  }

  /* Cleanup */
  freeKnowledgeGraph(&knowledgeGraph);
  freeHashTable(&hashTable);

  return 0;
}
