#include "minishell.h"

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
    if (str[0] == '$')
        str++;
    while (env_list)
    {
        if (ft_strncmp(str, env_list->key, ft_strlen(str)) == 0)  
            return (env_list->value);
        env_list = env_list->next;
    }
    return (NULL);
}