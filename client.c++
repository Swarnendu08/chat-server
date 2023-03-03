
//----------------------------------------CLIENT----------------------------------------//



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
#define				PUBLIC_CONNECTION_IP			"14.139.185.120"
#define 			CONNECTION_PORT				50005
#define 			STRING_LENGTH				1024
#define				USING_LOACL_IP				1



string 				USER_ID =				"";
int 				SOCKET_DESCRIPTOR = 			0;
struct sockaddr_in 		SERVER_INFO;
char				SERVER_MESSAGE[STRING_LENGTH];
queue<string> 			RECEIVED_MESSAGE;
char				CLIENT_MESSAGE[STRING_LENGTH];
queue<string> 			SENDING_MESSAGE;
vector<string>			PREFIX;
set<string>			ACTIVE_USERS;
bool 				SERVER_STATUS = 			true; // active
bool				LOGIN_STATUS =				false;

//----------------------------------------VALUES----------------------------------------//



//---------------------------------------FUNCTION---------------------------------------//


inline void			cpf_help(int parm);
inline void			cpf_socket_descriptor();
inline void			cpf_server_info();
inline void			cpf_communication();
inline void			cpf_send_message(string &content_s);
inline void			cpf_recv_message();
inline void			cpf_sending();
inline void			cpf_receiving();

inline void			cpf_set_prefixes();
inline void			cpf_loggin_in();
inline void			cpf_segment_message(char* msg, queue<string> &que);
inline int			cpf_string_to_int(string &str);
inline void 			cpf_build_received_message(); // feature: /message
inline void 			cpf_build_sending_message(); // feature: /message
inline void 			cpf_logout_send(); // feature: /logout
inline void 			cpf_logout_recv(bool &feedback); // feature: /logout
inline void 			cpf_send_close_server(); // feature: /close_server
inline void 			cpf_recv_close_server(); // feature: /close_server

//---------------------------------------FUNCTION---------------------------------------//



//-----------------------------------------MAIN-----------------------------------------//


int main( int argc, char** argv ){

	// Getting User ID
	if( argc <= 1 ){
		cpf_help(1);
		exit( EXIT_FAILURE );
	}

	USER_ID = string( argv[1] );


	// Socket Descriptor
	cpf_socket_descriptor();


	// Server Info
	cpf_server_info();

	// Setting Prefixes
	cpf_set_prefixes();

	// login to server
	cpf_loggin_in();

	// Client Sending Message
	std::thread sending_thread(cpf_sending);

	// Client Receiving Message
	std::thread receiving_thread(cpf_receiving);

	//  wait for both thread to finish
	sending_thread.join();
	receiving_thread.join();



	return 0;
}


//-----------------------------------------MAIN-----------------------------------------//



//-----------------------------------------HELP-----------------------------------------//


inline void			cpf_help(int parm = 0){
	puts( "Nothing is here yet" );
}


//-----------------------------------------HELP-----------------------------------------//



//----------------------------------SOCKET DESCRIPTOR-----------------------------------//


inline void			cpf_socket_descriptor(){

	SOCKET_DESCRIPTOR = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if( SOCKET_DESCRIPTOR < 0 ){
		perror("Error Creating Socket");
		exit( EXIT_FAILURE );
	}
}


//----------------------------------SOCKET DESCRIPTOR-----------------------------------//



//-------------------------------------SERVER INFO--------------------------------------//


inline void			cpf_server_info(){

	memset( &SERVER_INFO, 0, sizeof(SERVER_INFO));

	SERVER_INFO.sin_family		=	AF_INET; 		// IPv4
	if(USING_LOACL_IP)
		SERVER_INFO.sin_addr.s_addr     =       inet_addr( LOCAL_CONNECTION_IP ); // INADDR_ANY
	else
		SERVER_INFO.sin_addr.s_addr     =       inet_addr( PUBLIC_CONNECTION_IP ); // INADDR_ANY

	SERVER_INFO.sin_port		=		htons( CONNECTION_PORT );
}


//-------------------------------------SERVER INFO--------------------------------------//



//------------------------------------RECV MESSAGE--------------------------------------//


