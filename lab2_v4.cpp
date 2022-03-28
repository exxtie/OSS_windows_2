#include <iostream>
#include <windows.h>
#include <string.h>

int main() {
    SECURITY_ATTRIBUTES attributes;
    attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
    attributes.bInheritHandle = TRUE;
    attributes.lpSecurityDescriptor = NULL;

    HANDLE input_pipe_read_end;
    HANDLE input_pipe_write_end;
    CreatePipe(&input_pipe_read_end, &input_pipe_write_end, &attributes, 0);

    HANDLE output_pipe_read_end;
    HANDLE output_pipe_write_end;
    CreatePipe(&output_pipe_read_end, &output_pipe_write_end, &attributes, 0);

    SetHandleInformation(input_pipe_write_end, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(output_pipe_read_end, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.hStdInput = input_pipe_read_end;
    startup_info.hStdOutput = output_pipe_write_end;
    startup_info.hStdError = output_pipe_write_end;
    startup_info.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION pi;
    CreateProcess(
            NULL,
            "cmd.exe",
            NULL,
            NULL,
            TRUE,
            0,
            NULL,
            NULL,
            &startup_info,
            &pi);

    DWORD bytes_read;
    char read_buffer[64];
    do
    {
        ReadFile(output_pipe_read_end, read_buffer, sizeof(read_buffer), &bytes_read, NULL);
        fwrite(read_buffer, bytes_read, 1, stdout);
    } while ((bytes_read != 0) && (read_buffer[bytes_read - 1] != '>'));


    while (true)
    {
        const char PLEASE[] = "please";

        char* input = NULL;
        char buffer[256];
        while (!input)
        {
            fgets(buffer, sizeof(buffer), stdin);
            if (!strncmp(buffer, "thanks", 6))
            {
                break;
            }
            else if (strncmp(buffer, "please", 6))
            {
                fprintf(stderr, "Please ask politely!\n> ");
            }
            else
            {
                input = buffer + sizeof(PLEASE);
            }
        }

        if (input == NULL)
            break;
        else
        {
            WriteFile(input_pipe_write_end, input, strlen(input), NULL, NULL);

            char read_buffer[64];
            do
            {
                ReadFile(output_pipe_read_end, read_buffer, sizeof(read_buffer), &bytes_read, NULL);
                fwrite(read_buffer, bytes_read, 1, stdout);
            } while ((bytes_read != 0) && (read_buffer[bytes_read - 1] != '>'));
        }
    }

    TerminateProcess(pi.hProcess, 0);
    CloseHandle(input_pipe_write_end);
    CloseHandle(output_pipe_read_end);
    CloseHandle(output_pipe_write_end);


    return 0;
}
