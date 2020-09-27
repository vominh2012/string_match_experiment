#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <cassert>

#include "util.h"

struct StringMatchResult {
    size_t found_index;
    size_t next_search_position;
    size_t pattern_skip;
};

struct StringMatch {
    static const size_t NOT_FOUND = -1;
    
    StringRef pattern;
    StringRef text;
    
    // create lps[] that will hold the longest prefix suffix 
    // values for pattern 
    int kmp_lps[1024]; // = (int*)malloc(M*sizeof(int)); 
    size_t kmp_pattern_skip;
    size_t bmh_badchar[256]; 
    
    StringMatch(char *s_pattern, char *s_text, size_t text_length) : text(s_text, text_length), pattern(s_pattern) {
        kmp_preprocess(pattern.contents, pattern.length, kmp_lps); 
        bmh_preprocess(pattern.contents, pattern.length, bmh_badchar, array_count(bmh_badchar));
    }
    
    StringMatch(char *s_match, char *s_text) : StringMatch(s_match, s_text, strlen(s_text)) {}
    
    // memcmp is bad for perfomance because it have to  compare all char even first char not match
    StringMatchResult memcmp_match(size_t position = 0) {
        
        StringMatchResult result = {NOT_FOUND, NOT_FOUND};
        
        for (size_t i = position; i <= text.length - pattern.length; ++i)
        {
            if (memcmp(&text.contents[i], pattern.contents, pattern.length) == 0)
            {
                result.found_index = i;
                result.next_search_position = i + 1;
                break;
            }
        }
        return result;
    }
    
    // this usually run faster then kmp in optimized buid, kmp faster in case long pattern and a lot of nearly match or match
    // so this way may be better than kmp for normal matching
    StringMatchResult simple_string_match(size_t position = 0) {
        
        StringMatchResult result = {NOT_FOUND, 0, 0};
        
        for (size_t i = position; i <= text.length - pattern.length; ++i)
        {
            size_t j = 0;
            size_t first_hit = i;
            size_t tmp_i = i;
            while (tmp_i < text.length && pattern.contents[j++] == text.contents[tmp_i++])
            {
                if (j == pattern.length)
                {
                    result.found_index = first_hit;
                    result.pattern_skip = 0;
                    result.next_search_position = result.found_index + 1;
                    return result;
                }
            }
        }
        return result;
    }
    
    // Fills lps[] for given patttern pat[0..M-1] 
    void kmp_preprocess(char* pat, size_t M, int* lps) { 
        kmp_pattern_skip = 0;
        
        // length of the previous longest prefix suffix 
        int len = 0; 
        
        lps[0] = 0; // lps[0] is always 0 
        
        // the loop calculates lps[i] for i = 1 to M-1 
        int i = 1; 
        while (i < M) { 
            if (pat[i] == pat[len]) { 
                len++; 
                lps[i] = len; 
                i++; 
            } 
            else
            { 
                if (len == 0) { 
                    lps[i] = 0; 
                    i++;
                } 
                else {
                    // This is tricky.
                    len = lps[len - 1]; 
                    
                    // Also, note that we do not increment 
                    // i here 
                } 
            } 
        } 
    }
    
    StringMatchResult kmp_match(size_t position = 0) {
        size_t M = pattern.length; 
        size_t N = text.length - position; 
        char *pat = pattern.contents;
        char *txt = text.contents + position;
        
        StringMatchResult result = {NOT_FOUND, 0, 0};
        
        
        int i = 0; // index for txt[] 
        int j = 0; // index for pat[] 
        
        
        if (kmp_pattern_skip > 0) {
            j = (int)kmp_pattern_skip; // continue last search
        }
        
        while (i < N) { 
            if (pat[j] == txt[i]) { 
                j++; 
                i++; 
                if (j == M) { 
                    // found
                    result.found_index = i + position - j; 
                    j = kmp_lps[j - 1]; 
                    result.pattern_skip = j;
                    result.next_search_position = i + position;
                    break;
                }
            } 
            
            // mismatch after j matches 
            else { 
                // Do not match lps[0..lps[j-1]] characters, 
                // they will match anyway 
                if (j != 0) 
                    j = kmp_lps[j - 1]; 
                else
                    i++; 
            } 
        } 
        
        return result;
    }
    
    // The preprocessing function for Boyer Moore's 
    // bad character heuristic 
    void bmh_preprocess( char *str, size_t size,  
                        size_t *badchar, size_t badchar_len) 
    { 
        int i; 
        
        for (i = 0; i < badchar_len; i++) 
            badchar[i] = size; 
        
        for (i = 0; i < size - 1; i++) 
            badchar[str[i]] = size - 1 - i; 
    } 
    
    /* A pattern searching function that uses Bad 
       Character Heuristic of Boyer Moore Algorithm */
    StringMatchResult bmh_match(size_t position = 0)
    { 
        StringMatchResult result = {NOT_FOUND, 0, 0};
        
        size_t m = pattern.length; 
        size_t n = text.length - position; 
        char *pat = pattern.contents;
        char *txt = text.contents + position;
        
        size_t skip = 0;
        while (n - skip >= m)
        {
            char *str = txt + skip;
            if (str_cmp_backward(str, pat, m) == 0)
            {
                result.found_index = skip + position;
                result.next_search_position = result.found_index + 1;
                break;
            }
            
            skip += bmh_badchar[txt[skip + m - 1]];
        }
        
        return result;
    } 
    
};