inline void			cpf_recv_message(){

	// message received
	memset( SERVER_MESSAGE, 0, sizeof( SERVER_MESSAGE ));
	int read_size = recvfrom(
					SOCKET_DESCRIPTOR,
					(char*)SERVER_MESSAGE,
					sizeof(SERVER_MESSAGE),
					MSG_WAITALL,
					NULL,
					NULL
				);

	// puts(SERVER_MESSAGE);

	// Format the message from char[] to words and store in the queue
	cpf_segment_message(SERVER_MESSAGE, RECEIVED_MESSAGE);

	// clearing the stack
	memset(SERVER_MESSAGE, 0, sizeof(SERVER_MESSAGE));
}


//------------------------------------RECV MESSAGE--------------------------------------//



//------------------------------------SEND MESSAGE--------------------------------------//


inline void			cpf_send_message(string &content_s){

	// check if server is live or not
	// if( !SERVER_STATUS ){ // SERVER_STATUS == 0 -> server inactive
	// 	puts( "Couldn't send message. Server is not LIVE." );
	// 	return;
	// }

	int send_feedback = sendto(
					SOCKET_DESCRIPTOR,
					content_s.c_str(),
					content_s.length(),
					MSG_CONFIRM,
					(struct sockaddr*)&SERVER_INFO,
					sizeof(SERVER_INFO)
				);
	if( send_feedback < 0){
		perror("Sending Failed");
	}
}


//-------------------------------------SEND MESSAGE-------------------------------------//



//-------------------------------------SET PREFIXES-------------------------------------//


inline void			cpf_set_prefixes(){

	// Remove any pre stored values
	PREFIX.clear();

	// Set the prefix values
	PREFIX.push_back("/help"); 			// Index 0 : working of program
	PREFIX.push_back("/keepalive"); 		// Index 1 : keep alive connection
	PREFIX.push_back("/login"); 			// Index 2 : loggin in to server
	PREFIX.push_back("/active_user"); 		// Index 3 : active user id
	PREFIX.push_back("/message"); 			// Index 4 : sending message
	PREFIX.push_back("/logout");			// Index 5 : user closing
	PREFIX.push_back("/close_server");		// Index 6 : server closing
	PREFIX.push_back("/exit");			// Index 7 : exit a thread
	PREFIX.push_back("/reply"); 			// Index 8 : reply to recent ID
}



//------------------------------------SET PREFIXES--------------------------------------//


//--------------------------------------LOGGIN IN---------------------------------------//


inline void			cpf_loggin_in(){

	// PREFIX[2] = /login
	// PREFIX[3] = /active_user

	// First send loggin in message
	string loggin_in_message = PREFIX[2]+" " + USER_ID + "\n";
	cpf_send_message(loggin_in_message);

	// Now server will send messages about active users
	while(true){

		cpf_recv_message();
		if( (RECEIVED_MESSAGE.front()).find(PREFIX[3]) == 0 ){

			// Store active users in a local set.
			RECEIVED_MESSAGE.pop();
			ACTIVE_USERS.insert(RECEIVED_MESSAGE.front());
			RECEIVED_MESSAGE.pop();
		}
		else
			break;
	}

	puts( "login successful" );

	puts("\n\n---------------------------------------------------------------------\n");

	if(ACTIVE_USERS.size()){

		puts( "ACTIVE USERS:" );
		// Display the active users
		for(auto &elem: ACTIVE_USERS){
			cout<<"[ "<<elem<<" ]"<<'\n';
		}
	}
	else{
		puts( "No Active User");
	}
	puts("\n---------------------------------------------------------------------\n\n");
}


//--------------------------------------LOGGIN IN---------------------------------------//



//---------------------------------MESSAGE SEGMENTATION---------------------------------//


inline void			cpf_segment_message(char* msg, queue<string> &que){

	// Empty the queue
	while( que.size()){

		que.pop();
	}

	// Segment the reveived message
	string message_word = "";
	int n = 1024;

	for(int i=0; i<n; i++){
		if( (i == n-1) || (msg[i] == ' ') || (msg[i] == '\n') || (msg[i] == 0) ){

			que.push( message_word );
			message_word = "";
			if( (msg[i] == '\n') || (msg[i] == 0))
				break;
		}
		else
			message_word.push_back( msg[i] );
	}
	if( message_word != "" )
		que.push( message_word );
}


