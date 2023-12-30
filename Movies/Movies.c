#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>   // adding all the header files needed



typedef struct Movie {
    char title[100];
    int year;
    char language[100];     // creating the structure to hold each movie details 
    double rating;          //structure has same fields as movie details like title, year, langauge, and it's rating
    struct Movie *next;
} Movie;    



// for each new line in the file which shows each movie details, create a node in the linked list which is of type struct movie

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


//print all the details about a movie present in the linked list as seperate nodes
void print_moviess(Movie *head)
{
    while (head != NULL) {
        
            printf("%s %d %s %lf\n", head->title, head->year, head->language, head->rating);
            head= head->next;
            
        }

}


// considering the upper and lower bound for the year as mentioned in the assignment,  
// compare rating of all the movies for each year and print the highest rated movie in that year
void print_highest_rated_movies_per_year(Movie *head) {
    int min_year = 1900;
    int max_year = 2021;
    for (int year = min_year; year <= max_year; year++) {
        Movie *current = head;
        Movie *highest_rated_movie = NULL;
        while (current != NULL) {
            if (current->year == year) {
                if (highest_rated_movie == NULL || current->rating > highest_rated_movie->rating) {
                    highest_rated_movie = current;
                }
            }
            current = current->next;
        }

        if (highest_rated_movie != NULL) {
            printf("%d %.1f %s\n", year, highest_rated_movie->rating, highest_rated_movie->title);
        }
    }
}



// search for the user given  language in the language field of movies, here i am doing string based search, it is case sensative. 
//I'm using strstra() function to find if the given language is present of not
void print_movies_for_language(Movie *head, char *language) {
    Movie *current = head;
    int count = 0;
    while (current != NULL) {
        if (strstr(current->language, language) !=NULL){
            printf("%d %s\n", current->year, current->title);
            count++;
        }
        current = current->next;
    }

    if (count == 0) {
        printf("No movies found for the language %s.\n", language);
    }
}


// counting the total number of years by traversing through the linked list till the end and returning the count
int count_movies(Movie *head) {
    Movie *current = head;
    int count = 0;
    while (current != NULL) {
        
            count++;
        
        current = current->next;
    }

    return count;
}



// after execting all the user instruction, freeing up the space used for linked list
void free_movies_list(Movie *head) {
    Movie *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}


//main function
int main(int argc, char *argv[]) {
    if (argc != 2) {                                     // for taking movie file input in command line argument, checking whether the input is given or not
        printf("Usage: %s <csv_file>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");     // opening the movie file given in command line argument
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
        movies_list = add_movie_to_list(movies_list, new_movie);                // adding new movie node to linked list
        
    }

    fclose(file);                                                               // closing the file pointer  after reading all the files

    int choice;
    bool exit_program = false;

    printf("Processed file %s and parsed data for %d movies\n",argv[1], count_movies(movies_list) );         // printing the file information

    // printf(" content of the file\n");
    // print_moviess(movies_list);
    while (!exit_program) {
        printf("\nChoose an option:\n");
        printf("1. Show movies released in a specified year\n");
        printf("2. Show highest rated movie for each year\n");
        printf("3. Show movies and their year of release for a specific language\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                printf("Enter the year for which you want movie details: ");
                scanf("%d", &year);
                print_movies_from_year(movies_list, year);
                break;
            case 2:
                print_highest_rated_movies_per_year(movies_list);
                break;
            case 3:
                printf("Enter the language for which you want movie details: ");
                scanf("%s", language);
                print_movies_for_language(movies_list, language);
                break;
            case 4:
                exit_program = true;
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }

    free_movies_list(movies_list);
    return 0;
}
