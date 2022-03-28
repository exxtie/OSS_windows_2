#define WINVER 0x0502
#include <iostream>
#include <windows.h>
#include <sddl.h>

//static PSECURITY_DESCRIPTOR create_security_descriptor()
//{
//    const char* sddl = "D:(A;OICI;GRGW;;;AU)(A;OICI;GA;;;BA)";
//    PSECURITY_DESCRIPTOR security_descriptor = NULL;
//    ConvertStringSecurityDescriptorToSecurityDescriptor(
//    sddl, SDDL_REVISION_1, &security_descriptor, NULL);
//    return security_descriptor;
//}
//
//static SECURITY_ATTRIBUTES create_security_attributes()
//{
//    SECURITY_ATTRIBUTES attributes;
//    attributes.nLength = sizeof(attributes);
//    attributes.lpSecurityDescriptor =
//    create_security_descriptor();
//    attributes.bInheritHandle = FALSE;
//    return attributes;
//}

int main() {
    char mailName[128];

    printf("Enter full mailslot path >> ");
    fgets(mailName, sizeof(mailName), stdin);
    mailName[strlen(mailName)-1] = '\0';

    BOOL createdMail = TRUE;

    HANDLE m_slot = CreateMailslotA(
            mailName,
            256,
            MAILSLOT_WAIT_FOREVER,
            NULL);

//    if (m_slot == INVALID_HANDLE_VALUE)
//    {
//        printf("CreateMailSlot Error: %d", GetLastError());
//        getchar();
//        return 1;
//    }

//    if (GetLastError() == ERROR_ALREADY_EXISTS)
    if (m_slot == INVALID_HANDLE_VALUE)
    {
        m_slot = CreateFileA (
                mailName,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                0,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                0);

        createdMail = FALSE;
    }

    if (createdMail) //TODO: убрать восклицательный знак, когда закончу отладку write
    {
        while (true)
        {
            printf("Choose an action:\n");
            printf("\t'read' to read the next message;\n");
            printf("\t'check' to check for messages;\n");
            printf("\t'quit' to terminate.\n");
            printf(">> ");

            char command[64];
            fgets(command, sizeof(command), stdin);
            command[strlen(command)-1] = '\0';

            if (!strcmp(command, "read"))
            {
                DWORD maxMessage, nextMessage, messageCount;
                GetMailslotInfo(m_slot, &maxMessage, &nextMessage, &messageCount, NULL);
                if (nextMessage == MAILSLOT_NO_MESSAGE)
                {
                    printf("[ OK ] There is no messages now.\n");
                }
                else
                {
                    char nextFile[256];
                    ReadFile(m_slot, nextFile, sizeof(nextFile), NULL, NULL);
                    nextFile[nextMessage] = '\0';
                    printf("%s", nextFile);
                }

            }
            else if (!strcmp(command, "check"))
            {
                DWORD maxMessage, nextMessage, messageCount;
                GetMailslotInfo(m_slot, &maxMessage, &nextMessage, &messageCount, NULL);
                printf("[ OK ] Max message size: %i\n", maxMessage);
                printf("[ OK ] Next message size: %i\n", nextMessage);
                printf("[ OK ] Messages left: %i\n", messageCount);
            }
            else if (!strcmp(command, "quit"))
            {
                break;
            }

            printf("\n");
        }
    }
    else
    {
        while (true)
        {
            printf("Choose an action:\n");
            printf("\t'write' to write a message;\n");
            printf("\t'check' to check for messages;\n");
            printf("\t'quit' to terminate.\n");
            printf(">> ");

            char command[64];
            fgets(command, sizeof(command), stdin);
            command[strlen(command)-1] = '\0';

            if (!strcmp(command, "write"))
            {
                //TODO: разобраться с ошибкой вывода newMessage
                printf("Enter message text. Terminate with an empty line.\n");

                char newMessage[256] = "";
                int i = 0;
                do
                {
                    newMessage[i] = getchar();
                    i++;
                } while((newMessage[i-1] != '\n') || (newMessage[i-2] != '\n'));

//                newMessage[strlen(newMessage)-1] = '\0';
//                newMessage[strlen(newMessage)-1] = '\0';
//                for (int j = 0; j < strlen(newMessage); j++)
//                    printf("%c", newMessage[j]);
//                printf("\n");
//                printf("%s", newMessage);

//                fgets(newMessage, sizeof(newMessage), stdin);
//                newMessage[strlen(newMessage)-1] = '\0';

                WriteFile(m_slot, newMessage, strlen(newMessage), NULL, NULL);

                printf("[ OK ] Message was sent.\n");

            }
            else if (!strcmp(command, "check"))
            {
                DWORD maxMessage, nextMessage, messageCount;
                GetMailslotInfo(m_slot, &maxMessage, &nextMessage, &messageCount, NULL);
                if (maxMessage == 0)
                    printf("[ OK ] Max message size: unlimited\n");
                else
                    printf("[ OK ] Max message size: %i\n", maxMessage);
                printf("[ OK ] Next message size: %i\n", nextMessage);
                printf("[ OK ] Messages left: %i\n", messageCount);
            }
            else if (!strcmp(command, "quit"))
            {
                break;
            }

            printf("\n");
        }
    }

    CloseHandle(m_slot);

    return 0;
}
