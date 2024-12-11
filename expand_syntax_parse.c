//readline --> compilar con -lreadline
# include <stdio.h>
# include <readline/readline.h>
# include <readline/history.h>
//------------------
# include <stdlib.h> //malloc
# include <unistd.h> // write

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
    int     has_space;
    t_type  type;
    t_token *next;
    t_token *prev;
};

typedef struct  s_args
{
    char    *line;
    int     *i;
    t_token **tkn_lst;
    int     spaces;
}   t_args;

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

void	ft_putstr_fd(char *s, int fd)
{
	if (s)
		write (fd, s, ft_strlen(s));
}

int	ft_strncmp(const char *s1, const char *s2, size_t n)
{
	size_t	i;

	if (n == 0)
		return (0);
	i = 0;
	while (s1[i] && s1[i] == s2[i] && i < n - 1)
		i++;
	return ((unsigned char)s1[i] - (unsigned char)s2[i]);
}

int	ft_isalpha(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
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

//-------------------ENV
typedef struct s_env
{
    char    *key;
    char    *value;
    struct s_env *next;
}   t_env;


void    free_env_list(t_env *env_list)
{
    t_env   *tmp;
    while (env_list)
    {
        tmp = env_list;
        env_list = env_list->next;
        free(tmp->key);
        free(tmp->value);
        free(tmp);
    }
}

void    add_env_variable(t_env **env_list, char *key, char *value)
{
    t_env   *new_node;
    
    if (!(new_node = malloc(sizeof(t_env))))
    {
        free_env_list(*env_list);
        perror("malloc error creating env node\n");
        return ;
    }
    if (!(new_node->key = ft_strdup(key)))
    {
        free(new_node);
        perror("malloc error for env key\n");
        return ;
    }
    if (!(new_node->value = ft_strdup(value)))
    {
        free(new_node->key);
        free(new_node);
        perror("malloc error for env value\n");
        return ;
    }
    new_node->next = *env_list;
    *env_list = new_node;
}

t_env *init_env_list(char **env)
{
    t_env   *env_list = NULL;
    char    *key;
    char    *value;
    char    *separator;

    while(*env)
    {
        separator = ft_strchr(*env, '=');
        if (separator)
        {
            if (!(key = ft_strndup(*env, separator - *env)))
            {
                perror("malloc error in env key duplication\n");
                free_env_list(env_list);
                return (NULL);
            }
            if(!(value = ft_strdup(separator + 1)))
            {
                perror("malloc error in env value duplication\n");
                free(key);
                free_env_list(env_list);
                return (NULL);
            }
            add_env_variable(&env_list, key, value);
            free(key);
            free(value);
        }
        env++;
    }
    return (env_list);
}

void    print_env_list(t_env *env_list)
{
    while(env_list)
    {
        printf("%s=%s\n", env_list->key, env_list->value);
        env_list = env_list->next;
    }
}

char    *get_env_value(char *str, t_env *env_list)
{
    // if (str[0] == '$')
    //     str++;
    while (env_list)
    {
        if (ft_strncmp(str, env_list->key, ft_strlen(str)) == 0)  
            return (env_list->value);
        env_list = env_list->next;
    }
    return (NULL);
}
//-------------------ENV

t_args  *pass_args(char *line, int *i, t_token **tkn_lst, int spaces)
{
    t_args  *args;

    args = malloc(sizeof(t_args));
    if (!args)
    {
        perror("args malloc error\n");
        return (NULL);
    }
    args->line = line;
    args->i = i;
    args->tkn_lst = tkn_lst;
    args->spaces = spaces;
    return(args);
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
    new_tkn->prev = NULL;
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
        token->prev = curr_tkn;
    }
}

void    free_tkn_lst(t_token *tkn_lst)
{
    t_token *next;

    while(tkn_lst)
    {
        next = tkn_lst->next;
        if (tkn_lst->value)
            free(tkn_lst->value);
        free(tkn_lst);
        tkn_lst = next;
    }
}

t_token    *parse_quote(t_args *args, char quote, int start, int end)
{
    char    *str;
    t_token *tkn;

    str = ft_strndup (&(args->line[start + 1]), end - start - 1);
    if (!str)
    {
        perror("strndup token error\n");
        return (NULL);
    }
    tkn = init_token(QUOTED);
    tkn->value = str;
    tkn->is_quote = quote;
    tkn->has_space = args->spaces;
    tkn->expand = (quote == '"' && ft_strchr(str, '$') != NULL);
    return (tkn);
}

t_token   *manage_quote(t_args *args, int start)
{
    char    quote;
    int     end;
    t_token *token;

    quote = args->line[(*args->i)++];
    while (args->line[*args->i] && args->line[*args->i] != quote)
        (*args->i)++;
    if (!args->line[*args->i])
    {
        perror("unclosed quote error\n");
        free_tkn_lst(*args->tkn_lst);
        return (NULL);
    }
    end = (*args->i)++;
    token = parse_quote(args, quote, start, end);
    add_token(args->tkn_lst, token);
    return(*(args->tkn_lst));
}

