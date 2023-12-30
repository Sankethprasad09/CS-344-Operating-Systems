#include<stdio.h>
#include<stdlib.h>
#include <stdbool.h>
#define MAX_LIMIT 20
#include <sys/stat.h> 
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>


typedef struct Movie {
    char title[100];
    int year;
    char language[100];     // creating the structure to hold each movie details 
    double rating;          //structure has same fields as movie details like title, year, langauge, and it's rating
    struct Movie *next;
} Movie; 


void largest_file();
void file_select();
void search_file();
bool file_exists (char filename[]);
char* create_directory();
void smallest_file1();


void create_year_files(Movie *list, const char *dir_name) {
    // Create the directory if it doesn't exist
    mkdir(dir_name, 0777);

    // Traverse the linked list and create one file per year
    while (list != NULL) {
        // Build the file name
        char file_name[100];
        sprintf(file_name, "%s/%d.txt", dir_name, list->year);

        // Open the file for appending
        FILE *file = fopen(file_name, "a");
        if (file == NULL) {
            fprintf(stderr, "Error: could not open file %s\n", file_name);
            exit(EXIT_FAILURE);
        }

        // Write the movie title to the file
        fprintf(file, "%s\n", list->title);

        // Close the file
        fclose(file);

        // Move to the next movie
        list = list->next;
    }
}


void file_select()
{
    int choice;
    bool again = false;
    while(!again)
    {
    printf("Which file you want to process?\n");
    printf("Enter 1 to pick the largest file\n");
    printf("Enter 2 to pick the smallest file\n");
    printf("Enter 3 to specify the name of a file\n");
    printf("Enter a choice from 1 to 3: ");
    scanf("%d",&choice);
    
    char str[MAX_LIMIT];
    switch (choice)
    {
    case 1:
        largest_file();
        again=true;
        break;
    
    case 2:
        smallest_file1();
        again = true;
        break;
    
    case 3: 
        printf("please enter the name of the file you want to search: ");
        fflush(stdin);
        fgets(str, MAX_LIMIT, stdin);
        str[strcspn(str, "\n")] = 0;

        if (file_exists(str))
            {  
            search_file(str);
            again = true;
            } 
        else
            printf("file %s does not exist in the current directory\n", str);
            
        
        break;
    
    default:
        printf("Invalid choice. Please try again.\n");
        break;
    }
    }
       
}

Movie *create_movie(char *title, int year, char *language, double rating) {
    // fprintf(stderr, " creating movie, values are %s %d %s %lf", title,year, language,rating);
    Movie *new_movie = (Movie *)malloc(sizeof(Movie));
    strcpy(new_movie->title, title);
    new_movie->year = year;
    strcpy(new_movie->language, language);
    new_movie->rating = rating;
    new_movie->next = NULL;
    return new_movie;
}


void create_files_by_year(Movie *movies,char* directory) {
    int num_years = 0;
    int *years = NULL;
    Movie *ptr = movies;
    while (ptr != NULL) {
        int year = ptr->year;
        int found = 0;
        for (int i = 0; i < num_years; i++) {
            if (years[i] == year) {
                found = 1;
                break;
            }
        }
        if (!found) {
            num_years++;
            years = (int *)realloc(years, num_years * sizeof(int));
            years[num_years - 1] = year;
        }
        ptr = ptr->next;
    }

    

    
    // Now, for each unique year, we create a file in the "sanketh" directory and write the titles of the movies released in that year
    for (int i = 0; i < num_years; i++) {
        char filename[15]; // sanketh/yyyy.txt + '\0'
        sprintf(filename, "%d.txt", years[i]);
        strcat(directory,"/");
        strcat(directory,filename);
    //
        FILE *fp = fopen(filename, "w");
        fprintf(stderr, "file name %s",filename);
        if (fp == NULL) {
            fprintf(stderr, "Error: Could not open file %s\n", filename);
            continue;
        }
        chmod(filename, S_IRUSR | S_IWUSR | S_IRGRP); // set permissions to rw-r-----

        ptr = movies;
        while (ptr != NULL) {
            if (ptr->year == years[i]) {
                fprintf(fp, "%s\n", ptr->title);
                // fprintf(stderr, "printing title ");
            }
            ptr = ptr->next;
        }

        fclose(fp);
    }

    free(years);
}

// after creating new node for each movie, add the new movie node to linked list

Movie *add_movie_to_list(Movie *head, Movie *new_movie) {
    if (head == NULL) {
        return new_movie;
    } else {
        Movie *current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_movie;
        return head;
    }
}

void print_moviess(Movie *head)
{
    while (head != NULL) {
        
            printf("%s %d %s %lf\n", head->title, head->year, head->language, head->rating);
            head= head->next;
            
        }

}


