#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include "cJSON.h"

#define MAX 10000
#define IP "127.0.0.1"
#define PORT 8080

#define SA struct sockaddr 

void run_profiles_system(int sockfd);
void show_operations();
cJSON *create_new_profile(void);
void print_profile_list(cJSON *profiles);
void print_email_name_and_course(cJSON *profile_list);
void print_email_and_name(cJSON *profile_list);
void print_profile(cJSON *profile);

int main() { 
	int sockfd, connfd; 
	struct sockaddr_in servaddr, cli; 

	// Socket create and verificação 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 

	bzero(&servaddr, sizeof(servaddr)); 

	// Assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = inet_addr(IP); 
	servaddr.sin_port = htons(PORT); 

	// Connect the client socket to server socket 
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
		printf("connection with the server failed...\n"); 
		exit(0); 
	} else {
		printf("connected to the server...\n"); 
	}

	run_profiles_system(sockfd); 

	// close the socket 
	close(sockfd);
} 

// Show the list of operations.
void show_operations() {
	printf("\n1 - Save new profile\n");
	printf("2 - Add professional experience\n");
	printf("3 - Filter profiles by course\n");
	printf("4 - Filter profiles by skill\n");
	printf("5 - Filter profiles by graduation year\n");
	printf("6 - List all profiles\n");
	printf("7 - Get profile by email\n");
	printf("8 - Delete profile by email\n\n");
	printf("Select a command to continue: ");
}

void run_profiles_system(int sockfd) { 
	int n;
	char buff[MAX]; 

	while (1) { 
		bzero(buff, sizeof(buff));

		show_operations();
		scanf(" %[^\n]s", buff);

		// Send command to server
		write(sockfd, buff, sizeof(buff)); 

		// POST: Create new profile
		if ((strncmp(buff, "1", 1)) == 0) {
			bzero(buff, sizeof(buff)); 
			cJSON *new_profle = create_new_profile();
			strcpy(buff, cJSON_Print(new_profle));
			cJSON_Delete(new_profle);
			write(sockfd, buff, sizeof(buff)); 
			read(sockfd,buff,sizeof(buff));

			printf("\n-------------------------------------------------------------\n\n");
			printf("%s\n", buff);
			printf("\n-------------------------------------------------------------\n");
		}

		// POST: Add professional experience to a profile
		if ((strncmp(buff, "2", 1)) == 0) {
			bzero(buff, sizeof(buff));

			char input[256];
			cJSON *body = cJSON_CreateObject(); // body contains email and professional experience.

			// Get email
			printf("Email: ");
			scanf(" %[^\n]s", input);
			cJSON_AddStringToObject(body, "email", input);

			// Get professional experience
			printf("Professional experience: ");
			scanf(" %[^\n]s", input);
			cJSON_AddStringToObject(body, "professional_experience", input);

			strcpy(buff, cJSON_Print(body));
			write(sockfd, buff, sizeof(buff)); 
			read(sockfd,buff,sizeof(buff));

			printf("\n-------------------------------------------------------------\n\n");
			printf("%s\n", buff);
			printf("\n-------------------------------------------------------------\n");
		}
		
		// GET: Filter profiles by course.
		if ((strncmp(buff, "3", 1)) == 0) {
			bzero(buff, sizeof(buff)); 
			printf("Course: ");
			scanf(" %[^\n]s", buff);
			write(sockfd, buff, sizeof(buff)); 
			read(sockfd,buff,sizeof(buff));

			cJSON *result = cJSON_Parse(buff);
			print_email_and_name(result);
		}
		
		// GET: Filter profile by skill
		if ((strncmp(buff, "4", 1)) == 0){
			bzero(buff, sizeof(buff)); 
			printf("Skill: ");
			scanf(" %[^\n]s", buff);
			write(sockfd, buff, sizeof(buff)); 
			read(sockfd,buff,sizeof(buff));

			cJSON *result = cJSON_Parse(buff);
			print_email_and_name(result);
		}
		
		// GET: Filter profiles by graduation year
		if ((strncmp(buff, "5", 1)) == 0) {
			bzero(buff, sizeof(buff)); 
			printf("Graduation year: ");
			scanf(" %[^\n]s", buff);

			if (strlen(buff) != 4) {
				printf("Invalid year.\n");
			} else {
				write(sockfd, buff, sizeof(buff)); 
				read(sockfd, buff, sizeof(buff));
				cJSON *profile_list = cJSON_Parse(buff);
				print_email_name_and_course(profile_list);
			}
		}

		// GET: Get all profiles
		else if ((strncmp(buff, "6", 1)) == 0){
			read(sockfd,buff,sizeof(buff));

			if((strncmp(buff,"empty",4)) == 0)
			{
				printf("Profile list is empty");
			} else {
				cJSON *profiles = cJSON_Parse(buff);
				print_profile_list(profiles);
			}
		}

		// GET: Get profile by email
		if ((strncmp(buff, "7", 1)) == 0){
			bzero(buff, sizeof(buff));
			printf("Enter the profile email: ");
			scanf(" %[^\n]s", buff);
			write(sockfd, buff, sizeof(buff)); 
			read(sockfd,buff,sizeof(buff));

			printf("\n-------------------------------------------------------------\n\n");

			if((strncmp(buff,"Profile not found.", 18)) == 0) {
				printf("Profile not found.\n");
			} else {
				cJSON *profile = cJSON_Parse(buff);
				print_profile(profile);
			}

			printf("\n-------------------------------------------------------------\n");
		}
		
		// DELETE: Delete profile by email
		if ((strncmp(buff, "8", 1)) == 0) {
			bzero(buff, sizeof(buff));

			printf("Enter the profile email: ");
			scanf(" %[^\n]s", buff);
			write(sockfd, buff, sizeof(buff));

			read(sockfd, buff, sizeof(buff));

			printf("\n-------------------------------------------------------------\n\n");
			printf("%s\n", buff);
			printf("\n-------------------------------------------------------------\n");
		}
	} 
}

