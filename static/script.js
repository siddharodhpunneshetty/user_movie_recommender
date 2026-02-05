/**
 * script.js - Movie Recommender Frontend Logic
 * 
 * Handles:
 * - Debounced autocomplete search
 * - Weight slider updates
 * - API calls to Flask backend
 * - Dynamic recommendation display
 */

// =====================================================
// DOM ELEMENTS
// =====================================================

const searchInput = document.getElementById('searchInput');
const autocompleteResults = document.getElementById('autocompleteResults');
const selectedMovieSection = document.getElementById('selectedMovieSection');
const selectedMovieTitle = document.getElementById('selectedMovieTitle');
const selectedMovieGenre = document.getElementById('selectedMovieGenre');
const selectedMovieRating = document.getElementById('selectedMovieRating');
const selectedMovieDirector = document.getElementById('selectedMovieDirector');
const clearSelectionBtn = document.getElementById('clearSelection');
const weightsSection = document.getElementById('weightsSection');
const genreWeight = document.getElementById('genreWeight');
const ratingWeight = document.getElementById('ratingWeight');
const directorWeight = document.getElementById('directorWeight');
const genreWeightValue = document.getElementById('genreWeightValue');
const ratingWeightValue = document.getElementById('ratingWeightValue');
const directorWeightValue = document.getElementById('directorWeightValue');
const generateBtn = document.getElementById('generateBtn');
const recommendationsSection = document.getElementById('recommendationsSection');
const recommendationsGrid = document.getElementById('recommendationsGrid');
const errorMessage = document.getElementById('errorMessage');

// =====================================================
// STATE
// =====================================================

let selectedMovie = null;
let debounceTimer = null;

// =====================================================
// UTILITY FUNCTIONS
// =====================================================

/**
 * Debounce function to limit API calls
 */
function debounce(func, delay) {
    return function (...args) {
        clearTimeout(debounceTimer);
        debounceTimer = setTimeout(() => func.apply(this, args), delay);
    };
}

/**
 * Show error message
 */
function showError(message) {
    errorMessage.textContent = message;
    errorMessage.style.display = 'block';
    setTimeout(() => {
        errorMessage.style.display = 'none';
    }, 5000);
}

/**
 * Hide error message
 */
function hideError() {
    errorMessage.style.display = 'none';
}

/**
 * Get genre emoji based on genre name
 */
function getGenreEmoji(genre) {
    const emojiMap = {
        'action': '💥',
        'adventure': '🗺️',
        'animation': '🎨',
        'comedy': '😂',
        'crime': '🔫',
        'drama': '🎭',
        'fantasy': '🧙',
        'horror': '👻',
        'romance': '💕',
        'sci-fi': '🚀',
        'thriller': '😱',
        'war': '⚔️'
    };
    return emojiMap[genre.toLowerCase()] || '🎬';
}

// =====================================================
// SEARCH & AUTOCOMPLETE
// =====================================================

/**
 * Search for movies by name
 */
async function searchMovies(query) {
    if (!query || query.length < 2) {
        autocompleteResults.classList.remove('active');
        return;
    }

    try {
        const response = await fetch(`/search?name=${encodeURIComponent(query)}`);
        const movies = await response.json();

        if (movies.length === 0) {
            autocompleteResults.classList.remove('active');
            return;
        }

        // Render autocomplete results
        autocompleteResults.innerHTML = movies.map(movie => `
            <div class="autocomplete-item" data-movie='${JSON.stringify(movie)}'>
                <div class="autocomplete-title">${movie.title}</div>
                <div class="autocomplete-meta">
                    <span>🎭 ${movie.genre}</span>
                    <span>⭐ ${movie.rating}</span>
                    <span>🎬 ${movie.director}</span>
                </div>
            </div>
        `).join('');

        autocompleteResults.classList.add('active');
    } catch (error) {
        console.error('Search error:', error);
        showError('Failed to search movies. Please try again.');
    }
}

// Debounced search
const debouncedSearch = debounce(searchMovies, 300);

// =====================================================
// MOVIE SELECTION
// =====================================================

/**
 * Select a movie from autocomplete
 */
function selectMovie(movie) {
    selectedMovie = movie;

    // Update UI
    selectedMovieTitle.textContent = movie.title;
    selectedMovieGenre.textContent = `🎭 ${movie.genre}`;
    selectedMovieRating.textContent = `⭐ ${movie.rating}`;
    selectedMovieDirector.textContent = `🎬 ${movie.director}`;

    // Show sections
    selectedMovieSection.style.display = 'block';
    weightsSection.style.display = 'block';

    // Hide autocomplete and clear search
    autocompleteResults.classList.remove('active');
    searchInput.value = '';

    // Hide recommendations if showing
    recommendationsSection.style.display = 'none';

    hideError();
}

