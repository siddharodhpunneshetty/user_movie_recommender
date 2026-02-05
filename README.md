# Movie Recommender System

A localhost Movie Recommender System using **C data structures** (hash table, knowledge graph), **Python Flask** backend, and **HTML/CSS/JavaScript** frontend with user-defined similarity weights.

## Features

- **Hash Table**: O(1) movie lookup with separate chaining collision handling
- **Knowledge Graph**: Movies connected by genre, rating, and director similarity
- **Weighted Algorithm**: User-selectable weights (0-10) for each similarity type
- **Autocomplete Search**: Search movies by name with instant suggestions
- **Netflix-inspired UI**: Dark theme with modern, clean design

## Project Structure

```
movie_recommender/
├── recommender.c      # C core engine (hash table + knowledge graph)
├── movie.h            # Data structure definitions
├── movies.txt         # Dataset (150 movies)
├── app.py             # Flask backend server
├── static/
│   ├── index.html     # Main web page
│   ├── style.css      # Netflix-dark theme
│   └── script.js      # Frontend logic
└── README.md          # This file
```

## Prerequisites

- **GCC Compiler** (for C code)
- **Python 3.7+** with Flask

## Setup Instructions

### 1. Compile the C Recommender Engine

Open PowerShell/Command Prompt in the project folder:

```powershell
cd c:\user_movie_recommender
gcc -o recommender.exe recommender.c -lm
```

### 2. Install Flask

```powershell
pip install flask
```

### 3. Run the Flask Server

```powershell
python app.py
```

### 4. Open in Browser

Navigate to: **http://127.0.0.1:5000**

## Usage Guide

1. **Search for a Movie**: Type a movie name in the search bar
2. **Select a Movie**: Click on a movie from the dropdown
3. **Adjust Weights**: Use sliders to set importance (0-10):
   - **Genre Similarity**: Higher = more same-genre movies
   - **Rating Similarity**: Higher = more similarly-rated movies
   - **Director Similarity**: Higher = more movies by same director
4. **Generate Recommendations**: Click the button to get 20 recommended movies

## API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Serve main page |
| `/search?name=<query>` | GET | Search movies by name |
| `/recommend` | POST | Get recommendations (JSON body) |

### Recommend Request Body

```json
{
  "movie_name": "The Matrix",
  "genre_weight": 5,
  "rating_weight": 3,
  "director_weight": 7
}
```

## Algorithm Details

### Knowledge Graph Construction

For each pair of movies:
- **GENRE_SIMILAR** edge: If genres match exactly
- **RATING_SIMILAR** edge: If rating difference ≤ 0.5
- **DIRECTOR_SIMILAR** edge: If directors match exactly

### Weighted Scoring

```
score = (has_genre_edge * genre_weight) + 
        (has_rating_edge * rating_weight) + 
        (has_director_edge * director_weight)
```

Results sorted by:
1. Total score (descending)
2. Movie rating (descending, tie-breaker)

## Testing the C Program Directly

```powershell
.\recommender.exe <movie_id> <genre_weight> <rating_weight> <director_weight>

# Example: Get recommendations for The Matrix (ID=1) with weights 5,3,7
.\recommender.exe 1 5 3 7
```

## License

Educational project for learning data structures and web development.