cJSON *create_new_profile(void) {
	char input[256];
	cJSON *profile = cJSON_CreateObject();

    printf("E-mail: ");
    scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "email", input);

    printf("First name: ");
	scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "first_name", input);

    printf("Last name: ");
    scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "last_name", input);

    printf("City: ");
	scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "city", input);

	printf("Academic background: ");
	scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "academic_background", input);

	printf("Graduation year: ");
	scanf(" %[^\n]s", input);
	cJSON_AddStringToObject(profile, "graduation_year", input);

	int i = 0;
	char option[1];
	char skills[5][128] = {"", "", "", "", ""};

	while(i < 5) {
		if (i == 0) {
			printf("\nWould you like to add some skill?\n"); 
		} else {
			printf("\nWould you like to add another skill?\n");
		}

		printf("1 - Add skill\n");
		printf("2 - Skip\n");
		printf("Select an option: ");

		scanf(" %[^\n]c", option);

		if ((strncmp(option, "1", 1)) == 0) {
			printf("Enter the description of the skill: ");
			scanf(" %[^\n]s", skills[i]);
			i++;
			if (i == 5) { break; }
		} else if ((strncmp(option, "2", 1)) == 0){
			break;
		}
	}

	cJSON *skills_json = NULL;
    skills_json = cJSON_CreateArray();
	cJSON *skill = NULL;

    cJSON_AddItemToObject(profile, "skills", skills_json);

    for (int index = 0; index < 5; ++index) {
		if (i > 0 && strlen(skills[index]) > 0) {
			skill = cJSON_CreateObject();
			cJSON_AddItemToArray(skills_json, skill);
			cJSON_AddItemToObject(skill, "description", cJSON_CreateString(skills[index]));
		}
    }

	i = 0;
	char experiences[5][128] = {"", "", "", "", ""};

	while(i < 5) {
		if (i == 0) {
			printf("\nWould you like to add some experience?\n");
		} else {
			printf("\nWould you like to add another experience?\n");
		}

		printf("1 - Add experience\n");
		printf("2 - Skip\n");
		printf("Select an option: ");

		scanf(" %[^\n]c", option);

		if ((strncmp(option, "1", 1)) == 0){
			printf("Enter the description of the experience: ");
			scanf(" %[^\n]s", experiences[i]);
			i++;
			if (i == 5) { break; }
		} else if ((strncmp(option, "2", 1)) == 0){
			break;
		}
	}

	cJSON *experience_array = NULL;
    experience_array = cJSON_CreateArray();
	cJSON *experience = NULL;

    cJSON_AddItemToObject(profile, "experiences", experience_array);

    for (int index = 0; index < 5; ++index) {
		if (i > 0 && strlen(experiences[index]) > 0) {
			experience = cJSON_CreateObject();
			cJSON_AddItemToArray(experience_array, experience);
			cJSON_AddItemToObject(experience, "description", cJSON_CreateString(experiences[index]));
		}
    }

	return profile;
}

