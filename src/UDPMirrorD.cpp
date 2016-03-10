/**
 * @file mirror.cpp
 * @author Caio Marcelo Campoy Guedes
 * @date 15 feb 2016
 * @brief A simple mirror for 'n' destinations
 */

//C++ Headers
//---------------------------------------------------------------------------------------------
#include <iostream> //C++ standard I/O
#include <vector>
//---------------------------------------------------------------------------------------------

//C Headers
//---------------------------------------------------------------------------------------------
#include <cstring> //C string manipulation
//---------------------------------------------------------------------------------------------

//Unix / Linux Headers
//---------------------------------------------------------------------------------------------
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
//---------------------------------------------------------------------------------------------

//Networking Headers
//---------------------------------------------------------------------------------------------
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
//---------------------------------------------------------------------------------------------

//Global variables and macros
//---------------------------------------------------------------------------------------------
#define BUFFER_LENGTH 1880 //Network buffer length

static int verbose_flag = 0; //Verbose flag
static int daemonize = 0; //Daemon activator flag
static int receiver_port = 0; //Receiver port
static int amount_of_mirrors = 0; //Mirrors count


//---------------------------------------------------------------------------------------------

/**
 * @brief Program Usage
 * @details Program Usage Specifics
 */
void usage (void) {
    std::cout << "\033[1;37mNAME\033[0m" << std::endl;
    std::cout << "        fogo_mirror - A simple mirror for the fogo suite" << std::endl;
	std::cout << "\033[1;37mSYNOPSIS\033[0m" << std::endl;
    std::cout << "        fogo_mirror <options> <receiver> [mirrors]" << std::endl;
    std::cout << "\033[1;37mDESCRIPTION\033[0m" << std::endl;
    std::cout << "        Receives data at an UDP port and mirrors the information to the desired IPs and ports." << std::endl;
    std::cout << "        --verbose" << std::endl;
    std::cout << "            Display extra information on the screen. If the daemonize flag is set this option will be disabled." << std::endl;
    std::cout << "        --daemonize" << std::endl;
    std::cout << "            Run the mirror as a Daemon."  << std::endl;
    std::cout << "        --receiver=PORT" << std::endl;
    std::cout << "            The port to wait for information" << std::endl;
    std::cout << "\033[1;37mEXIT STATUS\033[0m" << std::endl;
    std::cout << "        0 - If ok" << std::endl;
    std::cout << "        1 - If a problem occured" << std::endl;
    std::cout << "\033[1;37mUSE EXAMPLE\033[0m" << std::endl;
    std::cout << "        \033[0;35mfogo_mirror --daemonize --receiver=8000 127.0.0.1:5000 192.168.0.13:40000...\033[0m" << std::endl;
}
/**
 * @brief Verbose activation
 * @details Activate the verbose function
 * 
 * @param content String to be displayed
 */
void verbose (std::string content) {
    if (verbose_flag) {
        std::cout << content << std::endl;
    }
}
/**
 * @brief Arguments Parser
 * @details Parse the arguments and activate the respective flags
 * 
 * @param argc Argument counter
 * @param argv Argument Values
 * 
 * @return True if succeeded
 */
bool parseArguments (int argc, char** argv) {
    int long_index = 0;
    int opt        = 0;

    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
        {"daemonize", no_argument, 0, 'd'},
        {"receiver", required_argument, 0, 'r'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "r:vd", long_options, &long_index)) != -1) {
        switch (opt) {
            case 'r':
                receiver_port = atoi(optarg);
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 'd':
                daemonize = 1;
                break;
        }
    }
    return true;
}
/**
 * @brief Daemon creator
 * @details Spawn a clone process in background
 */
static void createDaemon (void) {
    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);
    if (pid > 0)
        exit(EXIT_SUCCESS);
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);
    chdir("/");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>0; x--) {
        close (x);
    }
    openlog ("firstdaemon", LOG_PID, LOG_DAEMON);
}
/**
 * @brief The main funcion
 *
 * @param argc filled by default
 * @param argv filled by default
 * 
 * @return 0 if succeeded
 **/
int main (int argc, char** argv) {
	if (argc <= 2) {
		std::cerr << "\033[1;31mNot all arguments were properly inserted!\033[0m" << std::endl;
		usage();
		return 1;
	}

    if (!parseArguments(argc, argv)) {
        std::cerr << "\033[1;31mFailed to parse arguments!\033[0m" << std::endl;
        return 1;
    }

    if (daemonize) {
        verbose_flag = 0;
        createDaemon();
    }
    amount_of_mirrors = argc - optind;

    struct sockaddr_in serv_addr;
    struct sockaddr_in sender_addr;
    struct sockaddr_in addresses[amount_of_mirrors];

    int sockfd_receiver = 0;
    int sockfds[amount_of_mirrors];
    int i = 0;

    int recv_len        = 0;
    socklen_t slen      = sizeof(sender_addr);

    char buffer[BUFFER_LENGTH];

    verbose("Initializing sockets...");
  
    if (optind < argc) {
        std::cout << argv[optind] << std::endl;
        std::cout << argv[optind + 1] << std::endl;
        std::cout << argv[optind + 2] << std::endl;
        for (int i = 0; i < amount_of_mirrors; i++) {
            char* ip   = strtok(argv[optind++], ":");
            char* port = strtok(NULL, ":");

            if ((sockfds[i] = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
                std::cerr << "\033[1;31mFailed to create receiver socket!\033[0m" << std::endl;
                return 1;
            }

            memset((char*)&addresses[i], 0, sizeof(serv_addr));

            addresses[i].sin_family = AF_INET;
            addresses[i].sin_port   = htons(atoi(port));
            addresses[i].sin_addr.s_addr = inet_addr(ip);
        }
    } else {
        std::cerr << "\033[1;31mCheck the mirrors IPs\033[0m" << std::endl;
        return 1;
    }

    if ((sockfd_receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("SocketError");
        return 1;
    }

    verbose("Sockets initialized!");
    verbose("Binding receiver socketing to port...");

    memset((char*)&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port   = htons(receiver_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd_receiver, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("SocketError");
        return 1;
    }

    verbose("Socket binded!");

    std::cout << "Waiting on: udp://localhost:" << receiver_port << std::endl;
    while (true) {
    	if ((recv_len = recvfrom(sockfd_receiver, buffer, BUFFER_LENGTH, 0, (struct sockaddr *) &sender_addr, &slen)) == -1) {
            std::cout << "\033[1;31mFailed to receive data! Please contact Fogo support!\033[0m" << std::endl;
            return 1;
        }
        if (verbose_flag) {
            printf("Received packet from %s:%d\n", inet_ntoa(sender_addr.sin_addr), ntohs(sender_addr.sin_port));
            printf("Data: %d\n" , recv_len);
        }
        for (int i = 0; i < amount_of_mirrors; i++) {
            if (sendto(sockfd_receiver, buffer, recv_len, 0, (struct sockaddr*) &addresses[i], slen) == -1){
                std::cout << "\033[1;31mFailed to send data to video port: " << ntohs(addresses[i].sin_port) << "! Please contact Fogo support!\033[0m" << std::endl;
            }
        }
    }
	return 0;
}