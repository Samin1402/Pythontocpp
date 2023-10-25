#include <iostream>
#include <string>
#include <thread>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>

static const char* SOCKET_PATH = "/var/run/codesysextension/extfuncs/myExternalFunction.sock";
static const unsigned int BUFFER_SIZE = 1024;

class ExternalFunctionBase {
protected:
    int sock;
    sockaddr_un local;

public:
    ExternalFunctionBase() {
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock == -1) {
            std::cerr << "Error creating socket." << std::endl;
            exit(1);
        }

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, SOCKET_PATH);
        unlink(local.sun_path);
        int len = strlen(local.sun_path) + sizeof(local.sun_family);
        if (bind(sock, (struct sockaddr*)&local, len) != 0) {
            std::cerr << "Error binding socket." << std::endl;
            exit(1);
        }
    }

    virtual ~ExternalFunctionBase() {
        close(sock);
    }

    virtual void Call(const std::string& params, std::string& result) = 0;

    void Run() {
        if (listen(sock, 5) != 0) {
            std::cerr << "Error on listen call." << std::endl;
            exit(1);
        }

        while (true) {
            sockaddr_un remote;
            unsigned int sock_len = sizeof(remote);
            int client_sock = accept(sock, (struct sockaddr*)&remote, &sock_len);

            char buffer[BUFFER_SIZE];
            read(client_sock, buffer, BUFFER_SIZE);

            // TODO: Parse the buffer to extract parameters for the Call function.
            // Here, for simplicity, we'll assume the buffer directly contains our parameters.

            std::string result;
            Call(buffer, result);

            write(client_sock, result.c_str(), result.size() + 1);
            close(client_sock);
        }
    }
};

class myExternalFunction : public ExternalFunctionBase {
public:
    void Call(const std::string& params, std::string& result) override {
        // TODO: Extract actual parameters from the string.
        // Here, for simplicity, we'll assume params is of the form "x,y" where x and y are integers to be added.

        size_t comma_pos = params.find(',');
        int x = std::stoi(params.substr(0, comma_pos));
        int y = std::stoi(params.substr(comma_pos + 1));
        result = std::to_string(x + y);
    }
};

int main() {
    myExternalFunction func;
    func.Run();
    return 0;
}
