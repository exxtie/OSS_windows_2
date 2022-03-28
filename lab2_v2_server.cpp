#include <iostream>
#include <stdio.h>
#include <windows.h>
#include <sstream>
#include <map>
#include <string.h>
//#include <algorithm>

#define BUF_SIZE2 64

using namespace std;

void set_func (istringstream *parser, map<string, string> &data, HANDLE pipe);
void get_func (istringstream *parser, map<string, string> data, HANDLE pipe);
void help_func (HANDLE pipe);

int main() {

    // Вар1. Обмен сообщениями через разделяемую память

//    char fpName[256];
//
//    HANDLE file_map = CreateFileMappingA(
//            INVALID_HANDLE_VALUE,           // используется часть системного файла подкачки
//            NULL,                           // отсутствие особых атрибутов безопасности
//            PAGE_READWRITE,                 // доступ к области памяти для чтения и записи
//            0,                              // максимальный размер области (страшее DWORD)
//            BUF_SIZE1,                      // максимальный размер области (младшее DWORD)
//            fpName);
//
//    if (file_map == NULL)
//    {
//        printf("CreateFileMapping Error: %d\n", GetLastError());
//        return 1;
//    }
//
//    LPVOID memory = MapViewOfFile(file_map,             // дескриптор файлового отображения
//                                  FILE_MAP_ALL_ACCESS,  // полный доступ к участку памяти
//                                  0,
//                                  0,
//                                  BUF_SIZE1);
//
//    if (memory == NULL)
//    {
//        printf("MapViewOfFile Error: %d\n", GetLastError());
//        CloseHandle(file_map);
//        return 1;
//    }
//
//    char* message = (char*) memory;
//
//    char input[128];
//    fgets(input, sizeof(input), stdin);
//    strcpy(message, input);
//
//    printf("Message from shared memory: %s\n", message);
//
//    UnmapViewOfFile(memory);
//    CloseHandle(file_map);

    // Вар2. Хранилище значений по ключу с доступом через именованные каналы
    BOOL fConnected = FALSE;
    map<string, string> data;

    printf("Enter pipe name (without `\\\\.\\pipe\\'): ");
    string name;
    cin >> name;

    // имя именованного канала
    auto path = "\\\\.\\pipe\\" + name;

    // создание именованного канала
    auto pipe = CreateNamedPipeA(path.c_str(),
                                 PIPE_ACCESS_DUPLEX,                // дуплексный режим открытия
                                 PIPE_TYPE_MESSAGE,                 // режим работы ориентированный на сообщения
                                 PIPE_UNLIMITED_INSTANCES,          // кол-во экземпляров ограничено возможностями системы
                                 BUF_SIZE2,                         // размер буфера передачи
                                 BUF_SIZE2,                         // размер буфера приема
                                 0,                                 // таймаут операций
                                 NULL);                             // атрибуты безопасности

    if (pipe == INVALID_HANDLE_VALUE)
    {
        printf("CreateNamedPipe Error: %d\n", GetLastError());
        return 1;
    }

    while (true)
    {
        map<string, int> keywords_int;
        keywords_int["set"] = 1;
        keywords_int["get"] = 2;
        keywords_int["help"] = 3;
        keywords_int["quit"] = 4;

        printf("Waiting for a client... ");

        // подключению к клиенту
        fConnected = ConnectNamedPipe(pipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (fConnected)
        {
            printf("connected.\n");

            BOOL quit_flag = TRUE;

            while (quit_flag)
            {
                // чтение команды
                printf("Waiting for command... ");
                string command(64, '\0');
                ReadFile(pipe, &command[0], command.size(), NULL, NULL);

                if (command[0] != '\0')
                    printf("received.\n");

                printf("comm: %s\n\n", command.c_str());

                istringstream parser{command};
                string keyword;
                parser >> keyword;

                // выбор действия в зависимости от полученной команды
                switch (keywords_int[keyword.c_str()])
                {
                    case 1:
                        set_func(&parser, data, pipe);
                        break;

                    case 2:
                        get_func(&parser, data, pipe);
                        break;

                    case 3:
                        help_func(pipe);
                        break;

                    case 4:
                        DisconnectNamedPipe(pipe);
                        quit_flag = FALSE;
                        break;

                    default:
                        char response[] = "No such command! Try again!\n";
                        WriteFile(pipe, response, strlen(response), NULL, NULL);
                        break;
                }
            }
        }
        else
        {
            CloseHandle(pipe);
            break;
        }

        // возможность продолжить работу сервера после отключения клиента
        string cont;
        while (true)
        {
            printf("Do you want to continue? [y or n]: ");
            cin >> cont;
            if (tolower(cont[0]) == 'n' or tolower(cont[0]) == 'y')
                break;
            else
                printf("Wrong choice! Try again!\n");
        }

        if (tolower(cont[0]) == 'n')
        {
            break;
        }
    }
    CloseHandle(pipe);


    printf("Work finished!\n");
    return 0;
}

// функция для команды set
void set_func (istringstream *parser, map<string, string> &data, HANDLE pipe)
{
    string name;
    string value;
    *parser >> name >> value;

    data[name] = value;

    string response = "acknowledged\n";
    WriteFile(pipe, response.c_str(), response.size(), NULL, NULL);
}

// функция для команды get
void get_func (istringstream *parser, map<string, string> data, HANDLE pipe)
{
    string get_param;
    *parser >> get_param;

    char response[256] = "\0";


    if (strcmp(get_param.c_str(), "all") == 0)
    {
        strcpy(response, "All data:\n");

        for (auto item : data)
        {
            strcat(response, item.first.c_str());
            strcat(response, " ");
            strcat(response, item.second.c_str());
            strcat(response, "\n");
        }

    }
    else if (data.find(get_param.c_str()) != data.end())
    {
        strcat(response, "Response: ");
        strcat(response, get_param.c_str());
        strcat(response, " ");
        strcat(response, data[get_param.c_str()].c_str());
        strcat(response, "\n");
    }
    else
    {
        strcat(response, "No such key!\n");
    }

    WriteFile(pipe, response, strlen(response), NULL, NULL);
}

// функция для команды help
void help_func(HANDLE pipe)
{
    char response[] = "There are 4 commands:\n"
                      "set <key> <value> - add one note to the map with key <key> and value <value>;\n"
                      "get [param] - you can choose one of 2 parameters: 'all' or <key>;\n"
                      "  'all' will return the whole map which was created during current work session;\n"
                      "  <key> if such key exists in the map, key and value will be returned, otherwise 'No such key!' phrase;\n"
                      "help - will return this message;\n"
                      "quit - stop the current client session (server can still work, so the map will be save, until server turned off).\n";

    WriteFile(pipe, response, strlen(response), NULL, NULL);
}