typedef StringMatchResult (*MatchFunc)(StringMatch*, size_t);

StringMatchResult fn_simpe_match(StringMatch *matcher, size_t loc) {
    return matcher->simple_string_match(loc);
}

StringMatchResult fn_kmp_match(StringMatch *matcher, size_t loc) {
    return matcher->kmp_match(loc);
}

StringMatchResult fn_memcmp_match(StringMatch *matcher, size_t loc) {
    return matcher->memcmp_match(loc);
}

StringMatchResult fn_bmh_match(StringMatch *matcher, size_t loc) {
    return matcher->bmh_match(loc);
}

size_t test_match(char *pattern, FileContents f, MatchFunc match_func, size_t *history, size_t history_max, 
                  bool print_detail = false) {
    AutoTimer timer(print_detail ? "matching" : 0);
    
    size_t last_position = StringMatch::NOT_FOUND;
    
    size_t match_count = 0;
    
    StringMatch matcher(pattern, f.buffer, f.size);
    StringMatchResult loc = match_func(&matcher, 0);
    while (loc.found_index != StringMatch::NOT_FOUND) {
        if (match_count < history_max) {
            history[match_count] = loc.found_index;
        }
        else {
            last_position = loc.found_index;
        }
        match_count++;
        matcher.kmp_pattern_skip = loc.pattern_skip;
        loc  = match_func(&matcher, loc.next_search_position);
    }
    
    if (print_detail)
    {
        printf("matched %zd\n", match_count);
        printf("location [");
        for(size_t i = 0; i < match_count && i < history_max; ++i)
        {
            printf("%zd,", history[i]);
        }
        printf("...,%zd]\n", last_position);
    }
    
    return match_count;
}

void test_all_alg(char *pattern, char *text, size_t expected_matched)
{
	size_t text_len = strlen(text);
    
    const int ALG_COUNT = 4;
    const int MAX_HISTORY = 10;
    size_t matched_counts[ALG_COUNT] = {0};
    size_t history[ALG_COUNT][MAX_HISTORY] = {0};
    size_t count = 0;
    FileContents f = {text, text_len};
    
    matched_counts[count] = test_match(pattern, f, fn_bmh_match, history[count], MAX_HISTORY);
    matched_counts[++count] = test_match(pattern, f, fn_kmp_match, history[count], MAX_HISTORY);
    matched_counts[++count] = test_match(pattern, f, fn_simpe_match, history[count], MAX_HISTORY);
    matched_counts[++count] = test_match(pattern, f, fn_memcmp_match, history[count], MAX_HISTORY);
    
    // validate
    for (int i = 0; i < ALG_COUNT; ++i)
    {
        assert(matched_counts[i] == expected_matched);
        for (int j = 0; j < min(expected_matched, MAX_HISTORY) && (i < ALG_COUNT - 1); ++j)
        {
            assert(history[i][j] ==  history[i+1][j]);
        }
    }
}

void run_tests()
{
    test_all_alg("a", "a", 1);
    test_all_alg("a", "bcd123", 0);
    test_all_alg("a", "aaaaaaaaaa", 10);
	test_all_alg("ab", "aac", 0);
	test_all_alg("ab", "aab", 1);
	test_all_alg("aa", "aaaa", 3);
	test_all_alg("abc", "abababab", 0);
	test_all_alg("abc", "ababababc", 1);
    
	test_all_alg("ABA", "ABAAAABAACD", 2);
    
    printf("all tests is passed\n\n");
}

int main() {
    AutoTimer main_timer("all");
    
    run_tests();
    
    const int ALG_COUNT = 4;
    const int MAX_HISTORY = 20;
    size_t matched_counts[ALG_COUNT] = {0};
    size_t history[ALG_COUNT][MAX_HISTORY] = {0};
    size_t count = 0;
    
    FileContents f = read_entire_file("samples/book1");
    printf("Sample file size: %zd\n", f.size);
    
    FileContents bigf = duplicate_content(f, 100);
    printf("Sample test size: %zd\n", bigf.size);
    
    char *pattern = "h";
    
    printf("\nSimple ");
    matched_counts[count] = test_match(pattern, bigf, fn_simpe_match, history[count], MAX_HISTORY, true);
    
    printf("\nKMP ");
    matched_counts[++count] = test_match(pattern, bigf, fn_kmp_match, history[count], MAX_HISTORY, true);
    
    printf("\nBoyer Moore Horspool ");
    matched_counts[++count] = test_match(pattern, bigf, fn_bmh_match, history[count], MAX_HISTORY, true);
    
    printf("\nmemcmp ");
    matched_counts[++count] = test_match(pattern, bigf, fn_memcmp_match, history[count], MAX_HISTORY, true);
    
    // free resources in case you have to do it
    // free(f.buffer);
    // free(bigf.buffer);
}
