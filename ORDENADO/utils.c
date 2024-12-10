#include "minishell.h"

char    *ft_strndup(const char *str, size_t len)
{
    char    *dup;
    size_t  s_len;
    size_t  i;

    if (!str)
        return (NULL);
    s_len = ft_strlen(str);
    if (s_len <= len)
        len = s_len;
    dup = malloc(sizeof(char) * (len + 1));
    if (!dup)
        return (NULL);
    i = 0;
    while (i < len)
    {
        dup[i] = str[i];
        i++;
    }
    dup[i] = '\0';
    return (dup);
}

extern inline int is_space(char c)
{
    return (c == ' ' || (c >= 9 && c <= 13));
}

extern inline int is_special_char(char c)
{
    return (c == '<' || c == '>' || c == '|' || c == '"' ||
         c == '\'' || c == ' ' || (c >= 9 && c <= 13));
}

void skip_spaces(const char *line, int *i, int *spaces)
{
    *spaces = 0;
    while (line[*i] && is_space(line[*i]))
    {
        (*spaces)++;
        (*i)++;
    }
}