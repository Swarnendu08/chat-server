
//----------------------------------------SERVER----------------------------------------//



//------------------------------------HEADER FILELS-------------------------------------//


#include 			<bits/stdc++.h>
#include 			<stdlib.h>
#include 			<unistd.h>
#include 			<string.h>
#include			<sys/types.h>
#include			<sys/socket.h>
#include			<arpa/inet.h>
#include			<netinet/in.h>
#include			<chrono>
#include			<thread>


//------------------------------------HEADER FILELS-------------------------------------//


using namespace std;


//----------------------------------------VALUES----------------------------------------//


#define 			LOCAL_CONNECTION_IP			"192.168.0.103"
#define				PUBLIC_CONNECTION_IP			INADDR_ANY
#define 			CONNECTION_PORT				50005
#define 			STRING_LENGTH				1024
#define				USING_LOACL_IP				1



string 				SERVER_KEY =				"";
map<string, sockaddr_in> 	ACTIVE_USERS;
int 				socket_descriptor = 			0;
queue<string> 			RECEIVED_MESSAGE;
string 				SEND_MESSAGE =				"";
int				READ_SIZE = 				0;
vector<string>			PREFIX;



struct  			sockaddr_in 				server_info;
struct 				sockaddr_in				client_info;
socklen_t 			len = 				sizeof( client_info );
char 				server_message[STRING_LENGTH] = 	{0};
char				client_message[STRING_LENGTH] = 	{0};


//----------------------------------------VALUES----------------------------------------//



//---------------------------------------FUNCTIONS--------------------------------------//


inline void 			spf_help( int parm );
inline void			spf_socket_descriptor();
inline void			spf_server_info();
inline void 			spf_binding();
inline void 			spf_communication();
inline void 			spf_message_recv();
inline void			spf_message_send(struct sockaddr_in &dest_host, string &content_s);

inline void 			spf_set_prefixes();
inline void 			spf_processing_received_message();
inline void 			spf_segment_message();
inline void 			spf_login();
inline void 			spf_client_to_client_messaging();
inline void 			spf_logout(); // feature: /logout
inline void 			spf_close_server(bool &feedback); // feature: /close_server


//---------------------------------------FUNCTIONS--------------------------------------//



//------------------------------------MAIN FUNCTION-------------------------------------//


int main( int argc, char** argv ){ 	// argc: argument count, argv: argument values

	// Assigning Server Key
	if( argc <= 1){
		spf_help(1);
		perror( "Please Provide Server Key" );
		exit( EXIT_FAILURE );
	}
	SERVER_KEY = string( argv[1] );


	// Socket Descriptor
	spf_socket_descriptor();


	// Server Information
	spf_server_info();


	// Binding
	spf_binding();


	// Set Prefixes
	spf_set_prefixes();


	// Printing Server Info
	if( USING_LOACL_IP )
		printf( "Server Listening @ Local IP: [ %s ]  and Port: [ %d ].\n", LOCAL_CONNECTION_IP , CONNECTION_PORT );
	else
		printf( "Server Listening @ Public IP: [ %s ]  and Port: [ %d ].\n", "0.0.0.0" , CONNECTION_PORT );

	// Communication
	spf_communication();


	// Server Closing
	close(socket_descriptor);
	return 0;

}


//------------------------------------MAIN FUNCTION-------------------------------------//



//-----------------------------------------HELP-----------------------------------------//


inline void                     spf_help( int parm = 0 ){

	if( parm == 0 || parm == 1 ){
		puts( "Syntax for initiating server: [ ./server.out <Server Key> ]");
	}


}


//-----------------------------------------HELP-----------------------------------------//



//-----------------------------------SOCKET DESCRIPTOR----------------------------------//


inline void                      spf_socket_descriptor(){

	socket_descriptor = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( socket_descriptor < 0 ){
		perror( "Socket Creation Failed" );
		exit( EXIT_FAILURE );
	}
	else
		puts( "Socket Creation Successful" );

}


//-----------------------------------SOCKET DESCRIPTOR----------------------------------//



//--------------------------------------SERVER INFO-------------------------------------//


inline void                     spf_server_info(){
	memset( &server_info, 0, sizeof( server_info ) );

	server_info.sin_family		=	AF_INET; 			// IPv4

	if(USING_LOACL_IP)
		server_info.sin_addr.s_addr     =       inet_addr( LOCAL_CONNECTION_IP );	// INADDR_ANY
	else
		server_info.sin_addr.s_addr     =       htonl( PUBLIC_CONNECTION_IP );	// INADDR_ANY

	server_info.sin_port		=	htons( CONNECTION_PORT );
}


//--------------------------------------SERVER INFO-------------------------------------//



//----------------------------------------BINDING---------------------------------------//