t_token *parse_operator(t_operator operator, int spaces)
{
    char    *name;
    t_token *tkn;

    char    *operator_name[]={"<", ">", "<<", ">>", "|"};
    tkn = init_token(OPERATOR);
    name = ft_strdup(operator_name[operator]);
    if (!name)
    {
        perror("strdup token error\n");
        free(tkn);
        return (NULL);
    }
    tkn->value = name;
    tkn->has_space = spaces;
    return (tkn);
}

t_operator get_operator_type(const char *line, int i)
{
    if (line[i] == '>' && line[i + 1] == '>')
        return (APPEND);
    if (line[i] == '<' && line[i + 1] == '<')
        return (HEREDOC);
    if (line[i] == '>')
        return (OUTPUT);
    if (line[i] == '<')
        return (INPUT);
    return (PIPE);
}

int create_operator_token(t_args *args, t_operator operator, int increment)
{
    t_token *token = parse_operator(operator, args->spaces);
    if (!token)
        return (0);
    *(args->i) += increment;
    add_token(args->tkn_lst, token);
    return (1);
}

t_token *manage_operator(t_args *args)
{
    int increment;

    t_operator operator = get_operator_type(args->line, *(args->i));
    if (operator == APPEND || operator == HEREDOC)
        increment = 2;
    else
        increment = 1;
    if (!create_operator_token(args, operator, increment))
        return (NULL);
    return (*(args->tkn_lst));
}

