#include "minishell.h"

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