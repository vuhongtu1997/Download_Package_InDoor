//============================================================================
// Name        : download_package.cpp
// Author      : rd
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <list>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <json-c/json.h>
#include <cstddef>

#define DOWN_LOAD_PACKAGE "DOWN_LOAD_PACKAGE"
#define CHECK_TIME "CHECK_TIME"
#define PORT 8181

using namespace std;

list<string> wifi_data;
list<string> rsp_wifi_data;

string msg_rsp;
string ip;
string mac;

bool flag_encryption = FALSE;

const char *PASSWORD,  *ENCRYPTION;
const char * SSID;

char* stringToChar(string s){
	char *sendChar = new char[s.length() + 1];
	strcpy(sendChar, s.c_str());
	return sendChar;
}

void getMacAddress(char *uc_Mac){
	struct ifreq s;
	unsigned char *mac = NULL;
	int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

	strcpy(s.ifr_name, "eth0");
	if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
		mac = (unsigned char*) s.ifr_addr.sa_data;
	}
	sprintf((char*) uc_Mac, (const char*) "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
			mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	puts(uc_Mac);
}

int findsubstr(const char *str, char *sub){
	const char *p = str;
	unsigned int len = strlen(sub);
	while(*p != NULL){
		if (strlen(p) >= len){
			if (strncmp(p, sub, strlen(sub)) == 0){
				return (p - str);
			}
		}
		else{
			break;
		}
		p++;
	}
	return -1;
}

int ping(){
	msg_rsp = "";
	char done[] = "0% packet loss";
	char msg_line[255]={0};
	int c;
	FILE *file;
	file = popen("ping -c3 www.google.com", "r");
	if(file == NULL){
		exit(1);
	}
	fgets(msg_line, 100, file);
	msg_rsp += msg_line;
	while((c = fgetc(file)) != EOF){
		fgets(msg_line, 100, file);
		if(feof(file)){
			break;
		}
		msg_rsp += msg_line;
	}
	pclose(file);
	if(findsubstr(stringToChar(msg_rsp),done) >= 0){
		cout<<msg_rsp<<endl;
		FILE *file1;
		file1 = popen("hwclock -w", "r");
		pclose(file1);
		FILE *file2;
		msg_rsp = "";
		file2 = popen("/etc/download_package.sh", "r");
		if(file2 == NULL){
			exit(1);
		}
		fgets(msg_line, 100, file2);
		msg_rsp += msg_line;
		while((c = fgetc(file2)) != EOF){
			fgets(msg_line, 100, file2);
			if (feof(file2)){
				break;
			}
			msg_rsp += msg_line;
		}
		cout<<msg_rsp<<endl;
		pclose(file2);
		FILE *file3;
		msg_rsp = "";
		file3 = popen("/etc/check_download", "r");
		if(file3 == NULL){
			exit(1);
		}
		fgets(msg_line, 100, file3);
		msg_rsp += msg_line;
		while((c = fgetc(file3)) != EOF){
			fgets(msg_line, 100, file3);
			if (feof(file3)){
				break;
			}
			msg_rsp += msg_line;
		}
		cout<<msg_rsp<<endl;
		pclose(file3);
		if(findsubstr(stringToChar(msg_rsp),"download done") >= 0){
			FILE *file4;
			file4 = popen("rm /etc/check_download", "r");
			file4 = popen("rm /etc/download_package.sh", "r");
			pclose(file4);
			return 1;
		}
		else
			return 0;
	}
	else{
		return 0;
	}
}

int main(){
	struct json_object *jobj;
	int sock, ret, count;
	fd_set readfd;
	unsigned int addr_len;
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0){
		perror("socket error\n");
		return -1;
	}
	addr_len = sizeof(struct sockaddr_in);
	memset((void*) &server_addr, 0, addr_len);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(PORT);
	ret = bind(sock, (struct sockaddr*) &server_addr, addr_len);
	if (ret < 0){
		perror("bind error\n");
		return -1;
	}
	while(1){
		FD_ZERO(&readfd);
		FD_SET(sock, &readfd);
		ret = select(sock + 1, &readfd, NULL, NULL, 0);
		if (ret > 0){
			if (FD_ISSET(sock, &readfd)){
				char buffer[1024] = {0};
				count = recvfrom(sock, buffer, 1024, 0, (struct sockaddr*) &client_addr, &addr_len);
				puts(buffer);
				jobj = json_tokener_parse(buffer);
				enum json_type type;
				const char *cmd;
				json_object_object_foreach(jobj, key, val){
					type = json_object_get_type(val);
					switch (type){
						case json_type_string:
							if(strcmp(key,"CMD") == 0){
								cmd = json_object_get_string(json_object_object_get(jobj,"CMD"));
								if( strcmp(cmd,DOWN_LOAD_PACKAGE) == 0){
									int stt = ping();
									if(stt == 1){
										struct json_object * object;
										object = json_object_new_object();
										json_object_object_add(object, "CMD", json_object_new_string("HC_RESPONE"));
										json_object_object_add(object, "STT", json_object_new_string("DOWNLOAD_SUCCESS"));
										const char * rsp;
										rsp = json_object_to_json_string(object);
										count = sendto(sock, rsp, strlen(rsp), 0, (struct sockaddr*) &client_addr, addr_len);
									}
									else{
										struct json_object * object;
										object = json_object_new_object();
										json_object_object_add(object, "CMD", json_object_new_string("HC_RESPONE"));
										json_object_object_add(object, "STT", json_object_new_string("DOWNLOAD_FAULT"));
										const char * rsp;
										rsp = json_object_to_json_string(object);
										count = sendto(sock, rsp, strlen(rsp), 0, (struct sockaddr*) &client_addr, addr_len);
									}
								}
								else if( strcmp(cmd,CHECK_TIME) == 0){
									string msg_rsp_read_time;
									char mac[20] = { 0 };
									getMacAddress(mac);
									char msg_line_read_time[64] = {0};
									int item;
									FILE *read_time;
									read_time = popen("hwclock -r", "r");
									if (read_time == NULL){
										exit(1);
									}
									fgets(msg_line_read_time, 64, read_time);
									msg_rsp_read_time += msg_line_read_time;
									while ((item = fgetc(read_time)) != EOF){
										fgets(msg_line_read_time, 64, read_time);
										if (feof(read_time)){
											break;
										}
										msg_rsp_read_time += msg_line_read_time;
									}
									pclose(read_time);
									cout << msg_rsp_read_time << endl;
									struct json_object * object;
									object = json_object_new_object();
									json_object_object_add(object, "CMD", json_object_new_string("HC_RESPONE"));
									json_object_object_add(object, "TIME", json_object_new_string(stringToChar(msg_rsp_read_time)));
									json_object_object_add(object, "MAC", json_object_new_string(stringToChar(mac)));
									const char * rsp;
									rsp = json_object_to_json_string(object);
									count = sendto(sock, rsp, strlen(rsp), 0, (struct sockaddr*) &client_addr, addr_len);
								}
							}
						break;
					}
				}
			}
		}
	}
	return 0;
}
