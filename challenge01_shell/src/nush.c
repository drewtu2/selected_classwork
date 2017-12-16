#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

#include "constants.h"
#include "tokens.h"
#include "svec.h"

int execute(svec* cmd);
int execute_pipe(svec* tokens);
int execute_base(svec* tokenized_cmd);
int execute_redirect_to(svec* tokenized_cmd);
int execute_redirect_from(svec* tokenized_cmd);
int execute_logical(svec* tokenized_cmd, int selector);
int execute_and(svec* tokenized_cmd);
int execute_or(svec* tokenized_cmd);
int execute_bg(svec* tokenized_cmd);
int execute_semi(svec* tokenized_cmd);

void assert_ok(int rv, char* symbol)
{
    if(rv == -1)
    {
        fprintf(stderr, "%s: failed\n", symbol);
        perror("error: ");
    }

}


int main(int argc, char* argv[])
{
    char cmd[MAX_LENGTH];
    svec* tokenized_cmd;
    //int procid = getpid();
    // In this case, we have not been given an argument.
    // Just run the standard shell
    if (argc == 1) {
        printf("nush$ ");
        //printf("%d nush$ ", procid);
        fflush(stdout);
        fgets(cmd, MAX_LENGTH, stdin);
        do {
            if(feof(stdin))
            {
                break;
            }

            tokenized_cmd = svec_make(MAX_LENGTH);
            tokenize(tokenized_cmd, cmd);
            execute(tokenized_cmd);
            svec_free(tokenized_cmd);    

            printf("nush$ ");
            //printf("%d nush$ ", procid);
            fflush(stdout);
        } while(fgets(cmd, MAX_LENGTH, stdin) != NULL);
    }
    else { // Run a given file
        FILE* fin = fopen(argv[1], "r"); 
        fgets(cmd, MAX_LENGTH, fin);
        do
        {
            if(feof(fin))
            {
                break;
            }

            tokenized_cmd = svec_make(MAX_LENGTH);
            tokenize(tokenized_cmd, cmd);
            execute(tokenized_cmd);
            svec_free(tokenized_cmd);    

        } while(fgets(cmd, MAX_LENGTH, fin) != NULL);
        fclose(fin);

        //printf("Read in: %s\n", cmd);
    }

    return 0;
}

int execute(svec* tokens)
{
    int cpid, rv, symbol_index;
    char* symbol;

    symbol_index = first_op(tokens);
    if(symbol_index >= 0)
    {
        symbol = svec_get(tokens, symbol_index);
    } else
    {
        symbol = "0";
    }

    // Contains a pipe
    if(strcmp(symbol, "|") == 0)
    {
        return execute_pipe(tokens);
    }
    // ; 
    else if(strcmp(symbol, ";") == 0)
    {
        return execute_semi(tokens);
    }
    // >
    else if(strcmp(symbol, ">") == 0)
    {
        return execute_redirect_to(tokens);        
    }
    
    // '<'
    else if(strcmp(symbol, "<") == 0)
    {
        return execute_redirect_from(tokens);        
    }
    
    // '&&'
    else if(strcmp(symbol, "&&") == 0)
    {
        return execute_and(tokens);        

    }
    
    // '||'
    else if(strcmp(symbol, "||") == 0)
    {
        return execute_or(tokens);        
    }
    
    // '&'
    else if(strcmp(symbol, "&") == 0)
    {
        return execute_bg(tokens);
    }
    else
    { 
        return execute_base(tokens); 
    }
    // Never get here 
}