//---------------------------------MESSAGE SEGMENTATION---------------------------------//



//----------------------------------------SENDING---------------------------------------//


inline void			cpf_sending(){

	while( true ){ // SERVER_STATUS == 1 -> active

		fgets( CLIENT_MESSAGE, sizeof(CLIENT_MESSAGE), stdin);
		cpf_segment_message( CLIENT_MESSAGE , SENDING_MESSAGE );
		memset( CLIENT_MESSAGE, 0, sizeof( CLIENT_MESSAGE ));




		if( (SENDING_MESSAGE.front()).find(PREFIX[0]) == 0){ 			// cpf_help

			SENDING_MESSAGE.pop();
			if(SENDING_MESSAGE.size() == 0)
				cpf_help();
			else{
				int index = cpf_string_to_int(SENDING_MESSAGE.front());

				if( index = -1 )
					continue;
				cpf_help( index );
			}
		}
		// keepalive
		else if( (SENDING_MESSAGE.front()).find(PREFIX[1]) == 0){

			cpf_send_message( PREFIX[1] );
		}
		// messgae
		else if( (SENDING_MESSAGE.front()).find(PREFIX[4]) == 0){

			// message <receiver ID> message
			cpf_build_sending_message();
		}

		// close
		else if( (SENDING_MESSAGE.front()).find(PREFIX[5]) == 0){

			// client wants to close
			cpf_logout_send();
			return;
		}

		// close server
		else if( (SENDING_MESSAGE.front()).find(PREFIX[6]) == 0){

			// SENDING_MESSAGE: /close_server <server_key>
			cpf_send_close_server();
		}

		// exit
		else if( (SENDING_MESSAGE.front()).find(PREFIX[7]) == 0){

			// SENDING_MESSAGE : /exit
			SENDING_MESSAGE.pop();

			puts("Sending is not be possible anymore...");
			return;
		}

		// reply


	}
}


//----------------------------------------SENDING---------------------------------------//



//----------------------------------------RECEIVING---------------------------------------//


inline void			cpf_receiving(){
	while(true){

		// receiving message
		cpf_recv_message();

		// PREFIX[2] = "/login"
		if( (RECEIVED_MESSAGE.front()).find(PREFIX[2]) == 0 ){

			// format: /login <user_id>
			RECEIVED_MESSAGE.pop();

			// format: <user_id>
			string msg = "[ " + RECEIVED_MESSAGE.front() +" ] is now active.";

			puts(msg.c_str());

			// inserting the new logged in user in local list
			ACTIVE_USERS.insert( RECEIVED_MESSAGE.front() );

			// clear the queue
			RECEIVED_MESSAGE.pop();
		}

		// PREFIX[4] = "/message"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[4]) == 0){

			// /message <sender ID> <receiver ID> message
			cpf_build_received_message();

		}

		// PREFIX[5] = "/logout"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[5]) == 0){

			// RECEIVED_MESSAGE queue : /logout <user_id>
			bool feedback = false;
			cpf_logout_recv( feedback );

			// if feedback == true client should be Closing
			if( feedback == true )
				return;
		}

		// PREFIX[6] = "/close_server"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[6]) == 0){

			// RECEIVED_MESSAGE: /close_server <user_id>
			cpf_recv_close_server();

			return;
		}

		// PREFIX[7] = "/exit"
		else if( (RECEIVED_MESSAGE.front()).find(PREFIX[7]) == 0){

			// RECEIVED_MESSAGE: /close_server <user_id>
			RECEIVED_MESSAGE.pop();

			puts("Receiving message is no more possible...");

			return;
		}
	}
}


//----------------------------------------SENDING---------------------------------------//



inline int			cpf_string_to_int(string &str){

	int num;
	try{
		num = stoi( str );
	}
	catch (std::invalid_argument& e) {
		num = -1;
    		std::cout << "Invalid argument: " << e.what() << '\n';
	}
	catch (std::out_of_range& e) {
		num = -1;
    		std::cout << "Out of range: " << e.what() << '\n';
	}
	return num;
}


//------------------------------------BUILD MESSAGE-------------------------------------//