/**
 * Clear movie selection
 */
function clearSelection() {
    selectedMovie = null;
    selectedMovieSection.style.display = 'none';
    weightsSection.style.display = 'none';
    recommendationsSection.style.display = 'none';
    searchInput.focus();
}

// =====================================================
// WEIGHT SLIDERS
// =====================================================

/**
 * Update weight display value
 */
function updateWeightDisplay(slider, display) {
    display.textContent = slider.value;
}

// =====================================================
// RECOMMENDATIONS
// =====================================================

/**
 * Generate recommendations
 */
async function generateRecommendations() {
    if (!selectedMovie) {
        showError('Please select a movie first.');
        return;
    }

    const btnText = generateBtn.querySelector('.btn-text');
    const btnLoader = generateBtn.querySelector('.btn-loader');

    // Show loading state
    generateBtn.disabled = true;
    btnText.textContent = 'Generating...';
    btnLoader.style.display = 'inline-block';

    try {
        const response = await fetch('/recommend', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify({
                movie_name: selectedMovie.title,
                genre_weight: parseInt(genreWeight.value),
                rating_weight: parseInt(ratingWeight.value),
                director_weight: parseInt(directorWeight.value)
            })
        });

        const data = await response.json();

        if (!response.ok) {
            throw new Error(data.error || 'Failed to get recommendations');
        }

        displayRecommendations(data.recommendations);
        hideError();

    } catch (error) {
        console.error('Recommendation error:', error);
        showError(error.message || 'Failed to generate recommendations. Please try again.');
    } finally {
        // Reset button
        generateBtn.disabled = false;
        btnText.textContent = 'Generate Recommendations';
        btnLoader.style.display = 'none';
    }
}

/**
 * Display recommendations in grid
 */
function displayRecommendations(recommendations) {
    if (!recommendations || recommendations.length === 0) {
        recommendationsGrid.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; padding: 2rem; color: var(--text-secondary);">
                No recommendations found. Try adjusting your weights or selecting a different movie.
            </div>
        `;
        recommendationsSection.style.display = 'block';
        return;
    }

    recommendationsGrid.innerHTML = recommendations.map(movie => `
        <div class="movie-card">
            <div class="movie-card-poster">
                ${getGenreEmoji(movie.genre)}
            </div>
            <div class="movie-card-content">
                <h3 class="movie-card-title">${movie.title}</h3>
                <div class="movie-card-meta">
                    <span class="badge genre-badge">${movie.genre}</span>
                    <span class="badge rating-badge">⭐ ${movie.rating}</span>
                </div>
                <div class="movie-card-director">${movie.director}</div>
            </div>
        </div>
    `).join('');

    recommendationsSection.style.display = 'block';

    // Scroll to recommendations
    recommendationsSection.scrollIntoView({ behavior: 'smooth', block: 'start' });
}

// =====================================================
// EVENT LISTENERS
// =====================================================

// Search input
searchInput.addEventListener('input', (e) => {
    debouncedSearch(e.target.value.trim());
});

// Close autocomplete on outside click
document.addEventListener('click', (e) => {
    if (!e.target.closest('.search-wrapper')) {
        autocompleteResults.classList.remove('active');
    }
});

// Autocomplete item selection
autocompleteResults.addEventListener('click', (e) => {
    const item = e.target.closest('.autocomplete-item');
    if (item) {
        const movie = JSON.parse(item.dataset.movie);
        selectMovie(movie);
    }
});

// Clear selection
clearSelectionBtn.addEventListener('click', clearSelection);

// Weight sliders
genreWeight.addEventListener('input', () => updateWeightDisplay(genreWeight, genreWeightValue));
ratingWeight.addEventListener('input', () => updateWeightDisplay(ratingWeight, ratingWeightValue));
directorWeight.addEventListener('input', () => updateWeightDisplay(directorWeight, directorWeightValue));

// Generate button
generateBtn.addEventListener('click', generateRecommendations);

// Keyboard shortcuts
searchInput.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') {
        autocompleteResults.classList.remove('active');
    }
});

// =====================================================
// INITIALIZATION
// =====================================================

// Focus search input on load
window.addEventListener('load', () => {
    searchInput.focus();
});