int execute_pipe(svec* tokens)
{
    int cpid, rv;
    int symbol_index = has_pipe(tokens);
   
    int stdin_dup = dup(0);
    int stdout_dup = dup(1);

    svec* tokenized_right = svec_split(tokens, symbol_index);
    
    int pipe_fds[2];

    rv = pipe(pipe_fds);
    assert_ok(rv, "pipe");

    if ((cpid = fork())) {
        // parent process: in the parent, we should read the piped output from  
        // the child and use that output as the args for the right hand side of 
        // the pipe (tokenized_right)
        int p_read = pipe_fds[0];
        int p_write = pipe_fds[1];
        //char temp_word[MAX_LENGTH];

        // Only care about reading here.... Close the write fd. 
        close(p_write);
        dup2(p_read, fileno(stdin));
        rv = execute(tokenized_right);

        close(p_read);

        int status;
        waitpid(cpid, &status, 0);
        dup2(stdin_dup, 0);

        return rv;

    }
    else {
        // child process: In the child process, we need to close the read fd
        // and set fd(1) equal to our write pipe. We then execute the command
        // to the left of the pipe (tokens)
        int p_read = pipe_fds[0];
        int p_write = pipe_fds[1];

        close(p_read);

        rv = dup2(p_write, 1);  // Replace stdout with the write fd.
        assert_ok(rv, "dup");

        rv = close(p_write);         // write has been moved to fd(1)
        assert_ok(rv, "close");

        rv = execute(tokens); // Recursive call back to execute...
        dup2(stdout_dup, 1);
        _Exit(0);
        //char* args[] = {"ls", 0};
        //execvp("ls", args);
    }
}

int execute_semi(svec* tokenized_cmd)
{
    int symbol_index = has_semi(tokenized_cmd);
    // Split on semi index. 
    svec* right_cmd = svec_split(tokenized_cmd, symbol_index);
        
    execute(tokenized_cmd);
    execute(right_cmd);

    return 0;
}

int execute_redirect_to(svec* tokenized_cmd)
{
    
    int cpid;
    if ((cpid = fork())) {
        // parent process
        // Child may still be running until we wait.

        int status;
        waitpid(cpid, &status, 0);
        
        if(WIFEXITED(status))
        {
            int es = WEXITSTATUS(status);
            //printf("Child of execute_redirect_to exited with status: %d\n", es); 
            return es;
        } else if(WIFSIGNALED(status))
        {
            int es = WTERMSIG(status);
            printf("Child of execute_redirect_to failed with status: %d\n", es); 
        }

        return -1;

    }
    else {
        // child process
        //printf("== executed program's output: ==\n");
        
        // Find the symbol.
        int symbol_index = has_redirect_to(tokenized_cmd);

        // We want to redirect INTO the token following the symbol...
        char* target_name = svec_get(tokenized_cmd, symbol_index + 1);
        FILE* target_file = fopen(target_name, "w");
        
        // Command we want to execute is everything before the symbol
        tokenized_cmd = svec_trunc(tokenized_cmd, 0, symbol_index);

        // The set the output file to be the new stdout
        dup2(fileno(target_file), 1);
        fclose(target_file);

        // Run the program using the command data.... (has been truncated)
        //execvp(program, tokenized_cmd->data);
        _Exit(execute(tokenized_cmd));

    }


}

int execute_redirect_from(svec* tokenized_cmd)
{
    
    int cpid;
    if ((cpid = fork())) {
        // parent process
        // Child may still be running until we wait.

        int status;
        waitpid(cpid, &status, 0);
        if(WIFEXITED(status))
        {
            int es = WEXITSTATUS(status);
            //printf("Child of execute_redirect_from exited with status: %d\n", es); 
            return es;
        } else if(WIFSIGNALED(status))
        {
            int es = WTERMSIG(status);
            printf("Child of execute_redirect_from failed with status: %d\n", es); 
        }
        return -1;

    }
    else {
        // child process
        //printf("== executed program's output: ==\n");
        int symbol_index = has_redirect_from(tokenized_cmd);

        // In a redirect FROM, we should read from the argument directly to the
        // left of the symbol. That argument is going to be a file name....
        // Open and read from there. 
        char* target_name = svec_get(tokenized_cmd, symbol_index + 1);
        FILE* target_file = fopen(target_name, "r");
        
        dup2(fileno(target_file), 0);
        fclose(target_file);
      
         

        // Truncate the command to be everything to the left of the symbol
        tokenized_cmd = svec_trunc(tokenized_cmd, 0, symbol_index);

        _Exit(execute(tokenized_cmd));

    }


}