inline void                     spf_binding(){
	int feedback = bind( socket_descriptor, (struct sockaddr*)&server_info, sizeof(server_info) );
	if(feedback < 0){
		perror( "Binding Failed" );
		exit( EXIT_FAILURE );
	}
	else
		puts( "Binding Successful" );
}


//----------------------------------------BINDING---------------------------------------//


//-------------------------------------MESSAGE SEND-------------------------------------//

inline void			spf_message_send(struct sockaddr_in &dest_host, string &content_s){

	int send_feedback = sendto(
								socket_descriptor,
								content_s.c_str(),
								content_s.length(),
								MSG_CONFIRM,
								(struct sockaddr*)&dest_host,
								sizeof(dest_host)
							);
	if( send_feedback < 0)
		perror("Sending Failed");
}




//------------------------------------MESSAGE RECEIVE-----------------------------------//


inline void 		spf_message_recv(){

	// Clear junk
	memset( &client_info, 0, sizeof(client_info) );
	memset( server_message, 0, sizeof(server_message));
	memset( client_message, 0, sizeof(client_message));

	// read for message
	READ_SIZE = recvfrom(
								socket_descriptor,
								(char*)client_message,
								sizeof(client_message),
								MSG_WAITALL,
								(struct sockaddr*)&client_info,
								&len
							);
	if( READ_SIZE > 0)
		spf_segment_message();
}


//------------------------------------MESSAGE RECEIVE-----------------------------------//



//-------------------------------------SET PREFIXES-------------------------------------//


inline void 		spf_set_prefixes(){

	// clean the vector
	PREFIX.clear();

	// set Prefixes
	PREFIX.push_back("/help");			// Index 0: help
	PREFIX.push_back("/keepalive");			// Index 1: keepalive
	PREFIX.push_back("/login"); 			// Index 2: Client logged in
	PREFIX.push_back("/active_user");		// Index 3: active users
	PREFIX.push_back("/message");			// Index 4: send message
	PREFIX.push_back("/logout");			// Index 5: client logging out
	PREFIX.push_back("/close_server");		// Index 6: for server closing
}


//-------------------------------------SET PREFIXES-------------------------------------//



//-------------------------------------COMMUNICATION------------------------------------//


inline void                     spf_communication(){
	while(true){

		// receives and process the message
		spf_message_recv();


		// if error reading message
		if( READ_SIZE < 0){

			puts("Error Reading Message");

			// wait for 50 millisecond
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			continue;
		}
		else if( READ_SIZE == 0){

			puts("Null Read Message");

			// wait for 50 millisecond
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			continue;
		}


		// here we figure out what the message contains

		// PREFIX[2] = "/login"
		if( (RECEIVED_MESSAGE.front()).find(PREFIX[2]) == 0 ){

			RECEIVED_MESSAGE.pop();
			spf_login();
		}

		// PREFIX[4] = "/message"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[4]) == 0){

			//format: /message <sender ID> <receiver ID> message
			RECEIVED_MESSAGE.pop();
			spf_client_to_client_messaging();
		}

		// PREFIX[5] = "/logout"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[5]) == 0 ){

			// RECEIVED_MESSAGE = /logout <user_id>
			spf_logout();
		}

		// PREFIX[6] = ""/close_server"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[6]) == 0 ){

			// RECEIVED_MESSAGE = /close_server <user_id> <server_key>
			bool feedback = false;
			spf_close_server(feedback);

			if( feedback ){
				puts( "Server is closing..." );

				// wait 5 seconds
				std::this_thread::sleep_for(std::chrono::seconds(5));
				break;
			}
		}
	}
}


//-------------------------------------COMMUNICATION------------------------------------//



//------------------------------------SEGMENTATION-------------------------------------//


inline void			spf_segment_message(){

	// clean the queue
	while( RECEIVED_MESSAGE.size() ){

		RECEIVED_MESSAGE.pop();
	}

	// assigning new values
	string word = "";
	for(int i=0; i<sizeof(client_message); i++){

		if( (i == sizeof(client_message) -1) || (client_message[i] == ' ') || (client_message[i] == '\n') || (client_message[i] == 0)){

			RECEIVED_MESSAGE.push( word );
			word = "";

			if( (client_message[i] == '\n') || (client_message[i] == 0) )
				break;
		}
		else{
			word.push_back( client_message[i] );
		}
	}
	if( word != ""){
		RECEIVED_MESSAGE.push( word );
	}
}


//------------------------------------SEGMENTATION-------------------------------------//



//---------------------------------------NEW LOGIN-------------------------------------//