t_token *manage_word(t_args *args)
{
    int start;
    int end;
    int expand_value;
    char    *word;
    t_token *tkn;

    expand_value = 0;
    start = (*args->i);
    while(args->line[*args->i] && !is_special_char(args->line[*args->i]))
    {
        if (args->line[*args->i] == '$')
            expand_value = 1;
        (*args->i)++;
    }
    end = (*args->i);
    tkn = init_token(WORD);
    word = ft_strndup(&(args->line[start]), end - start);
    if (!word)
    {
        perror("strndup token error\n");
        free(tkn);
        return (NULL);
    }
    tkn->value = word;
    tkn->has_space = args->spaces;
    tkn->expand = expand_value;
    add_token(args->tkn_lst, tkn);
    return(*(args->tkn_lst));
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

int process_token(t_args *args)
{
    int start = *(args->i);
    
    if (args->line[*(args->i)] == '"' || args->line[*(args->i)] == '\'')
        return (manage_quote(args, start) != NULL);
    if (args->line[*(args->i)] == '<' || args->line[*(args->i)] == '>'
        || args->line[*(args->i)] == '|')
        return (manage_operator(args) != NULL);
    if (!is_special_char(args->line[*(args->i)]))
        return (manage_word(args) != NULL);
    (*(args->i))++;
    return (1);
}

t_token *tokenize(char *line)
{
    t_token *tkn_lst;
    int i;
    int spaces;
    t_args *args;

    i = 0;
    tkn_lst = NULL;
    while (line[i])
    {
        skip_spaces(line, &i, &spaces);
        if (line[i] && !is_space(line[i]))
        {
            args = pass_args(line, &i, &tkn_lst, spaces);
            if (!args)
                return (NULL);
            if (!process_token(args))
            {
                free(args);
                return (NULL);
            }
            free(args);
        }
    }
    return (tkn_lst);
}
//------------------------SYNTAX CHECK
int syntax_check(t_token *tkn_lst)
{
    t_token *current;
    
    current = tkn_lst;
    while (current)
    {
        if (current->type == 2)
        {
            if (current->value[0] == '|')
            {
                if (!current->next || current == tkn_lst || current->next->type == OPERATOR) // Pipe al inicio o sin comando después
                    return (ft_putstr_fd("Syntax error: invalid pipe\n", 2), 0);
            }
            else if (current->value[0] == '>' || current->value[0] == '<')
            {
                if (!current->next || !(current->next->type == WORD || current->next->type == QUOTED)) // Redirección sin archivo o token válido después
                    return (ft_putstr_fd("Syntax error: invalid redirection\n", 2), 0);
            }
        }
        current = current->next;
    }
    return (1); // Sintaxis válida
}
//------------------------SYNTAX CHECK
//------------------------EXPANSION
size_t	ft_strlcpy(char *dest, const char *src, size_t size)
{
	size_t	i;

	i = 0;
	if (size == 0)
		return (ft_strlen(src));
	while (src[i] && i < (size - 1))
	{
		dest[i] = src[i];
		i++;
	}
	dest[i] = 0;
	return (ft_strlen(src));
}

size_t	ft_strlcat(char *dst, const char *src, size_t size)
{
	size_t	dst_len;
	size_t	src_len;
	size_t	i;
	size_t	j;

	dst_len = ft_strlen(dst);
	src_len = ft_strlen(src);
	if (size == 0 || size <= dst_len)
		return (size + src_len);
	i = 0;
	j = dst_len;
	while (src[i] && j < (size - 1))
	{
		dst[j] = src[i];
		i++;
		j++;
	}
	dst[j] = 0;
	return (dst_len + src_len);
}

char	*ft_strjoin(char const *s1, char const *s2)
{
	char	*s3;
	size_t	s1len;
	size_t	s2len;

	if (!s1 || !s2)
		return (NULL);
	s1len = ft_strlen(s1);
	s2len = ft_strlen(s2);
	s3 = (char *)malloc(sizeof(char) * (s1len + s2len + 1));
	if (!s3)
		return (NULL);
	ft_strlcpy(s3, s1, s1len + 1);
	ft_strlcat(s3, s2, s1len + s2len + 1);
	return (s3);
}

char *ft_strjoin_free(char *s1, char *s2)
{
    char *result;

    if (!s1 && s2)
        return (ft_strdup(s2));
    if (!s2)
        return (s1);
    result = ft_strjoin(s1, s2);
    free(s1);
    return (result);
}

int is_valid_env_char(char c)
{
    return (ft_isalpha(c) || ft_isdigit(c) || c == '_');
}

char *expand_value(t_token *token, t_env *env_lst)
{
    int     i;
    int     start;
    char    *temp;
    char    *sub_expand;
    char    *expand;

    if (token->prev && ft_strncmp(token->prev->value, "<<", 2) == 0)
        return (ft_strdup(token->value));
    i = 0;
    expand = NULL;
    while (token->value[i])
    {
        // Caso: texto literal antes de un '$'
        if (token->value[i] != '$')
        {
            start = i;
            while (token->value[i] && token->value[i] != '$')
                i++;
            temp = ft_strndup(&token->value[start], i - start); // Extrae texto literal
            expand = ft_strjoin_free(expand, temp);   // Une y libera `expand`
            free(temp);
        }
        // Caso: expansión de variable
        else if (token->value[i++] == '$')
        {
            if (!token->value[i] || token->value[i] == ' ' || !is_valid_env_char(token->value[i])) // Caso literal
            {
            temp = ft_strdup("$"); // Conserva el símbolo literal
            expand = ft_strjoin_free(expand, temp);
            free(temp);
            }
            else
            {
                start = i;
                while (token->value[i] && is_valid_env_char(token->value[i]))
                    i++;
                temp = ft_strndup(&token->value[start], i - start); // Nombre de la variable
                sub_expand = get_env_value(temp, env_lst); // Busca en la lista de variables
                free(temp);                                // Libera `temp`
                expand = ft_strjoin_free(expand, sub_expand); // Une y libera `expand`
            }
        }
    }
    return (expand);
}

void expand_variables(t_token *token, t_env *env_lst)
{
    char *new_value;

    if (token->expand)
    {
        if (token->type == WORD || token->type == QUOTED)
            new_value = expand_value(token, env_lst);
        else
            return;
        // Libera el valor anterior y actualiza con el nuevo
        free(token->value);
        token->value = new_value;
    }
}


//------------------------EXPANSION
int main(int argc, char **argv, char **env)
{
    char    *line;
    t_token *tkn_lst;
    t_token *curr_tkn;
    t_env   *env_lst;

    (void)argv;
    if (argc != 1)
    {
        printf("Wrong number of arguments\n");
        return (1);
    }
    env_lst = init_env_list(env);
    while (1)
    {
        line = readline("minishell> ");
        if (!line)
            break;
        if (line)
        {
            if (ft_strncmp(line, "env", 3) == 0)
                print_env_list(env_lst);
            add_history(line);
            tkn_lst = tokenize(line);
            if (tkn_lst && !syntax_check(tkn_lst))
            {
                free_tkn_lst(tkn_lst);
                free(line);
                continue;
            }
            if (tkn_lst)
            {
                curr_tkn = tkn_lst;
                while (curr_tkn)
                {
                    expand_variables(curr_tkn, env_lst);
                    curr_tkn = curr_tkn->next;
                }
                curr_tkn = tkn_lst;
            }
            if (tkn_lst)
            {
                curr_tkn = tkn_lst;
                while(curr_tkn)
                {
                    printf("Token value: *%s*\n", curr_tkn->value);
                    printf("is quote? --> %d\n", curr_tkn->is_quote);
                    printf("expands? --> %d\n", curr_tkn->expand);
                    printf("Number of spaces --> %d\n", curr_tkn->has_space);
                    printf("type: %u\n", curr_tkn->type);
                    curr_tkn = curr_tkn->next;
                }
            }
            free_tkn_lst(tkn_lst);
        }
        free(line);
    }
    free_env_list(env_lst);
    return (0);
}