void print_profile(cJSON *profile) {
	cJSON *email = cJSON_GetObjectItem(profile, "email");
	printf("Email: %s\n", email->valuestring);

	cJSON *first_name = cJSON_GetObjectItem(profile, "first_name");
	cJSON *last_name = cJSON_GetObjectItem(profile, "last_name");
	printf("Nome: %s Sobrenome: %s\n", first_name->valuestring, last_name->valuestring);

	cJSON *city = cJSON_GetObjectItem(profile, "city");
	printf("Residência:: %s\n", city->valuestring);

	cJSON *academic_background = cJSON_GetObjectItem(profile, "academic_background");
	printf("Formação Acadêmica: %s\n", academic_background->valuestring);

	cJSON *graduation_year = cJSON_GetObjectItem(profile, "graduation_year");
	printf("Ano de Formatura:: %s\n", graduation_year->valuestring);

	cJSON *skill_list = NULL;
	skill_list = cJSON_GetObjectItem(profile, "skills");

	if (cJSON_GetArraySize(skill_list) > 0) {
		printf("Habilidades: ");
		cJSON *skill = NULL;
		cJSON_ArrayForEach(skill, skill_list) {
			cJSON *description = cJSON_GetObjectItem(skill, "description");
			printf("%s, ", description->valuestring);
		}
	}

	cJSON *experience_list = NULL;
	experience_list = cJSON_GetObjectItem(profile, "experiences");

	if (cJSON_GetArraySize(experience_list) > 0) {
		printf("\nExperiência:\n");
		cJSON *experience = NULL;
		int count = 1;
		cJSON_ArrayForEach(experience, experience_list) {
			cJSON *description = cJSON_GetObjectItem(experience, "description");
			printf("\t(%d) %s\n", count, description->valuestring);
			count++;
		}
	}
}

void print_profile_list(cJSON *profile_list) {
	printf("\n-------------------------------------------------------------\n");

	if (cJSON_GetArraySize(profile_list) > 0) {
		printf("\n");

		cJSON *profile = NULL;

		cJSON_ArrayForEach(profile, profile_list) {
			print_profile(profile);
			printf("\n");
		}

		printf("-------------------------------------------------------------\n");
	} else {
		printf("\nResult is empty.\n");
		printf("\n-------------------------------------------------------------\n");
	}
}

void print_email_name_and_course(cJSON *profile_list) {
	printf("\n-------------------------------------------------------------\n");

	if (cJSON_GetArraySize(profile_list) > 0) {
		printf("\n");

		cJSON *profile = NULL;
		cJSON_ArrayForEach(profile, profile_list) {
			cJSON *email = cJSON_GetObjectItem(profile, "email");
			printf("Email: %s\n", email->valuestring);

			cJSON *first_name = cJSON_GetObjectItem(profile, "first_name");
			cJSON *last_name = cJSON_GetObjectItem(profile, "last_name");
			printf("Nome: %s Sobrenome: %s\n", first_name->valuestring, last_name->valuestring);

			cJSON *academic_background = cJSON_GetObjectItem(profile, "academic_background");
			printf("Formação Acadêmica: %s\n", academic_background->valuestring);

			printf("\n");
		}

		printf("-------------------------------------------------------------\n");
	} else {
		printf("\nResult is empty.\n");
		printf("\n-------------------------------------------------------------\n");
	}
}

void print_email_and_name(cJSON *result) {
	printf("\n-------------------------------------------------------------\n");
	if (cJSON_GetArraySize(result) > 0) {
		printf("\n");

		cJSON *profile = NULL;
		cJSON_ArrayForEach(profile, result) {
			cJSON *email = cJSON_GetObjectItem(profile, "email");
			printf("Email: %s\n", email->valuestring);

			cJSON *first_name = cJSON_GetObjectItem(profile, "first_name");
			cJSON *last_name = cJSON_GetObjectItem(profile, "last_name");
			printf("Nome: %s Sobrenome: %s\n", first_name->valuestring, last_name->valuestring);
			printf("\n");
		}

		printf("-------------------------------------------------------------\n");
	} else {
		printf("\nResult is empty.\n");
		printf("\n-------------------------------------------------------------\n");
	}
}