//6. Interprocess Communication (CO5)
//FIFOs: Full duplex communication between two independent processes. First process
//accepts sentences and writes on one pipe to be read by second process and second
//process counts number of characters, number of words and number of lines in accepted
//sentences, writes this output in a text file and writes the contents of the file on second
//pipe to be read by first process and displays on standard output.


//PROCESS 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFO1 "fifo1"  // Pipe to send data to Process 2
#define FIFO2 "fifo2"  // Pipe to receive results from Process 2

int main() {

    char buffer[1024];  // Stores input and received data
    int fd1, fd2;       // File descriptors for FIFO pipes
    ssize_t n;          // Number of bytes read

    // Create two named pipes
    mkfifo(FIFO1, 0666);
    mkfifo(FIFO2, 0666);

    // Ask user to enter sentences
    printf("Enter sentences (type END to finish):\n");

    // Open FIFO1 to send data
    fd1 = open(FIFO1, O_WRONLY);

    // Read sentences from user
    while (fgets(buffer, sizeof(buffer), stdin)) {

        // Stop when user enters END
        if (strcmp(buffer, "END\n") == 0 ||
            strcmp(buffer, "END") == 0)
            break;

        // Send sentence to Process 2
        if (write(fd1, buffer, strlen(buffer)) == -1) {
            perror("Write failed");
            close(fd1);
            return 1;
        }
    }

    // Close FIFO1 after sending data
    close(fd1);

    // Open FIFO2 to receive results
    fd2 = open(FIFO2, O_RDONLY);

    printf("\nResult received from Process 2:\n\n");

    // Read results sent by Process 2
    while ((n = read(fd2, buffer, sizeof(buffer))) > 0) {

        // Display received results on screen
        write(STDOUT_FILENO, buffer, n);
    }

    // Close FIFO2
    close(fd2);

    // Delete both named pipes
    unlink(FIFO1);
    unlink(FIFO2);

    return 0;
}


// PROCESS 2


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFO1 "fifo1"  // Pipe to receive data from Process 1
#define FIFO2 "fifo2"  // Pipe to send results to Process 1

int main() {

    char buffer[1024];  // Stores received data
    int fd1, fd2;       // File descriptors
    FILE *fp;           // File pointer
    ssize_t n;          // Number of bytes read

    long characters = 0; // Counts characters
    long words = 0;      // Counts words
    long lines = 0;      // Counts lines
    int in_word = 0;     // Checks whether inside a word

    // Create two named pipes
    mkfifo(FIFO1, 0666);
    mkfifo(FIFO2, 0666);

    // Open FIFO1 to receive data
    fd1 = open(FIFO1, O_RDONLY);

    // Read data sent by Process 1
    while ((n = read(fd1, buffer, sizeof(buffer))) > 0) {

        // Check every character
        for (ssize_t i = 0; i < n; i++) {

            unsigned char ch = (unsigned char)buffer[i];

            // Count each character
            characters++;

            // Count new line
            if (ch == '\n')
                lines++;

            // Check for spaces, tabs or newlines
            if (isspace(ch)) {

                // Word has ended
                in_word = 0;

            } else if (!in_word) {

                // New word has started
                words++;
                in_word = 1;
            }
        }
    }

    // Close FIFO1
    close(fd1);

    // Open output.txt to save results
    fp = fopen("output.txt", "w");

    // Check if file creation failed
    if (fp == NULL) {
        perror("Unable to create output file");
        return 1;
    }

    // Write calculated results into file
    fprintf(fp, "Number of characters: %ld\n", characters);
    fprintf(fp, "Number of words: %ld\n", words);
    fprintf(fp, "Number of lines: %ld\n", lines);

    // Close file after writing
    fclose(fp);

    // Open output.txt again for reading
    fp = fopen("output.txt", "r");

    // Check if file opening failed
    if (fp == NULL) {
        perror("Unable to open output file");
        return 1;
    }

    // Open FIFO2 to send results to Process 1
    fd2 = open(FIFO2, O_WRONLY);

    // Read results from output.txt
    while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) {

        // Send results through FIFO2
        if (write(fd2, buffer, n) == -1) {
            perror("Write failed");
            fclose(fp);
            close(fd2);
            return 1;
        }
    }

    // Close file and FIFO2
    fclose(fp);
    close(fd2);

    // Display completion message
    printf("Analysis completed. Results saved in output.txt\n");

    return 0;
}


//Terminal 1
gcc process1.c -o process1
./process1

//Terminal 2
gcc process2.c -o process2
./process2

# STEP 1: Create a folder
mkdir fifo_assignment
cd fifo_assignment

# STEP 2: Create Process 1
nano process1.c

# Paste PROCESS 1 code, then save:
# Ctrl + O -> Enter -> Ctrl + X

# STEP 3: Create Process 2
nano process2.c

# Paste PROCESS 2 code, then save:
# Ctrl + O -> Enter -> Ctrl + X

# STEP 4: Compile both programs
gcc process1.c -o process1
gcc process2.c -o process2

# STEP 5: Open Terminal 1 and run Process 1
./process1

# Output:
# Enter sentences (type END to finish):

# STEP 6: Open Terminal 2
cd fifo_assignment

# Run Process 2
./process2

# STEP 7: Enter the following sentences in Terminal 1
Hello world
Operating system practical
This is FIFO communication
END

# STEP 8: Expected output in Terminal 1
# Result received from Process 2:
# Number of characters: 57
# Number of words: 8
# Number of lines: 3

# STEP 9: Expected output in Terminal 2
# Analysis completed. Results saved in output.txt

# STEP 10: Verify the result file
cat output.txt

# Expected output:
# Number of characters: 57
# Number of words: 8
# Number of lines: 3    
