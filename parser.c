//readline --> compilar con -lreadline
# include <stdio.h>
# include <readline/readline.h>
# include <readline/history.h>
//------------------
# include <stdlib.h> //malloc

typedef enum    s_type
{
    WORD,
    QUOTED,
    OPERATOR
}   t_type;

typedef enum    s_operator
{
    INPUT,
    OUTPUT,
    HEREDOC,
    APPEND,
    PIPE
}   t_operator;

typedef struct  s_token t_token;

struct s_token
{
    char    *value;
    short   is_quote;
    short   expand;
    t_type  type;
    t_token *next;
};

//-------------------LIBFT
size_t  ft_strlen(const char *str)
{
    size_t  i;

    i = 0;
    while (str[i])
        i++;
    return (i);
}

char	*ft_strchr(const char *s, int c)
{
	char	char_c;

	char_c = (char)c;
	while (*s)
	{
		if (*s == char_c)
		{
			return ((char *)s);
		}
		s++;
	}
	if (char_c == 0)
	{
		return ((char *)s);
	}
	return (NULL);
}

char	*ft_strdup(const char *str)
{
	int		str_len;
	char	*dest;
	int		i;

	str_len = ft_strlen(str);
	dest = (char *)malloc(sizeof(char) * (str_len + 1));
	i = 0;
	if (!dest)
		return (NULL);
	while (str[i])
	{
		dest[i] = str[i];
		i++;
	}
	dest[i] = 0;
	return (dest);
}
//-------------------LIBFT

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

t_token *init_token(t_type type)
{
    t_token *new_tkn;

    new_tkn = malloc(sizeof(t_token));
    if (!new_tkn)
    {
        perror("token malloc error\n");
        return (NULL);
    }
    new_tkn->value = NULL;
    new_tkn->is_quote = 0;
    new_tkn->expand = 0;
    new_tkn->type = type;
    new_tkn->next = NULL;
    return(new_tkn);
}

void    add_token(t_token **tkn_lst, t_token *token)
{
    t_token *curr_tkn;

    if (!*tkn_lst)
        *tkn_lst = token;
    else
    {
        curr_tkn = *tkn_lst;
        while (curr_tkn->next)
            curr_tkn = curr_tkn->next;
        curr_tkn->next = token;
    }
}

void    free_tkn_lst(t_token *tkn_lst)
{
    t_token *next;

    while(tkn_lst)
    {
        next = tkn_lst->next;
        free(tkn_lst->value);
        free(tkn_lst);
        tkn_lst = next;
    }
}

t_token    *parse_quote(char *line, char quote, int start, int end)
{
    char    *str;
    t_token *tkn;

    str = ft_strndup (&line[start + 1], end - start - 1);
    if (!str)
    {
        perror("strndup token error\n");
        return (NULL);
    }
    tkn = init_token(QUOTED);
    tkn->value = str;
    tkn->is_quote = quote;
    tkn->expand = (quote == '"' && ft_strchr(str, '$') != NULL);
    return (tkn);
}

t_token   *manage_quote(char *line, int *i, int start, t_token **tkn_lst)
{
    char    quote;
    int     end;
    t_token *token;

    quote = line[(*i)++];
    while (line[*i] && line[*i] != quote)
        (*i)++;
    if (!line[*i])
    {
        perror("unclosed quote error\n");
        free_tkn_lst(*tkn_lst);
        return (NULL);
    }
    end = (*i)++;
    token = parse_quote(line, quote, start, end);
    add_token(tkn_lst, token);
    return(*tkn_lst);
}

t_token *parse_operator(t_operator operator)
{
    char    *name;
    t_token *tkn;

    char    *operator_name[]={"INPUT", "OUTPUT", "HEREDOC", "APPEND", "PIPE"};
    tkn = init_token(OPERATOR);
    name = ft_strdup(operator_name[operator]);
    if (!name)
    {
        perror("strdup token error\n");
        free(tkn);
        return (NULL);
    }
    tkn->value = name;
    return (tkn);
}

t_token *manage_operator(char *line, int *i, t_token **tkn_lst)
{
    t_token *token;

    if (line[*i] == '>' && line[(*i) + 1] == '>')
    {
        token = parse_operator(APPEND);
        (*i) += 2;
    }
    else if (line[*i] == '<' && line[(*i) + 1] == '<')
    {
        token = parse_operator(HEREDOC);
        (*i) += 2;
    }
    else if (line[*i] == '>')
    {
        token = parse_operator(OUTPUT);
        (*i)++;
    }
    else if (line[*i] == '<')
    {
        token = parse_operator(INPUT);
        (*i)++;
    }
    else if (line[*i] == '|')
    {
        token = parse_operator(PIPE);
        (*i)++;
    }
    add_token(tkn_lst, token);
    return (*tkn_lst);
}

t_token *manage_word(char *line, int *i, t_token **tkn_lst)
{
    int start;
    int end;
    char    *word;
    t_token *tkn;

    start = (*i);
    while(line[*i] && !is_special_char(line[*i]))
        (*i)++;
    end = (*i);
    tkn = init_token(WORD);
    word = ft_strndup(&line[start], end - start);
    if (!word)
    {
        perror("strndup token error\n");
        free(tkn);
        return (NULL);
    }
    tkn->value = word;
    add_token(tkn_lst, tkn);
    return(*tkn_lst);
}

t_token *tokenize(char *line)
{
    t_token *tkn_lst;
    t_token    *token;
    int     i;
    int    start;

    tkn_lst = NULL;
    i = 0;
    while (line[i])
    {
        while(line[i] && is_space(line[i]))
                i++;
        start = i;
        if (line[i] && !is_space(line[i]))
        {
            if (line[i] == '"' || line[i] == '\'')
            {
                if(!manage_quote(line, &i, start, &tkn_lst))
                    return (NULL);
            }
            else if (line[i] == '<' || line[i] == '>' || line[i] == '|')
            {
                if (!manage_operator(line, &i, &tkn_lst))
                    return (NULL);
            }
            else if (!is_special_char(line[i]))
            {
                if (!manage_word(line, &i, &tkn_lst))
                    return (NULL);
            }
            else
                i++;
        }
    }
    return (tkn_lst);
}

int main(void)
{
    char    *line;
    t_token *tkn_lst;
    t_token *curr_tkn;

    while (1)
    {
        line = readline("minishell> ");
        if (line)
        {
            add_history(line);
            tkn_lst = tokenize(line);
            if (tkn_lst)
            {
                curr_tkn = tkn_lst;
                while(curr_tkn)
                {
                    printf("Token value: *%s*\n", curr_tkn->value);
                    printf("is quote? --> %d\n", curr_tkn->is_quote);
                    printf("expands? --> %d\n", curr_tkn->expand);
                    printf("type: %u\n", curr_tkn->type);
                    curr_tkn = curr_tkn->next;
                }
            }
            free_tkn_lst(tkn_lst);
        }
        free(line);
    }
}