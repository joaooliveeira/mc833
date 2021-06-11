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

void show_operations();
cJSON *create_new_profile(void);
void print_profile_list(cJSON *profiles);
void print_email_name_and_course(cJSON *profile_list);
void print_email_and_name(cJSON *profile_list);
void print_profile(cJSON *profile);

int main() { 
	int sockfd;
    struct sockaddr_in servaddr;
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(0);
    } else {
		printf("socket created successfully...\n");
	}

	bzero(&servaddr, sizeof(servaddr));
  
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
      
    int n, len, nready;
	char send_msg[MAX], recv_msg[MAX];

	struct timeval tv;
	// wait until either socket has data ready to be recv()d (timeout 3.0 secs)
	tv.tv_sec = 3;
	tv.tv_usec = 0;

	fd_set rset;
	// clear the descriptor set
    FD_ZERO(&rset);

	while (1) {
		// set sockfd in readset
		FD_SET(sockfd, &rset);

		bzero(send_msg, MAX);
		bzero(recv_msg, MAX);

		show_operations();
		scanf(" %[^\n]s", send_msg);

		// POST: Create new profile
		if ((strncmp(send_msg, "1", 1)) == 0) {
			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "1");

			cJSON *new_profle = create_new_profile();
			strcpy(send_msg, cJSON_Print(new_profle));
			cJSON_Delete(new_profle);

			cJSON_AddStringToObject(request, "body", send_msg);

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				printf("\n-------------------------------------------------------------\n\n");
				printf("%s\n", recv_msg);
				printf("\n-------------------------------------------------------------\n");
			}
		}

		// POST: Add professional experience to a profile
		if ((strncmp(send_msg, "2", 1)) == 0) {
			char input[256];

			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "2");

			cJSON *body = cJSON_CreateObject(); // body contains email and professional experience.

			// Get email
			printf("Email: ");
			scanf(" %[^\n]s", input);
			cJSON_AddStringToObject(body, "email", input);

			// Get professional experience
			printf("Professional experience: ");
			scanf(" %[^\n]s", input);
			cJSON_AddStringToObject(body, "professional_experience", input);

			strcpy(send_msg, cJSON_Print(body));
			cJSON_AddStringToObject(request, "body", cJSON_Print(body));

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			// Create datagram message
			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				printf("\n-------------------------------------------------------------\n\n");
				printf("%s\n", recv_msg);
				printf("\n-------------------------------------------------------------\n");
			}
		}
		
		// GET: Filter profiles by course.
		if ((strncmp(send_msg, "3", 1)) == 0) {
			bzero(send_msg, sizeof(send_msg)); 
			printf("Course: ");
			scanf(" %[^\n]s", send_msg);

			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "3");
			cJSON_AddStringToObject(request, "body", send_msg);

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				cJSON *result = cJSON_Parse(recv_msg);
				print_email_and_name(result);
			}
		}
		
		// GET: Filter profile by skill
		if ((strncmp(send_msg, "4", 1)) == 0){
			bzero(send_msg, sizeof(send_msg)); 
			printf("Skill: ");
			scanf(" %[^\n]s", send_msg);

			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "4");
			cJSON_AddStringToObject(request, "body", send_msg);

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				cJSON *result = cJSON_Parse(recv_msg);
				print_email_and_name(result);
			}
		}
		
		// GET: Filter profiles by graduation year
		if ((strncmp(send_msg, "5", 1)) == 0) {
			bzero(send_msg, sizeof(send_msg)); 
			printf("Graduation year: ");
			scanf(" %[^\n]s", send_msg);

			if (strlen(send_msg) != 4) {
				printf("Invalid year.\n");
			} else {
				// Create request with operation and body
				cJSON *request = cJSON_CreateObject();
				cJSON_AddStringToObject(request, "operation", "5");
				cJSON_AddStringToObject(request, "body", send_msg);

				// Create datagram message
				bzero(send_msg, sizeof(send_msg));
				strcpy(send_msg, cJSON_Print(request));

				sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

				nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
				if (nready == 0) 
				{
					printf("Timeout occurred!  No data after 3.0 seconds.\n");
				} else {
					recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

					cJSON *profile_list = cJSON_Parse(recv_msg);
					print_email_name_and_course(profile_list);
				}
			}
		}

		// GET: Get all profiles
		else if ((strncmp(send_msg, "6", 1)) == 0) {
			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "6");

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);
				cJSON *profiles = cJSON_Parse(recv_msg);
				print_profile_list(profiles);
			}
		}

		// GET: Get profile by email
		if ((strncmp(send_msg, "7", 1)) == 0){
			bzero(send_msg, sizeof(send_msg));
			printf("Enter the profile email: ");
			scanf(" %[^\n]s", send_msg);

			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "7");
			cJSON_AddStringToObject(request, "body", send_msg);

			// Create datagram message
			bzero(send_msg, sizeof(send_msg));
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				printf("\n-------------------------------------------------------------\n\n");
				if((strncmp(recv_msg, "Profile not found.", 18)) == 0) {
					printf("Profile not found.\n");
				} else {
					cJSON *profile = cJSON_Parse(recv_msg);
					print_profile(profile);
				}

				printf("\n-------------------------------------------------------------\n");
			}
		}
		
		// DELETE: Delete profile by email
		if ((strncmp(send_msg, "8", 1)) == 0) {
			printf("Enter the profile email: ");
			scanf(" %[^\n]s", send_msg);

			// Create request with operation and body
			cJSON *request = cJSON_CreateObject();
			cJSON_AddStringToObject(request, "operation", "8");
			cJSON_AddStringToObject(request, "body", send_msg);

			// Create datagram message
			bzero(send_msg, MAX);
			strcpy(send_msg, cJSON_Print(request));

			sendto(sockfd, (const char *)send_msg, strlen(send_msg), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));

			nready = select(sockfd + 1, &rset, NULL, NULL, &tv);
			if (nready == 0) 
			{
				printf("Timeout occurred!  No data after 3.0 seconds.\n");
			} else {
				bzero(recv_msg, MAX);
				recvfrom(sockfd, (char *)recv_msg, MAX, 0, (struct sockaddr *) &servaddr, &len);

				printf("\n-------------------------------------------------------------\n\n");
				printf("%s\n", recv_msg);
				printf("\n-------------------------------------------------------------\n");
			}
		}

	} 

	return 0;
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