inline void			spf_login(){
	// Reply Host
	struct sockaddr_in reply_host = client_info;

	// Get the User ID
	string USER_ID = RECEIVED_MESSAGE.front();
	RECEIVED_MESSAGE.pop();

	// Broadcast message to everyone active
	string content_s = "/login " + USER_ID + "\n";
	string reply_msg = "";

	for(auto const&[user_id, user_addr] : ACTIVE_USERS){

		struct sockaddr_in dest_host = user_addr;
		spf_message_send(dest_host, content_s);

		// Message to reply host
		reply_msg = "/active_user " + user_id + "\n";

		spf_message_send(reply_host, reply_msg);
	}

	reply_msg = "/end\n";
	spf_message_send(reply_host, reply_msg);


	// Register Client Info in the Map
	ACTIVE_USERS[ USER_ID ] = reply_host;

	reply_msg = "User ID: [ " + USER_ID+ " ] logged in";
	puts( reply_msg.c_str() );
}


//---------------------------------------NEW LOGIN-------------------------------------//

//-------------------------------CLIENT TO CLIENT MESSAGING-----------------------------//

inline void 		spf_client_to_client_messaging(){

	//format: <sender id> <receiver id> message
	string sender_id = RECEIVED_MESSAGE.front();
	RECEIVED_MESSAGE.pop();


	//format: <receiver id> message
	string receiver_id = RECEIVED_MESSAGE.front();
	RECEIVED_MESSAGE.pop();

	string feedback = "/message from [ " + sender_id +" ] to [ " + receiver_id + " ] : ";

	//format: message
	// check receiver_id active
	if(ACTIVE_USERS.find(receiver_id) == ACTIVE_USERS.end()){

		struct sockaddr_in dest_host = ACTIVE_USERS[sender_id];

		// server replys with "client inactive"
		string reply_msg = "/message server " + sender_id + " " + receiver_id + " is not active";

		spf_message_send(dest_host, reply_msg);

		feedback.append("FAILED");
	}
	else{

		struct sockaddr_in dest_host = ACTIVE_USERS[receiver_id];

		// message to be sent
		string reply_msg = "/message " + sender_id + " " + receiver_id;
		while( RECEIVED_MESSAGE.size() ){

			reply_msg.append( " " + RECEIVED_MESSAGE.front() );
			RECEIVED_MESSAGE.pop();
		}

		reply_msg.push_back('\n');

		spf_message_send(dest_host, reply_msg);

		feedback.append("SUCCESS");
	}

	puts( feedback.c_str() );
}


//-------------------------------CLIENT TO CLIENT MESSAGING-----------------------------//



//------------------------------------FEATURE LOGOUT------------------------------------//


inline void 			spf_logout(){

	// PREFIX[5] = "/logout"
	// RECEIVED_MESSAGE = /logout <user_id>
	string msg_broadcast = "", user_lout = "";

	// build the broadcast message
	while( RECEIVED_MESSAGE.size() ){

		msg_broadcast.append(RECEIVED_MESSAGE.front());
		user_lout = RECEIVED_MESSAGE.front();
		RECEIVED_MESSAGE.pop();
		if(RECEIVED_MESSAGE.size())
			msg_broadcast.push_back(' ');
	}
	msg_broadcast.push_back('\n');

	// broadcast the message to everone
	for(auto const&[user_id, user_addr] : ACTIVE_USERS){

		// send message
		struct sockaddr_in dest_host = user_addr;
		spf_message_send( dest_host, msg_broadcast);
	}

	if( ACTIVE_USERS.find(user_lout) != ACTIVE_USERS.end())
		ACTIVE_USERS.erase(user_lout);

	user_lout = "User [ " + user_lout + " ] is logged out.";
	puts( user_lout.c_str() );
}


//------------------------------------FEATURE LOGOUT------------------------------------//



//---------------------------------FEATURE SERVER CLOSE---------------------------------//


inline void 			spf_close_server(bool &feedback){

	// RECEIVED_MESSAGE: /close_server <sender_id> <server_key>
	RECEIVED_MESSAGE.pop();

	// store sender_id
	// RECEIVED_MESSAGE: <sender_id> <server_key>
	string sender_id = RECEIVED_MESSAGE.front();

	// RECEIVED_MESSAGE: <server_key>
	RECEIVED_MESSAGE.pop();


	// check wheather server_key matches
	if( RECEIVED_MESSAGE.front() == SERVER_KEY ){

		// user verified
		string msg = PREFIX[6] + " " + sender_id + "\n";
		// msg = /close_server <sender_id>

		puts(("Server is closing by user: [ " + sender_id + "]").c_str());

		// broadcast the update
		for( auto const&[user_id, user_addr]: ACTIVE_USERS){

			struct sockaddr_in dest_host = user_addr;
			spf_message_send(dest_host, msg);
		}

		feedback = true;
	}
	else{

		// if server_key doesnot matches
		puts(("Failed attempt to close server by user: [ " + sender_id + " ]").c_str());

		// Upcoming feature: Blocking
		// if a particular client takes more than 4 chance to server close
		// he will be blocked
	}
}


//---------------------------------FEATURE SERVER CLOSE---------------------------------//
