#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include "cJSON.h"

#define MAX 10000 // Size of buffer
#define PORT 8080

#define SA struct sockaddr

cJSON *get_profile_by_email(cJSON *profile_list, char *email);
char *delete_profile_by_email(cJSON *profile_list, char *profile_email);
cJSON *filter_profiles_by_graduation_year(cJSON *profile_list, char *find_genre);
cJSON *filter_profiles_by_academic_background(cJSON *profile_list, char *academic_background_filter);
cJSON *filter_profiles_by_skill(cJSON *profile_list, char *skill);
void save_profile_file(cJSON *profile_list, FILE *profiles_file);

int main() {
	int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buff[MAX];
      
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed...");
        exit(0);
    } else {
		printf("socket created successfully...\n");
	}
      
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));
      
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
      
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(0);
    }

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

	int len, n;
	
	while (1) { 
		// Reset buffer
		bzero(buff, MAX); 

		len = sizeof(cliaddr);
    	n = recvfrom(sockfd, (char *)buff, MAX, 0, ( struct sockaddr *) &cliaddr, &len);
		buff[n] = '\0';
		
		// print buffer which contains the client contents 
		printf("Request from client:\n%s\n", buff);

		cJSON *request = cJSON_Parse(buff);
		cJSON *operation = cJSON_GetObjectItem(request, "operation");

		// POST: Create new profile.
		if(strncmp("1", operation->valuestring, 1) == 0) {
			bzero(buff, MAX);
			cJSON *body = cJSON_GetObjectItem(request, "body");

			cJSON_AddItemToArray(profile_list, cJSON_Parse(body->valuestring));

			save_profile_file(profile_list, profiles_file);
			
			strcpy(buff, "Profile created.");

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		}

		// POST: Add professional experience to a profile.
		if(strncmp("2", operation->valuestring, 1) == 0) {
			bzero(buff, MAX);
			cJSON *body = cJSON_GetObjectItem(request, "body");

			char aux[MAX];
			strcpy(aux, body->valuestring);
			aux[strlen(body->valuestring)] = "}";

			body = cJSON_Parse(aux);

			cJSON *email = cJSON_GetObjectItem(body, "email");
			cJSON *professional_experience = cJSON_GetObjectItem(body, "professional_experience");

			cJSON *profile = get_profile_by_email(profile_list, cJSON_GetStringValue(email));

			if (profile == NULL) {
				bzero(buff, MAX);
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

				bzero(buff, MAX);
				strcpy(buff, "Professional experience added successfully.");
			}

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		}

		// GET: Filter profiles by academic background.
		if(strncmp("3", operation->valuestring, 1) == 0) {
			cJSON *body = cJSON_GetObjectItem(request, "body");

			cJSON *result = filter_profiles_by_academic_background(profile_list, body->valuestring);

			bzero(buff, MAX);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
		}

		// GET: Filter profile by skill
		if(strncmp("4", operation->valuestring, 1) == 0) {
			cJSON *body = cJSON_GetObjectItem(request, "body");

			cJSON *result = filter_profiles_by_skill(profile_list, body->valuestring);
			bzero(buff, MAX);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
		}

		// GET: Filter profiles by graduation year
		if(strncmp("5", operation->valuestring, 1) == 0) {
			cJSON *body = cJSON_GetObjectItem(request, "body");

			cJSON *result = filter_profiles_by_graduation_year(profile_list, body->valuestring);
			bzero(buff, MAX);
			strcpy(buff, cJSON_Print(result));
			cJSON_Delete(result); // Deallocate memory

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);  
		}

		// GET: Return all profiles 
		if(strncmp("6", operation->valuestring, 4) == 0) {
			if(cJSON_GetArraySize(profile_list) == 0) {
				bzero(buff, MAX);
				strcpy(buff, "[]");
			} else {
				bzero(buff, MAX);
				strcpy(buff, cJSON_Print(profile_list));
			}

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		}
		
		// GET: Return all profile info by email
		if(strncmp("7", operation->valuestring, 1) == 0) {
			cJSON *body = cJSON_GetObjectItem(request, "body");

			cJSON *profile = get_profile_by_email(profile_list, body->valuestring);
			if (profile != NULL) {
				bzero(buff, MAX);
				strcpy(buff, cJSON_Print(profile));
			} else {
				bzero(buff, MAX);
				strcpy(buff, "Profile not found.");
			}

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		}

		// DELETE: Remove pofile by email
		if(strncmp("8", operation->valuestring, 1) == 0) {
			cJSON *body = cJSON_GetObjectItem(request, "body");

			bzero(buff, MAX);
			strcpy(buff, delete_profile_by_email(profile_list, body->valuestring));

			// update profiles file.
			profiles_file = fopen("profiles.txt","w"); 
			fprintf(profiles_file, "%s", cJSON_Print(profile_list));
			fclose(profiles_file);

			sendto(sockfd, (const char *)buff, strlen(buff), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
		}
	}

	return 0;
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