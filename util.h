/* date = September 27th 2020 7:27 pm */

#ifndef UTIL_H
#define UTIL_H

#define max(a, b) ( ((a) > (b)) ? (a) : (b) )
#define min(a, b) ( ((a) < (b)) ? (a) : (b) )
#define array_count(arr) (sizeof(arr) / sizeof(arr[0]))

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
        if (s_label)
            printf("%s time spent(s): %f\n", s_label, time_spent);
        s_label = 0;
        return time_spent;
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

size_t str_cmp_backward(char *str1, char *str2, size_t len)
{
    size_t i = len - 1;
    while (str1[i] == str2[i])
    {
        if (i == 0) return 0;
        i--;
    }
    
    return 1;
}

size_t str_cmp_raita(char *str1, char *str2, size_t len)
{
    size_t i = 0;
    size_t mid = len / 2;
    
    if (len >= 3) {
        if (str1[mid] != str2[mid]) {
            return 1;
        }
    }
    
    if (len >= 1) {
        if (str1[0] != str2[0]) {
            return 1;
        }
    }
    
    if (len >= 2) {
        if (str1[len - 1] != str2[len - 1]) {
            return 1;
        }
    }
    
    if (len < 3)
        return 0;
    
    return str_cmp_backward(&str1[1], &str2[1], len - 2);
}


#endif //UTIL_H