inline void 		cpf_build_received_message(){

	// /messgae <sender ID> <receiver ID> message
	RECEIVED_MESSAGE.pop();

	// <sender ID> <receiver ID> message
	string build_message = "Message from ["+RECEIVED_MESSAGE.front()+"]: ";
	RECEIVED_MESSAGE.pop();

	// <receiver ID> message
	RECEIVED_MESSAGE.pop();

	// message
	while( RECEIVED_MESSAGE.size() ){
		build_message.append( " " + RECEIVED_MESSAGE.front() );
		RECEIVED_MESSAGE.pop();
	}

	// print the message
	puts( build_message.c_str() );
}


//--------------------------------------------------------------------------------------//



//--------------------------------------------------------------------------------------//


inline void 		cpf_build_sending_message(){

	string build_msg = PREFIX[4]+ " " + USER_ID;

	// format: /message <receiver ID> message
	SENDING_MESSAGE.pop();

	// format: <receiver ID> message
	// build_msg = /message <sender ID>
	while( SENDING_MESSAGE.size() ){

		build_msg.append( " " + SENDING_MESSAGE.front());
		SENDING_MESSAGE.pop();
	}

	// format: /message <sender ID> <receiver ID> message
	build_msg.push_back( '\n' );

	// send message to server
	cpf_send_message( build_msg );
}


//------------------------------------MESSAGE BUILD-------------------------------------//



//------------------------------------FEATURE LOGOUT------------------------------------//


inline void 			cpf_logout_send(){ // feature: /logout

	// PREFIX[5] = "/logout"
	// SENDING_MESSAGE queue : /logout
	// USER_ID contains user's login ID

	SENDING_MESSAGE.pop();
	string msg = PREFIX[5] + " " + USER_ID + "\n";

	cpf_send_message( msg );
}


//--------------------------------------------------------------------------------------//


inline void 			cpf_logout_recv(bool &feedback){ // feature: /logout

	// PREFIX[5] = "/logout"
	// RECEIVED_MESSAGE queue : /logout <user_id>
	// USER_ID contains user's login ID
	RECEIVED_MESSAGE.pop();

	// RECEIVED_MESSAGE queue : <user_id>
	//check if server's message matches with the client's user_id
	feedback = (RECEIVED_MESSAGE.front() == USER_ID);

	if( feedback == true )
		return;

	string msg = "[ " + RECEIVED_MESSAGE.front() + " ] is now Inactive";
	puts( msg.c_str() );

	// remove the user_id from ACTIVE_USERS lits if found
	if( ACTIVE_USERS.find(RECEIVED_MESSAGE.front()) != ACTIVE_USERS.end() ){

		ACTIVE_USERS.erase(RECEIVED_MESSAGE.front());
	}

	// clean RECEIVED_MESSAGE
	RECEIVED_MESSAGE.pop();
}


//------------------------------------FEATURE LOGOUT------------------------------------//



//--------------------------------FEATURE CLOSING SERVER--------------------------------//

inline void 			cpf_send_close_server(){

	// SENDING_MESSAGE: /close_server <server_key>
	SENDING_MESSAGE.pop();

	// SENDING_MESSAGE: <server_key>
	string msg = PREFIX[6] + " " + USER_ID + " " + SENDING_MESSAGE.front()+"\n";
	// msg = /close_server <user_id> <server_key>

	// empty SENDING_MESSAGE
	SENDING_MESSAGE.pop();

	// Send the message
	cpf_send_message(msg);
}


//--------------------------------------------------------------------------------------//


inline void 			cpf_recv_close_server(){

	// RECEIVED_MESSAGE: /close_server <user_id>
	RECEIVED_MESSAGE.pop();

	// RECEIVED_MESSAGE: <user_id>
	// check for user_id
	if( RECEIVED_MESSAGE.front() != USER_ID ){

		// print who closed the server
		puts(("Server closing soon... Closed by [ " + RECEIVED_MESSAGE.front() + " ].").c_str());
		std::this_thread::sleep_for(std::chrono::seconds(2));
		puts("Please close your client by using /exit");
	}
	else{
		puts( "Server closing soon..." );
		std::this_thread::sleep_for(std::chrono::seconds(2));
		puts("Please close your client by using /exit");
	}
}


//--------------------------------FEATURE CLOSING SERVER--------------------------------//
