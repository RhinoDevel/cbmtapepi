
// Marcel Timm, RhinoDevel, 2024

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
    char const * const sudo_user = getenv("SUDO_USER");

    printf("sudo user is \"%s\".\n", sudo_user);

    uid_t const sudo_uid = atoi(getenv("SUDO_UID"));

    printf("sudo user ID is %u.\n", sudo_uid);

    uid_t const sudo_gid = atoi(getenv("SUDO_GID"));

    printf("sudo group ID is %u.\n", sudo_gid);

    long const page_size = sysconf(_SC_PAGESIZE);

    printf("Page size is %ld bytes.\n", page_size);

    size_t const size = (size_t)page_size * 1024;

    printf("Size to map is %zu bytes\n", size);

    pid_t child_pid = -1;
    int child_status = -1;
    char* shared_mem = mmap(
            NULL,
            size,
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS,
            -1,
            0);

    if(shared_mem == MAP_FAILED)
    {
        perror("Error: Memory map failed");
        exit(EXIT_FAILURE);
    }

    // root is writing something to the mapped memory:
    //
    shared_mem[0] = 'M';
    shared_mem[1] = 'T';
    shared_mem[2] = '\0';

    // A child process is created:
    //
    child_pid = fork();
    if(child_pid == -1)
    {
        perror("Error: Fork failed");
        exit(EXIT_FAILURE);
    }

    // The initial/parent and the new/child processes are running.

    if(child_pid == 0)
    {
        // This is where the new/child process proceeds.

        // Drop root privileges:
        //
        if(setgid(sudo_gid) == -1)
        {
            perror("Error: Setting group ID failed");
            exit(EXIT_FAILURE);
        }
        if(setuid(sudo_uid) == -1)
        {
            perror("Error: Setting user ID failed");
            exit(EXIT_FAILURE);
        }

        printf("CHILD PROCESS: Message from parent process is \"%s\".\n", shared_mem);

        // Write something back for the parent process:
        //
        shared_mem[0] = 'R';
        shared_mem[1] = 'D';
        shared_mem[2] = '!';
        shared_mem[3] = '\0';

        //raise(SIGTERM); // To test non-normal exit.

        exit(EXIT_SUCCESS);
    }

    // This is where the initial/parent process proceeds.

    if(waitpid(child_pid, &child_status, 0) == -1)
    {
        perror("Error: Wait failed");
        exit(EXIT_FAILURE);
    }
    if(!WIFEXITED(child_status))
    {
        printf("Error: Child process did not exit normally!\n");
        exit(EXIT_FAILURE);
    }
    if(WEXITSTATUS(child_status) != 0)
    {
        printf("Error: Child process exited with failure!\n");
        exit(EXIT_FAILURE);
    }

    printf("Message from child process is \"%s\".\n", shared_mem);

    if(munmap(shared_mem, size) != 0)
    {
        perror("Error: Memory unmap failed");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
