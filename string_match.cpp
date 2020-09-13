#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct AutoTimer {
    clock_t begin;
    clock_t end;
    char *s_label;
    
    AutoTimer(char *label = "") {
        s_label = label;
        begin = clock();
    }
    
    ~AutoTimer() {
        release();
    }
    
    double release() {
        end = clock();
        double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        printf("%s time spent(s): %f\n", s_label, time_spent);
        return time_spent;
    }
};

struct StringRef {
    char *contents;
    size_t length;
    
    StringRef(char *s_contents) {
        contents = s_contents;
        length = strlen(contents);
    }
    
    StringRef(char *s_contents, size_t s_length) {
        contents = s_contents;
        length = s_length;
    }
};

struct StringMatch {
    StringRef text;
    StringRef match;
    
    // create lps[] that will hold the longest prefix suffix 
    // values for pattern 
    int lps[1024]; // = (int*)malloc(M*sizeof(int)); 
    
    StringMatch(char *s_match, char *s_text, size_t text_length) : text(s_text, text_length), match(s_match) {
        // Preprocess the pattern (calculate lps[] array) 
        compute_lps_array(match.contents, match.length, lps); 
        
    }
    
    StringMatch(char *s_match, char *s_text) : text(s_text), match(s_match) {
        // Preprocess the pattern (calculate lps[] array) 
        compute_lps_array(match.contents, match.length, lps); 
    }
    
    // return position that found
    size_t simple_string_match(size_t position = 0) {
        //size_t text_length = text.length - position;
        size_t found_location = -1;
        
        for (size_t i = position; i < text.length; ++i)
        {
            size_t j = 0;
            size_t first_hit = i;
            size_t tmp_i = i;
            while (tmp_i < text.length && match.contents[j++] == text.contents[tmp_i])
            {
                if (j == match.length)
                    return first_hit;
                ++tmp_i;
            }
        }
        return found_location;
    }
    
    // Fills lps[] for given patttern pat[0..M-1] 
    void compute_lps_array(char* pat, size_t M, int* lps) { 
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
            else // (pat[i] != pat[len]) 
            { 
                // This is tricky. Consider the example. 
                // AAACAAAA and i = 7. The idea is similar 
                // to search step. 
                if (len != 0) { 
                    len = lps[len - 1]; 
                    
                    // Also, note that we do not increment 
                    // i here 
                } 
                else // if (len == 0) 
                { 
                    lps[i] = 0; 
                    i++; 
                } 
            } 
        } 
    }
    
    size_t kmp_match(size_t position = 0) {
        size_t M = match.length; 
        size_t N = text.length - position; 
        char *pat = match.contents;
        char *txt = text.contents + position;
        
        size_t found_location = -1;
        
        
        int i = 0; // index for txt[] 
        int j = 0; // index for pat[] 
        while (i < N) { 
            if (pat[j] == txt[i]) { 
                j++; 
                i++; 
            } 
            
            if (j == M) { 
                // found
                found_location = i - j; 
                j = lps[j - 1]; 
                break;
            } 
            
            // mismatch after j matches 
            else if (i < N && pat[j] != txt[i]) { 
                // Do not match lps[0..lps[j-1]] characters, 
                // they will match anyway 
                if (j != 0) 
                    j = lps[j - 1]; 
                else
                    i = i + 1; 
            } 
        } 
        
        if (found_location != -1)
            return found_location + position;
        return found_location;
    }
    
};

struct FileContents {
    char *buffer;
    size_t size;
};

FileContents read_entire_file(char *filename)
{
    FileContents contents = {};
    
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  /* same as rewind(f); */
    
    void*mem = malloc(fsize);
    if (mem)
    {
        if (fread(mem, 1, fsize, f) == fsize)
        {
            contents.size = fsize;
            contents.buffer = (char*)mem;
        }
        else
        {
            free(mem);
        }
        
    }
    
    fclose(f);
    
    return contents; 
}

FileContents duplicate_content(FileContents f, int times) {
    FileContents newf;
    newf.size = f.size * times;
    newf.buffer = (char*)malloc(newf.size);
    for (int i = 0; i < times; ++i) {
        memcpy(newf.buffer + i * f.size, f.buffer, f.size);
    }
    
    return newf;
}

typedef size_t (*MatchFunc)(StringMatch*, size_t);

size_t fn_simpe_match(StringMatch *matcher, size_t loc) {
    return matcher->simple_string_match(loc);
}

size_t fn_kmp_match(StringMatch *matcher, size_t loc) {
    return matcher->kmp_match(loc);
}

void test_match(char *pattern, FileContents f, MatchFunc match_func) {
    AutoTimer timer("match");
    
    size_t match_count = 0;
    
    StringMatch matcher(pattern, f.buffer, f.size);
    size_t loc = match_func(&matcher, 0);
    while (loc != -1) {
        //printf("Found location: %zd\n", loc);
        match_count++;
        loc  = match_func(&matcher, loc + 1);
    }
    
    printf("Match count: %zd\n", match_count);
}

int main() {
    AutoTimer main_timer("all");
    
    FileContents f = read_entire_file("samples/book1");
    printf("Sample file size: %zd\n", f.size);
    
    FileContents bigf = duplicate_content(f, 100);
    printf("Sample test size: %zd\n", bigf.size);
    
    char *pattern = "he was a good man";
    
    printf("Do Simple Match\n");
    test_match(pattern, bigf, fn_simpe_match);
    
    printf("Do KMP Match\n");
    test_match(pattern, bigf, fn_kmp_match);
    
    // free resource
    free(f.buffer);
    free(bigf.buffer);
}
