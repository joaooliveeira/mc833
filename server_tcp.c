#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <cjson/cJSON.h>

#define MAX 10000 // Size of buffer
#define PORT 8080

#define SA struct sockaddr

cJSON *get_profile_by_email(cJSON *profile_list, char *email);
char *delete_profile_by_email(cJSON *profile_list, char *profile_email);
cJSON *filter_profiles_by_graduation_year(cJSON *profile_list, char *find_genre);
cJSON *filter_profiles_by_academic_background(cJSON *profile_list, char *academic_background_filter);
cJSON *filter_profiles_by_skill(cJSON *profile_list, char *skill);
void save_profile_file(cJSON *profile_list, FILE *profiles_file);
void init_profiles_system(int sockfd);

int main() {
	pid_t pid;
	int socketfd, new_socketfd, len; 
	struct sockaddr_in servaddr, cli; 

	// Socket create and verification 
	socketfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (socketfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} else {
		printf("Socket successfully created..\n"); 
	}
		
	bzero(&servaddr, sizeof(servaddr));

	// Assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification 
	if ((bind(socketfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    } else {
        printf("Socket successfully binded..\n");
	}

	// Now server is ready to listen and verification 
	if ((listen(socketfd, 5)) != 0) { 
		printf("Listen failed...\n"); 
		exit(0);
	} else {
		printf("Server listening..\n"); 
	}

	while (1)
	{
		len = sizeof(cli); 
		// Accept the data packet from client and verification 
		new_socketfd = accept(socketfd, (SA*)&cli, &len);

		if (new_socketfd < 0) { 
			printf("server acccept failed...\n"); 
		} else {
			printf("server acccept the client...\n");

			if ((pid = fork()) == 0) {
				close(socketfd);
				init_profiles_system(new_socketfd);
				exit(0);
			}
		}

		close(new_socketfd); 
	}
	
} 

void init_profiles_system(int sockfd) { 
	// Get the list of profiles in the profiles.txt file.
	FILE *profiles_file;
	profiles_file = fopen("profiles.txt", "r");
	 
	// Get all the content of profiles_file
	int count = 0;
	char *json = malloc(sizeof(char) * 50000);
	while((json[count++] = fgetc(profiles_file)) != EOF);

	cJSON *profile_list = NULL;

	if (profiles_file != NULL) {
		int size = ftell(profiles_file);
		
		if (size == 0) {
			profile_list = cJSON_CreateArray();		
		} else {
			profile_list = cJSON_Parse(json);
		}
	} else {
		printf("Error opening file");
		return;
	}

	free(json);

	char buff[MAX]; 
	int n;
	
	while (1) { 
		// Reset buffer
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		if ((read(sockfd, buff, sizeof(buff))) <= 0) {
			break;
		}

		// print buffer which contains the client contents 
		printf("Command from client : %s\n", buff); 

		// POST: Create new profile.
		if(strncmp("1", buff, 1) == 0) {
			bzero(buff, MAX);
			read(sockfd, buff, sizeof(buff));
			cJSON_AddItemToArray(profile_list, cJSON_Parse(buff));

			save_profile_file(profile_list, profiles_file);
			
			strcpy(buff, "Profile created.");
			write(sockfd, buff, sizeof(buff));
		}

		// POST: Add professional experience to a profile.
		if(strncmp("2", buff, 1) == 0) {
			bzero(buff, MAX);
			read(sockfd, buff, sizeof(buff));

			cJSON *body = cJSON_Parse(buff); // body contains email and professional experience.
			cJSON *email = cJSON_GetObjectItem(body, "email");
			cJSON *professional_experience = cJSON_GetObjectItem(body, "professional_experience");

			cJSON *profile = get_profile_by_email(profile_list, cJSON_GetStringValue(email));

			if (profile == NULL) {
				strcpy(buff, "Profile not found.");
			} else {
				cJSON *experience_array = NULL;
				experience_array = cJSON_GetObjectItem(profile, "experiences");

				cJSON *experience = NULL;
				experience = cJSON_CreateObject();
				cJSON_AddItemToArray(experience_array, experience);
				cJSON_AddItemToObject(experience, "description", professional_experience);

				// Update profiles file.
				profiles_file = fopen("profiles.txt","w"); 
				fprintf(profiles_file, "%s", cJSON_Print(profile_list));
				fclose(profiles_file);

				strcpy(buff, "Professional experience added successfully.");
			}

			write(sockfd, buff, sizeof(buff));
		}

		// GET: Filter profiles by academic background.
		if(strncmp("3",buff,1) == 0) {
			bzero(buff, MAX);
			read(sockfd, buff, sizeof(buff)); 

			cJSON *result = filter_profiles_by_academic_background(profile_list, buff);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			write(sockfd, buff, sizeof(buff));
		}

		// GET: Filter profile by skill
		if(strncmp("4", buff, 1) == 0) {
			bzero(buff, MAX); 
			read(sockfd, buff, sizeof(buff));

			cJSON *result = filter_profiles_by_skill(profile_list, buff);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			write(sockfd,buff, sizeof(buff));  
		}

		// GET: Filter profiles by graduation year
		if(strncmp("5",buff,1) == 0) {
			bzero(buff, MAX); 
			read(sockfd, buff, sizeof(buff)); 

			cJSON *result = filter_profiles_by_graduation_year(profile_list, buff);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			write(sockfd,buff,sizeof(buff));  
		}

		// GET: Return all profiles 
		if(strncmp("6", buff, 4) == 0) {
			bzero(buff, MAX); 

			if(cJSON_GetArraySize(profile_list) == 0){
				strcpy(buff, "[]");
			} else {
				strcpy(buff, cJSON_Print(profile_list));
			}

			write(sockfd, buff, sizeof(buff));  
		}
		
		// GET: Return all profile info by email
		if(strncmp("7", buff, 1) == 0) {
			bzero(buff, MAX);
			read(sockfd, buff, sizeof(buff));

			cJSON *profile = get_profile_by_email(profile_list, buff);
			if (profile != NULL) {
				strcpy(buff, cJSON_Print(profile));
			} else
				strcpy(buff, "Profile not found.");

			write(sockfd, buff, sizeof(buff));  
		}

		// DELETE: Remove pofile by email
		if(strncmp("8", buff, 1) == 0) {
			bzero(buff, MAX); 
			read(sockfd, buff, sizeof(buff)); 
			strcpy(buff, delete_profile_by_email(profile_list, buff));

			// update profiles file.
			profiles_file = fopen("profiles.txt","w"); 
			fprintf(profiles_file, "%s", cJSON_Print(profile_list));
			fclose(profiles_file);

			write(sockfd, buff, sizeof(buff));
		}
	} 
}

void save_profile_file(cJSON *profile_list, FILE *profiles_file) {
	profiles_file = fopen("profiles.txt","w"); 
	fprintf(profiles_file, "%s", cJSON_Print(profile_list));
	fclose(profiles_file);
}

cJSON *get_profile_by_email(cJSON *profile_list, char *email) {
    cJSON *profile = NULL;

    cJSON_ArrayForEach(profile, profile_list) {
        cJSON *item_email = cJSON_GetObjectItem(profile, "email");
        if (strcmp(item_email->valuestring, email) == 0){
            return profile;
        }
    }

    return NULL;
}

char *delete_profile_by_email(cJSON *profile_list, char *profile_email) {
    int index = 0;
    cJSON *profile = NULL;

    cJSON_ArrayForEach(profile, profile_list) {
		cJSON *item_email = cJSON_GetObjectItem(profile, "email");

        if (strcmp(item_email->valuestring, profile_email) == 0){
            cJSON_DeleteItemFromArray(profile_list, index);
            return "Profile deleted.";
        }
       
        index++;
    }

    return "Profile not found.";
}

cJSON *filter_profiles_by_graduation_year(cJSON *profile_list, char *year) {
    cJSON *result = cJSON_CreateArray();
    cJSON *profile = NULL;

    cJSON_ArrayForEach(profile, profile_list) {
        cJSON *graduation_year = cJSON_GetObjectItem(profile, "graduation_year");
        if (strcmp(graduation_year->valuestring, year) == 0) {
			cJSON *aux = cJSON_Parse(cJSON_Print(profile));
            cJSON_AddItemToArray(result, aux);
        }
    }

    return result;
}

cJSON *filter_profiles_by_skill(cJSON *profile_list, char *skill) {
    cJSON *result = cJSON_CreateArray();
    cJSON *profile = NULL;

    cJSON_ArrayForEach(profile, profile_list) { // For each profile
        cJSON *skills = cJSON_GetObjectItem(profile, "skills");
		cJSON *item = NULL;

		cJSON_ArrayForEach(item, skills) { // For each skill
			cJSON *description = cJSON_GetObjectItem(item, "description");
			if (strcmp(description->valuestring, skill) == 0) {
				cJSON *aux = cJSON_Parse(cJSON_Print(profile));
				cJSON_AddItemToArray(result, aux);
				break;
			}
		}
    }

    return result;
}

cJSON *filter_profiles_by_academic_background(cJSON *profile_list, char *academic_background_filter) {
	cJSON *result = cJSON_CreateArray();
    cJSON *profile = NULL;

    cJSON_ArrayForEach(profile, profile_list) { // For each profile
        cJSON *academic_background = cJSON_GetObjectItem(profile, "academic_background");

        if (strcmp(academic_background->valuestring, academic_background_filter) == 0) {
			cJSON *aux = cJSON_Parse(cJSON_Print(profile));
            cJSON_AddItemToArray(result, aux);
        }
    }

    return result;
}