// based on the year  given print all the movies present in the linked list, take the year and traverse the linked list till the end 
// and compare the user given year with each node's year
void print_movies_from_year(Movie *head, int year) {
    Movie *current = head;
    int count = 0;
    while (current != NULL) {
        if (current->year == year) {
            printf("%s\n", current->title);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("No movies found for the year %d.\n", year);
    }
}


Movie *process_file(char* filename)
{
     FILE *file = fopen(filename, "r"); 
     if (file == NULL) {
        printf("Error opening the file.\n");
        return 1;
    }

    char line[256];
    fgets(line, sizeof(line), file); // Skip header line in the movie file

    Movie *movies_list = NULL;    // initilizing movie list to null
    char title[100], language[50];
    int year;
    double rating;

    

    while (fgets(line, sizeof(line), file)) {                    // reading all the lines in the file line by line
               

        char *saveptr;
        char *end;
       char *token   = strtok_r(line, ",", &saveptr);             // creating token from the line based on "comma" beacuase movie file is csv file
       if (token[0] == '\"') {
        memmove(token, token + 1, strlen(token));
    }

        strcpy(title, token);                                     // copying the token to title variable which holds the movie title

        year= atoi(strtok_r(NULL, ",", &saveptr));                 // converting string year to int year using atoi
     
        token  =  strtok_r(NULL, ",", &saveptr);
      
         
        if (token[0] == '[')                                        // removing "[" present in the langaues field in the csv file
        memmove(token, token + 1, strlen(token));

        token[strlen(token)-1]= '\0';                               //   removing the closing square bracket("]") present in the end of the langauge field
       strcpy(language, token);
        rating = strtod(strtok_r(NULL, "\n", &saveptr),&end);
        Movie *new_movie = create_movie(title, year, language, rating);       //  creating the movie node usin the movie details collected in the above steps
        movies_list = add_movie_to_list(movies_list, new_movie);  
                     // adding new movie node to linked list
        
    }

    fclose(file);  
    
    return movies_list;                                                             // closing the file pointer  after reading all the files


}


char *create_directory()
{
    char onid[50]="karutusa.movies.";
    srand(time(0));
    int random  = rand()%99999 + 1;
    char ran[MAX_LIMIT] ;
    sprintf(ran, "%d", random);
     strcat(onid,ran);

    printf("%s\n", onid);

    struct stat st = {0};

    int permissions = S_IRWXU | S_IRGRP | S_IXGRP;
    int result = mkdir(onid, permissions | O_CREAT | O_EXCL);
    if (result != 0) {
        perror("mkdir");
        return 1;
    }
    result = chmod(onid, permissions);
    if (result != 0) {
        perror("chmod");
        return 1;
    }

    return onid;
}



void largest_file()
{
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    off_t max_size = 0;
    char *max_name = NULL;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    bool loop = false;

    while (((entry = readdir(dir)) != NULL) && !loop) {
        if (entry->d_type == DT_REG) {  // regular file
            if (strncmp(entry->d_name, "movies_", 7) == 0 && strstr(entry->d_name, ".csv") != NULL) {
                if (stat(entry->d_name, &filestat) == -1) {
                    perror("stat");
                    continue;
                }
                if (filestat.st_size > max_size) {

                    max_size = filestat.st_size;
                    max_name = entry->d_name;   
                    
                }
            }
        }
    }

    char *dir_name = create_directory();
                   Movie *movies_list = NULL; 
                   movies_list = process_file(max_name);
                //    print_moviess(movies_list); 
                //printf("dir name %s\n",dir_name);
                // create_files_by_year(movies_list,dir_name);
                create_year_files(movies_list,dir_name);

    closedir(dir);


    if (max_name != NULL) {
        printf("Now processing the choosen file named %s\n", max_name);
    } else {
        printf("No matching CSV files found in current directory.\n");
    }

    return;
}


void smallest_file1()
{
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    off_t max_size = 10000;
    char *max_name = NULL;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

bool loop2=false;

    while (((entry = readdir(dir)) != NULL) && !loop2) {
        
        if (entry->d_type == DT_REG) {  // regular file
            if (strncmp(entry->d_name, "movies_", 7) == 0 && strstr(entry->d_name, ".csv") != NULL) {
                if (stat(entry->d_name, &filestat) == -1) {
                    perror("stat");
                    continue;
                }

                if (filestat.st_size <= max_size) {
                    //  printf("inside\n");
                    max_size = filestat.st_size;
                    max_name = entry->d_name;
    
                }
            }
        }
    }

        char *dir_name = create_directory();
        Movie *movies_list = NULL; 
        movies_list = process_file(max_name);
        create_year_files(movies_list,dir_name);

    closedir(dir);


    if (max_name != NULL) {
        printf("Now processing the choosen file named %s\n", max_name);
    } else {
        printf("No matching CSV files found in current directory.\n");
    }
}




void search_file(const char *filename)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(".");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // regular file
            if (strcmp(entry->d_name, filename) == 0) {
                printf("Now processing the choosen file named %s\n", filename);

                if (strncmp(entry->d_name, "movies_", 7) == 0 && strstr(entry->d_name, ".csv") != NULL) {
               

                char *dir_name = create_directory();
                    Movie *movies_list = NULL; 
                    movies_list = process_file(filename);
                    create_year_files(movies_list,dir_name);

                }

                closedir(dir);
                return;
            }
        }
    }

    closedir(dir);
    printf("Error: file '%s' not found in current directory.\n", filename);
}

bool file_exists (char filename[]) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

int main()
{
    int choice;
    bool exit = false;
    while(!exit)
    {
        printf("1. Select file to process\n2. Exit the program\n");
        printf("Enter a choice 1 or 2: ");
        scanf("%d",&choice);
        if(choice==1)
        {
            file_select();
            exit=true;
        }
        else if(choice==2)
        {
            exit=true;
        }
        else
        {
            printf("please Enter the valid choice 1 or 2\n");
        }

    }

    return 0;
}
