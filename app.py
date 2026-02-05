"""
app.py - Flask Backend for Movie Recommender System

Provides REST API endpoints for:
- /search: Autocomplete search for movies by name
- /recommend: Generate weighted recommendations using C engine

Usage: python app.py
Server runs on http://127.0.0.1:5000
"""

from flask import Flask, request, jsonify, send_from_directory
import subprocess
import os
import csv

app = Flask(__name__, static_folder='static')

# =====================================================
# MOVIE DATA LOADING
# =====================================================

# In-memory movie storage for fast search
movies = []
movie_by_id = {}
movie_by_name = {}

def load_movies():
    """Load movies from CSV file into memory for fast search"""
    global movies, movie_by_id, movie_by_name
    
    movies_file = os.path.join(os.path.dirname(__file__), 'movies.txt')
    
    try:
        with open(movies_file, 'r', encoding='utf-8') as f:
            reader = csv.DictReader(f)
            for row in reader:
                movie = {
                    'id': int(row['id']),
                    'title': row['title'],
                    'genre': row['genre'],
                    'rating': float(row['rating']),
                    'director': row['director']
                }
                movies.append(movie)
                movie_by_id[movie['id']] = movie
                # Store lowercase for case-insensitive search
                movie_by_name[movie['title'].lower()] = movie
        
        print(f"Loaded {len(movies)} movies from database")
    except Exception as e:
        print(f"Error loading movies: {e}")

# =====================================================
# STATIC FILE SERVING
# =====================================================

@app.route('/')
def index():
    """Serve the main HTML page"""
    return send_from_directory('static', 'index.html')

@app.route('/static/<path:filename>')
def serve_static(filename):
    """Serve static files (CSS, JS)"""
    return send_from_directory('static', filename)

# =====================================================
# API ENDPOINTS
# =====================================================

@app.route('/search', methods=['GET'])
def search():
    """
    Search for movies by name (autocomplete)
    
    Query Parameters:
        name (str): Search query string
    
    Returns:
        JSON array of matching movies (max 10)
    """
    query = request.args.get('name', '').lower().strip()
    
    if not query:
        return jsonify([])
    
    # Find movies that contain the search query
    results = []
    for movie in movies:
        if query in movie['title'].lower():
            results.append({
                'id': movie['id'],
                'title': movie['title'],
                'genre': movie['genre'],
                'rating': movie['rating'],
                'director': movie['director']
            })
            if len(results) >= 10:
                break
    
    return jsonify(results)

@app.route('/recommend', methods=['POST'])
def recommend():
    """
    Generate movie recommendations using C engine
    
    Request Body (JSON):
        movie_name (str): Name of base movie
        genre_weight (int): Weight for genre similarity (0-10)
        rating_weight (int): Weight for rating similarity (0-10)
        director_weight (int): Weight for director similarity (0-10)
    
    Returns:
        JSON object with recommendations array
    """
    try:
        data = request.get_json()
        
        if not data:
            return jsonify({'error': 'No JSON data provided'}), 400
        
        movie_name = data.get('movie_name', '').strip()
        genre_weight = int(data.get('genre_weight', 5))
        rating_weight = int(data.get('rating_weight', 5))
        director_weight = int(data.get('director_weight', 5))
        
        # Validate weights
        for weight, name in [(genre_weight, 'genre'), (rating_weight, 'rating'), (director_weight, 'director')]:
            if weight < 0 or weight > 10:
                return jsonify({'error': f'{name}_weight must be between 0 and 10'}), 400
        
        # Find movie by name (case-insensitive)
        movie = movie_by_name.get(movie_name.lower())
        
        if not movie:
            # Try partial match
            for m in movies:
                if movie_name.lower() in m['title'].lower():
                    movie = m
                    break
        
        if not movie:
            return jsonify({'error': f'Movie "{movie_name}" not found'}), 404
        
        movie_id = movie['id']
        
        # Call C recommender program
        recommender_path = os.path.join(os.path.dirname(__file__), 'recommender.exe')
        
        if not os.path.exists(recommender_path):
            return jsonify({'error': 'Recommender engine not compiled. Please compile recommender.c first.'}), 500
        
        try:
            result = subprocess.run(
                [recommender_path, str(movie_id), str(genre_weight), str(rating_weight), str(director_weight)],
                capture_output=True,
                text=True,
                timeout=10,
                cwd=os.path.dirname(__file__)
            )
            
            if result.returncode != 0:
                return jsonify({'error': f'Recommender error: {result.stderr}'}), 500
            
            # Parse CSV output from C program
            recommendations = []
            for line in result.stdout.strip().split('\n'):
                if not line:
                    continue
                parts = line.split(',')
                if len(parts) >= 5:
                    rec = {
                        'id': int(parts[0]),
                        'title': parts[1],
                        'genre': parts[2],
                        'rating': float(parts[3]),
                        'director': parts[4]
                    }
                    recommendations.append(rec)
            
            return jsonify({
                'base_movie': {
                    'id': movie['id'],
                    'title': movie['title'],
                    'genre': movie['genre'],
                    'rating': movie['rating'],
                    'director': movie['director']
                },
                'recommendations': recommendations,
                'weights': {
                    'genre': genre_weight,
                    'rating': rating_weight,
                    'director': director_weight
                }
            })
            
        except subprocess.TimeoutExpired:
            return jsonify({'error': 'Recommendation engine timed out'}), 500
        except Exception as e:
            return jsonify({'error': f'Error running recommender: {str(e)}'}), 500
            
    except Exception as e:
        return jsonify({'error': f'Server error: {str(e)}'}), 500

@app.route('/movies', methods=['GET'])
def get_all_movies():
    """
    Get all movies (for debugging/testing)
    
    Returns:
        JSON array of all movies
    """
    return jsonify(movies)

# =====================================================
# MAIN ENTRY POINT
# =====================================================

if __name__ == '__main__':
    # Load movies on startup
    load_movies()
    
    print("\n" + "="*50)
    print("Movie Recommender System")
    print("="*50)
    print("Server starting on http://127.0.0.1:5000")
    print("Press Ctrl+C to stop")
    print("="*50 + "\n")
    
    # Run Flask development server
    app.run(host='127.0.0.1', port=5000, debug=True)
