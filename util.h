
#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

int hex_digit_to_int(char c);
char *str_duplicate(const char *str);
char *str_to_upper(char *str); 
// char *substr(const char *source, int start, int end);

#endif // util_h_INCLUDED

#ifdef IMPLEMENT_UTIL

int hex_digit_to_int(char c)
{
    if((c >= '0' && c <= '9')) return c - '0';
    else if((c >= 'A' && c <= 'F')) return c - 'A' + 10;
    else if((c >= 'a' && c <= 'f')) return c - 'a' + 10;
    else return -1;
}

char *str_duplicate(const char *str)
{
    size_t len = strlen(str);
    char *dulicated_str = malloc(len + 1);

    if (!dulicated_str)
        return NULL;

    memcpy(dulicated_str, str, len);
    dulicated_str[len] = '\0';

    return dulicated_str;
}

char *str_to_upper(char *str)
{
    char* temp = str;
    while(*temp)
    {
        if(*temp>='a' && *temp<='z')
        {
            *temp = *temp - 32;
        }
        temp++;
    }
   return str; 
} 
/*
char *substr(const char *source, int start, int end)
{
    printf("%d %s\n",__LINE__, __FILE__);
    if (start < 0 || start > end || !source){
        return NULL;
    }

    int substr_size = end - start;
    char *result = calloc(substr_size + 1, 1);
    if (!result){
        fprintf(stderr,"Error: Unable to allocate memory\n");
        exit(1);
    }
    strncpy(result, &source[start], substr_size);
    result[substr_size] = '\0';
    return result;
}
*/
#endif // IMPLEMENT_UTIL
