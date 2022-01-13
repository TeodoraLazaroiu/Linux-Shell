/*
Implementarea in C a unui shell cu elemente de baza: help, history,
clear, cd, pwd, ls, touch, rm, cp, makedir, removedir, echo,
pipe (|), expresii logice (&&, ||) si suspendarea programului (quit)

Proiect realizat de: Buta Gabriel, Lazaroiu Teodora, Roman Andrei 241
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <readline/readline.h>
#define NSize 1024

char ** words;
char *command_line, *output, *p, *nr_comanda, *history;
char current_path[NSize];
int nr_comenzi, nr_words, got_error;


// functie pentru afisarea mesajului specific unei erori
void parse_error(int error_code)
{
    if(error_code == 1) printf("No such file or directory\n");

    if(error_code == 2) printf("Unable to locate current path\n");

    if(error_code == 3) printf("Unable to open source file\n");

    if(error_code == 4) printf("Unable to open destination file\n");

    if(error_code == 5) printf("Unable to create folder\n");

    if(error_code == 6) printf("Unable to delete folder\n");

    if(error_code == 7) printf("Unable to create file\n");

    if(error_code == 8) printf("Unable to delete file\n");

    if(error_code == 9) printf("Invalid number of operands\n");

    if(error_code == 10) printf("Command '%s' not found\n", words[0]);

}

void help()
{
	
	printf("\nCommands:\n");
	printf("--------------------------------------------------------\n\n");

    printf("history: prints the command history of the current session \n");
	printf("clear: clears the terminal \n");
	printf("cd: changes the path to a particular directory \n");
	printf("pwd: prints the path of the current directory \n");
	printf("ls: lists all files and directories in the current directory \n");
    printf("touch: creates a new empty file \n");
    printf("rm: deletes a particular file \n");
	printf("cp: copies the content of a file to another file \n");
	printf("makedir: creates a new directory \n");
	printf("removedir: deletes an already existing directory \n");
    printf("echo: displays a string that is passed as an argument\n");
    printf("quit: exits the shell \n");

	printf("\n-----------------------------------------------------\n\n");
}

void hist()
{
    printf("%s", history);
}

void clear()
{
    // escape sequence pentru eliberarea terminalului
	write(1, "\33[H\33[2J", 7);
}

void cd(char* folder)
{
    if (chdir(folder))
    {
        got_error = 1;
    }
}

void pwd()
{
    // se obtine current path-ul cu ajutorul unei functii
    if (getcwd(current_path, sizeof(current_path))) 
    {
        strcat(output, current_path);
        printf("%s\n", output);
    } 
    else 
    {
        got_error = 2;
    }
}

void ls()
{
    // se creeaza un proces nou pentru
    // executarea functiei ls din bin
    pid_t pid = fork ();
    if (pid == 0)
    {
        char *arguments[] = {"ls", NULL};
        execve ("/bin/ls", arguments , NULL);
        kill(getpid(), 0);
    }
    else
    {
        int status;
        waitpid(pid, &status, 0);
    }
}

void touch(char* file)
{
    // se creeaza un nou fisier cu permisiune de scriere
    // si se deschide cu functia fopen
    FILE *aux;
    aux = fopen(file, "w");
    if (aux)
    {
        strcat(output, "File created succesfully");
        printf("%s\n", output);
    }
    else
    {
        got_error = 7;
    }
    
    fclose(aux);
}

void rm (char* filename)
{
    // se obtine current path-ul
    char file_path[1024];
    if (getcwd(file_path, sizeof(file_path)) == NULL) 
    {
        got_error = 2;
    }

    // adaugam numele fisierul ce trebuie sters
    // la current path pentru a il sterge
    strcat(file_path, "/");
    strcat(file_path, filename);

    if(!remove(file_path))
	{
        strcat(output, "File deleted succesfully");
        printf("%s\n", output);
	}
	else
	{
		got_error = 8;
	}
}

void cp(char* file1, char* file2)
{
	char curr_car;
	FILE *f1, *f2;
	
    // deschidem fisierul sursa pentru citire
	if ((f1 = fopen(file1, "r")) == 0)
	{
		got_error = 3;
        return;
	}

    // deschidem fisierul de destinatie pentru scriere
    if ((f2 = fopen(file2, "w")) == 0)
	{
		got_error = 4;
        return;
	}

    // citim continutul primului fisier caracter cu caracter
	curr_car = fgetc(f1);

	while(curr_car != EOF)
	{
		//scrie in al doilea fisier caracter cu caracter
		fputc(curr_car, f2);
		curr_car = fgetc(f1);
	}

    strcat(output, "File copied succesfully");
    printf("%s\n", output);

	fclose(f1);
	fclose(f2);
}

void makedir(char* folder)
{
    // se obtine current path-ul
    if (getcwd(current_path, sizeof(current_path)) == NULL) 
    {
        got_error = 2;
        return;
    }

    strcat(current_path, "/");
    strcat(current_path, folder);

    // se creeaza un director cu permisiuni
    // de scriere, citire si executie (0777)
    if (mkdir(current_path, 0777) == -1)
	{
        got_error = 5;
    }
    else
    {
        strcat(output, "Folder created succesfully");
        printf("%s\n", output);
    }
}

void removedir(char* folder)
{
    // se obtine current path-ul
    if (getcwd(current_path, sizeof(current_path)) == NULL) 
    {
        got_error = 2;
        return;
    }

    strcat(current_path, "/");
    strcat(current_path, folder);

    if(rmdir(current_path) == -1)
	{
        got_error = 6;
    }
    else
    {
        strcat(output, "Folder deleted succesfully");
        printf("%s\n", output);
    }
}

void echo()
{
    // parcurgem toate cuvintele scrise
    // dupa comanda 'echo' si le afisam
    for (int i = 1; i < nr_words; i ++)
    {
        printf("%s ", words[i]);
        strcat(output, words[i]);
        strcat(output, " ");
    }

    printf("\n");
}

// functia principala care apeleaza functiile
// de mai sus specifice fiecarei comenzi

void execute(char ** words, int nr_args)
{
    // inainte de executarea oricarei comenzi
    // se testeaza daca numarul de argumente
    // este corect, altfel se va afisa o eroare

    if (!strcmp(words[0], "help"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        help();
    }
    else if (!strcmp(words[0], "history"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        hist();
    }
    else if (!strcmp(words[0], "clear"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        clear();
    }
    else if (!strcmp(words[0], "cd"))
    {
        if (nr_args > 2)
        {
            got_error = 9;
            return;
        }
        if(nr_args == 2)cd(words[1]);
        else cd("..");
    }
    else if (!strcmp(words[0], "pwd"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        pwd();
    }
    else if(!strcmp(words[0], "ls"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        ls();
    }
    else if (!strcmp(words[0], "touch"))
    {
        if(nr_args != 2)
        {
            got_error = 9;
            return;
        }
        touch(words[1]);
    }
    else if (!strcmp(words[0], "cp"))
    {
        if (nr_args != 3)
        {
            got_error = 9;
            return;
        }
        cp(words[1], words[2]);
    }
    else if (!strcmp(words[0], "makedir"))
    {
        if(nr_args != 2)
        {
            got_error = 9;
            return;
        }
        makedir(words[1]);
    }
    else if (!strcmp(words[0], "removedir"))
    {
        if(nr_args != 2)
        {
            got_error = 9;
            return;
        }
       removedir(words[1]);
    }
    else if (!strcmp(words[0], "rm"))
    {
        if(nr_args != 2)
        {
            got_error = 9;
            return;
        }
        rm(words[1]);
    }

    else if (!strcmp(words[0], "echo"))
    {
        echo();
    }

    else if (!strcmp(words[0], "quit"))
    {
        if (nr_args != 1)
        {
            got_error = 9;
            return;
        }
        exit(0);
    }
    else
    {
        got_error = 10;
        return;
    }
}
int main()
{
    history  = malloc(NSize * 100 * sizeof(char));
    output = malloc(NSize * sizeof(char));

    while(1)
    {

        if (getcwd(current_path, sizeof(current_path)) != NULL) 
        {
            printf("SHELL: %s$ ", current_path);
        } 
        else 
        {
            got_error = 2;
        }

        command_line = readline("");

        // putem apasa enter in consola fara sa se intample nimic
        if(!strcmp(command_line, ""))
        {
            printf("\n");
            continue;
        }

        // adaugam comanda curenta in history
        nr_comenzi++;
        nr_comanda = malloc(5 * sizeof(char));

        sprintf(nr_comanda, "%d", nr_comenzi);

        strcat(history, nr_comanda);
        strcat(history, ": ");
        strcat(history, command_line); 
        strcat(history, "\n");

        // despartim comanda curenta in cuvinte
        words = malloc(NSize * sizeof(char*));
        nr_words = 0;

        p = strtok(command_line, " ");

        while(p != NULL)
        {
            // trecem prin fiecare cuvant al comenzii
            char* c = malloc(NSize * sizeof(char));
            strcpy(c, p);

            if (!strcmp(c, "|"))
            {
                // executa comanda dinainte de pipe
                if (nr_words > 0)
                {
                    free(output);
                    output = malloc(NSize * sizeof(char));
                    execute(words, nr_words);
                }

                // trece la comanda de dupa pipe
                p = strtok(NULL, " ");

                // cazul in care nu mai exista
                // nicio comanda dupa pipe
                if (p == NULL)
                {
                    got_error = 9;
                    continue;
                }
                strcpy(c, p);

                // in vectorul de cuvinte vor fi salvate comanda
                // de dupa pipe si rezultatul primei comenzi
                words[0] = c;
                strcpy(words[1], output);

                nr_words = 2;

                // comanda a doua va fi rulata avand ca
                // argument rezultatul primei comenzi
                free(output);
                output = malloc(NSize * sizeof(char));
                execute(words, nr_words);

                got_error = -1;

                nr_words = 0;
            }
            else if (!strcmp(c, "||"))
            {
                free(output);
                output = malloc(NSize * sizeof(char));
                execute(words, nr_words);

                if(got_error)
                {
                    // daca a intampinat o eroare o va ignora
                    // deoarece doar prima comanda corecta va rula
                    got_error = 0;
                    nr_words = 0;
                    p = strtok(NULL, " ");
                    continue;
                }
                else
                {
                    // cand gaseste prima comanda care
                    // nu da eroare le va ignora pe restul
                   nr_words = 0;
                   while(p != NULL)
                    {
                        strcpy(c, p);
                        if(!strcmp(c, "&&"))
                        {
                            break;
                        }
                        p = strtok(NULL, " ");
                    }
                    if (p == NULL) got_error = -1;
                }
            }
            else if (!strcmp(c, "&&"))
            {
                free(output);
                output = malloc(NSize * sizeof(char));
                execute(words, nr_words);
                
                // la prima eroare intalnita va opri executia
                if (got_error != 0)
                {
                    break;
                }

                nr_words = 0;
            }
            else words[nr_words ++] = c;

            p = strtok(NULL, " ");
        }

        // daca nu am avut o eroare
        // va executa comanda curenta
        if (got_error == 0)
        {
            free(output);
            output = malloc(NSize * sizeof(char));
            execute(words, nr_words);
        }

        // daca am avut o eroare se va afisa
        // mesajul corespunzator acesteia
        if (got_error != 0)
        {
            parse_error(got_error);
            got_error = 0;
        }
    }
    return 0;
}

// Exemple

// touch fisier | makedir
// echo sir de caractere | echo
// echo folder | makedir

// lss || ls
// cd nfm || ls || history
// cd nfm || ls && history

// cd nfm && ls
// help && ls
// ls && lss && help
// ls && cd nfm || ls