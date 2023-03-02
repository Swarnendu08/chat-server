

#include                        <bits/stdc++.h>
#include                        <stdlib.h>
#include                        <unistd.h>
#include                        <string.h>
#include                        <sys/types.h>
#include                        <sys/socket.h>
#include                        <arpa/inet.h>
#include                        <netinet/in.h>

using namespace std;

#define CONNECTION_IP           "192.168.0.103"
#define CONNECTION_PORT         5500
#define STRING_LENGTH		1024
char SERVER_KEY[10] = "";
map<string, sockaddr_in>  active_user_list;


inline bool startswith(const char* str, const char* prefix){
	if( strncmp(str, prefix, sizeof(prefix)) == 0 )
		return 1;
	return 0;
}	

inline void			spf_new_login(char client_message[], int len = STRING_LENGTH){
	string USER_ID = "";
	for(int i=11; i<len; i++){
		if( client_message[i] == ' ' || client_message[i] == '\n' || client_message[i] == char(0))
			break;
		USER_ID.push_back( client_message[i] );
	}
	cout<<USER_ID<<'\n';
}

int main( int argc, char** argv ){
	string sk = string(argv[1]);
	cout<<sk<<'\n';
	const char* charArray = sk.c_str();
	cout<< charArray <<'\n';

	cout<< sk.length() <<' '<< strlen(charArray)<<'\n';

	char str[1024];
	//memset( str, 0, sizeof(str));
	//fgets(str, sizeof(str), stdin);
	//if( startswith(str, "/new_login"))
		//spf_new_login(str);
	return 0;
}