int execute_and(svec* tokenized_cmd)
{
    return execute_logical(tokenized_cmd, 0);
}

int execute_or(svec* tokenized_cmd)
{
    return execute_logical(tokenized_cmd, 1);
}

// Responsible for handling the logic of logical and or or.
// Selector 0: AND, run second cmd if first returned 0
// Selector 1: OR, run second cmd if first returned !0
int execute_logical(svec* tokenized_cmd, int selector)
{
    
    int cpid, symbol_index;
    
    // In and AND operator, we need to run the left hand side first. if it
    // returns true, then run the second half. Return the result of the
    // second half. (know first to be true)
    if(selector == 0)
    {
        symbol_index = has_and(tokenized_cmd);
    }
    
    // In an OR operator, we need to run the left hand side first. if it
    // returns false, then run the second half. Return the result of the
    // second half. (know first to be true)
    else if(selector == 1)
    {
        symbol_index = has_or(tokenized_cmd);
    }
    else
    {
        _Exit(-1);
    }

    svec* right_cmd = svec_split(tokenized_cmd, symbol_index);

    if ((cpid = fork())) {
        // parent process
        // Child may still be running until we wait.

        int status;
        waitpid(cpid, &status, 0);

        if(WIFEXITED(status))
        {
            int es = WEXITSTATUS(status);
            if(selector == 0 && es == 0) // AND
            {
                return execute(right_cmd);
            } else if(selector != 0 && es > 0)
            {
                return execute(right_cmd); 
            }
        } else if(WIFSIGNALED(status))
        {
            int es = WTERMSIG(status);
            printf("Child of execute_logical failed with status: %d\n", es); 
        }
    } else {
        // child process
        //printf("== executed program's output: ==\n");
        // Exit with the return code from our command
        _Exit(execute(tokenized_cmd));

        // Kill ourselves so that our parents know whats up!
        _Exit(-1);
   }


}


int execute_bg(svec* tokenized_cmd)
{
    int cpid;
    char* program = svec_get(tokenized_cmd, 0);

    if(strcmp(program,"cd") == 0)
    {
        chdir(svec_get(tokenized_cmd, 1));
    } else if ((cpid = fork())) {
        // parent process
        // We want the child to run in the background... don't wait for it.
        // Return immediately... 
    }
    else {
        // child process
        //printf("== executed program's output: ==\n");

        if(strcmp(program, "exit") == 0)
        {
            _Exit(0);
        } else
        {
            execvp(program, tokenized_cmd->data);
            assert(0);
        }

        printf("execute_bg: Can't get here, exec only returns on error.");
    }
}

int execute_base(svec* tokenized_cmd)
{
    int cpid;
    char* program = svec_get(tokenized_cmd, 0);
    char** cmd_sequence = usedArgs(tokenized_cmd, tokenized_cmd->size); 

    if(strcmp(program,"cd") == 0)
    {
        chdir(svec_get(tokenized_cmd, 1));
    } else if(strcmp(program, "exit") == 0)
    {
        exit(0);        
    }else if ((cpid = fork())) {
        // parent process
        // Child may still be running until we wait.

        //int procid = getpid();
        //printf("%d executing %s\n", cpid, program);

        int status;
        waitpid(cpid, &status, 0);
        free(cmd_sequence);
        
        if(WIFEXITED(status))
        {
            int es = WEXITSTATUS(status);
            //printf("%d of execute_base exited with status: %d\n", cpid, es); 
            return es;
        } else if(WIFSIGNALED(status))
        {
            int es = WTERMSIG(status);
            printf("Child of execute_base failed with status: %d\n", es); 
        }

        // If we didn't exit properly from our children, return -1 up to the parent
        return -1;
    }
    else {
        // child process
        //printf("== executed program's output: ==\n");
        execvp(program, cmd_sequence);
        assert(0);

        printf("execute_base: Can't get here, exec only returns on error.");
    }
}
