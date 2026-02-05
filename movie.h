/*
 * movie.h - Header file for Movie Recommender System
 * Contains all data structure definitions for hash table, knowledge graph, and queue
 */

#ifndef MOVIE_H
#define MOVIE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* =====================================================
 * CONSTANTS
 * ===================================================== */

#define MAX_TITLE_LEN 256
#define MAX_GENRE_LEN 64
#define MAX_DIRECTOR_LEN 128
#define HASH_TABLE_SIZE 211    /* Prime number for better distribution */
#define MAX_MOVIES 500
#define MAX_RECOMMENDATIONS 20

/* =====================================================
 * EDGE TYPES FOR KNOWLEDGE GRAPH
 * ===================================================== */

typedef enum {
    GENRE_SIMILAR,      /* Movies share the same genre */
    RATING_SIMILAR,     /* Movies have rating difference <= 0.5 */
    DIRECTOR_SIMILAR    /* Movies share the same director */
} EdgeType;

/* =====================================================
 * MOVIE STRUCTURE
 * ===================================================== */

typedef struct {
    int id;
    char title[MAX_TITLE_LEN];
    char genre[MAX_GENRE_LEN];
    float rating;
    char director[MAX_DIRECTOR_LEN];
} Movie;

/* =====================================================
 * HASH TABLE STRUCTURES (Separate Chaining)
 * ===================================================== */

/* Node for hash table linked list (collision handling) */
typedef struct HashNode {
    Movie movie;
    struct HashNode* next;
} HashNode;

/* Hash table structure */
typedef struct {
    HashNode* buckets[HASH_TABLE_SIZE];
    int count;
} HashTable;

/* =====================================================
 * KNOWLEDGE GRAPH STRUCTURES (Adjacency List)
 * ===================================================== */

/* Edge in the knowledge graph */
typedef struct GraphEdge {
    int targetMovieId;          /* ID of connected movie */
    EdgeType edgeType;          /* Type of relationship */
    struct GraphEdge* next;     /* Next edge in adjacency list */
} GraphEdge;

/* Node in the knowledge graph (represents a movie) */
typedef struct GraphNode {
    int movieId;
    GraphEdge* edges;           /* Head of adjacency list */
    struct GraphNode* next;     /* For hash-based graph storage */
} GraphNode;

/* Knowledge Graph structure */
typedef struct {
    GraphNode* nodes[HASH_TABLE_SIZE];
    int nodeCount;
} KnowledgeGraph;

/* =====================================================
 * QUEUE STRUCTURES (For BFS Traversal)
 * ===================================================== */

typedef struct QueueNode {
    int movieId;
    struct QueueNode* next;
} QueueNode;

typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int size;
} Queue;

/* =====================================================
 * CANDIDATE STRUCTURE (For Weighted Scoring)
 * ===================================================== */

typedef struct {
    int movieId;
    int score;              /* Weighted score based on edge types */
    float rating;           /* Movie rating for tie-breaking */
} Candidate;

/* =====================================================
 * FUNCTION PROTOTYPES - HASH TABLE
 * ===================================================== */

/* Initialize hash table */
void initHashTable(HashTable* ht);

/* Hash function for movie ID */
unsigned int hashFunction(int movieId);

/* Insert movie into hash table */
void insertMovie(HashTable* ht, Movie movie);

/* Find movie by ID - returns pointer or NULL */
Movie* findMovie(HashTable* ht, int movieId);

/* Free hash table memory */
void freeHashTable(HashTable* ht);

/* =====================================================
 * FUNCTION PROTOTYPES - KNOWLEDGE GRAPH
 * ===================================================== */

/* Initialize knowledge graph */
void initKnowledgeGraph(KnowledgeGraph* kg);

/* Add bidirectional edge between two movies */
void addEdge(KnowledgeGraph* kg, int movieId1, int movieId2, EdgeType type);

/* Get graph node for a movie ID */
GraphNode* getGraphNode(KnowledgeGraph* kg, int movieId);

/* Build knowledge graph from hash table of movies */
void buildKnowledgeGraph(KnowledgeGraph* kg, HashTable* ht);

/* Free knowledge graph memory */
void freeKnowledgeGraph(KnowledgeGraph* kg);

/* =====================================================
 * FUNCTION PROTOTYPES - QUEUE
 * ===================================================== */

/* Initialize queue */
void initQueue(Queue* q);

/* Check if queue is empty */
int isQueueEmpty(Queue* q);

/* Enqueue movie ID */
void enqueue(Queue* q, int movieId);

/* Dequeue movie ID - returns -1 if empty */
int dequeue(Queue* q);

/* Free queue memory */
void freeQueue(Queue* q);

/* =====================================================
 * FUNCTION PROTOTYPES - RECOMMENDATION ALGORITHM
 * ===================================================== */

/* 
 * Generate weighted recommendations
 * Returns count of recommendations found
 */
int recommendMoviesWeighted(
    KnowledgeGraph* kg,
    HashTable* ht,
    int baseMovieId,
    int genreWeight,
    int ratingWeight,
    int directorWeight,
    Candidate* results,
    int maxResults
);

/* Compare function for sorting candidates */
int compareCandidates(const void* a, const void* b);

/* =====================================================
 * FUNCTION PROTOTYPES - FILE I/O
 * ===================================================== */

/* Load movies from CSV file */
int loadMovies(const char* filename, HashTable* ht);

/* Print movie recommendation */
void printRecommendation(Movie* movie);

#endif /* MOVIE_H */
