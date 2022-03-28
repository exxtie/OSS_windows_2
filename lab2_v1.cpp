#include <iostream>
#include <stdio.h>
#include <windows.h>
#define BUF_SIZE1 256

int main() {


    char fpName[256];

    printf("Enter shared memory name: ");
    scanf("%s", &fpName);
    getchar();

    HANDLE file_map;

    file_map = OpenFileMappingA(
            FILE_MAP_ALL_ACCESS,
            FALSE,
            fpName);
    if (GetLastError() != ERROR_SUCCESS)
    {
        file_map = CreateFileMappingA(
                INVALID_HANDLE_VALUE,           // используется часть системного файла подкачки
                NULL,                           // отсутствие особых атрибутов безопасности
                PAGE_READWRITE,                 // доступ к области памяти для чтения и записи
                0,                              // максимальный размер области (страшее DWORD)
                BUF_SIZE1,                      // максимальный размер области (младшее DWORD)
                fpName);

        if (file_map == NULL)
        {
            printf("CreateFileMapping Error: %d\n", GetLastError());
            return 1;
        }
    }

    LPVOID memory = MapViewOfFile(file_map,             // дескриптор файлового отображения
                                  FILE_MAP_ALL_ACCESS,  // полный доступ к участку памяти
                                  0,
                                  0,
                                  BUF_SIZE1);

    if (memory == NULL)
    {
        printf("MapViewOfFile Error: %d\n", GetLastError());
        CloseHandle(file_map);
        return 1;
    }

    char* message = (char*) memory;

    while(true)
    {
        char choice;
        printf("\nChoose an action:\n");
        printf("R)ead message from shared memory;\n");
        printf("W)rite message to shared memyro;\n");
        printf("Q)uit.\n");
        printf("Enter a letter >> ");
        scanf("%c", &choice);
        getchar();

        if (tolower(choice) == 'r')
        {
            printf("Message from shared memory:\n%s\n", message);
        }

        else if (tolower(choice) == 'w')
        {
            printf("Enter message text: ");
            char input[128];
            fgets(input, sizeof(input), stdin);
            input[strlen(input)-1] = '\0';
            strcpy(message, input);
            printf("Message was sent!\n");
        }

        else if (tolower(choice) == 'q')
        {
            break;
        }

        else
            printf("No such command! Try again!\n");
    }

    UnmapViewOfFile(memory);
    CloseHandle(file_map);
}
