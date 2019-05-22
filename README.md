# File System Utility
## CIS 452 - W16 - Project 3

### Overview
The purpose of this program was to gather information about various files in the Unix file system in a similar manner to the du system utility. Information such as the size of a file in bytes, it’s disk allocation in blocks, the file’s type, any files stored within it if it was a directory, the last access time of the file, etc. was gathered. This project was implemented in C++ by:
- Parsing input from the command line
- Defining a custom FileNode class to store information about each file
- Recursively traversing the file system starting from the specified directory and creating a new FileNode for each file.
- Applying any sorting or formatting options specified via command line arguments and printing to standard output

### Usage
The syntax of the call to the program was as follows:
    ./a.out –[flags] directory
Where each flag must be individually preceded by a ‘-‘ and directory is the desired starting directory. If a directory argument is not supplied, the directory in which the executable is located is used. Supported flags are:

    -h    Sets the output to human readable (Kilobytes, Megabytes, Gigabytes)

    -s    Sorts the output in decreasing order by size

    -n    Displays the number of files and subdirectories in a directory, including any files and subdirectories within those subdirectories.

    -b    Displays the apparent size of each file in bytes

    -a    Displays all files, not just directories

    -v    Verbose output formatted as comma delimited fields for easy import into Microsoft Excel or other spreadsheet programs.


### Parsing Input
A function called parse_input ( int, char **) was created for use in parsing the input from the command line. It takes as arguments the argc and argv[] parameters passed to main when the executable is run from the command line. It cycles through each argument. If a flag is encountered, the corresponding global Boolean flag is set to true. If an argument that does not start with ‘-‘ is encountered, it is assumed to be the target directory. The function returns the target directory. If no target directory is specified, the empty string is returned.

### Defining the FileNode
Rather than rely entirely on the struct returned by a call to stat() or lstat(), a custom class was defined to keep track of relevant information about each file. The first variable of this class, statBuf, is simply the stat struct for the file. The second variable, children, is a vector of pointers to the FileNode objects corresponding to all files and subdirectories contained within a directory. The third variable, size, is the total size apparent of a file in bytes. For directories, this includes the size of all files and subdirectories. The fourth variable, blocks, is the total disk usage of a file in 512 byte blocks.  Again, for directories, this includes the size of all files and subdirectories. The fifth variable, num_files, is the total number of files contained within a directory, including subdirectories and any files in them. The final variable, name, is simply the relative path of the file starting from the initial directory.

### Traversing the File System
Once the input has been parsed, the main function calls the function read_file (string, string) in order to begin reading through the file system. Read_file takes as arguments the path to the current directory, and the filename of the file in that directory to be accessed, and returns a pointer to a FileNode object. If no target directory was specified via command lines arguments, “.” Is passed as the path. The main function passes the empty string as the argument for the filename. Read_file interprets this by reading the file explicitely defined by the path, otherwise Read_file reads the file specified by path + “/” + filename. Path and filename are kept as separate parameters to quickly ensure that the “.” And “..” entries of each directory are not read.
Once supplied with arguments, read_file calls lstat() with the specified file as an argument. Lstat() ensures that no symbolic links are followed, but beyond that, no special considerations are made for symbolic links. Read_file then instanties a FileNode object with information from the stat struct. If the file is a directory, the function opens it using opendir() and reads each file using readdir(). Read_file() is then called recursively for each file within the directory. The pointer that is returned is added to the children vector, and the relevant fields are incremented by the appropriate amounts (size, blocks, etc.). The directory is then closed, and a pointer to the newly created FileNode is added to a globally maintained vector called files. Finally, the function returns a pointer to the newly created FileNode.

### Formatting and Output
Once the filesystem has been fully read in and the vector of pointers to all the FileNodes constructed, the vector is sorted using a custom comparator, comp, if the “-s” flag was specified. Then one of two functions are called. If the “-v” falg was not specified, print_files() is called. The function takes no arguments. It iterates through the files vector and prints each file to standard output. The format out the printing depends on what flags (“-a”, “-b”, “-h”, and/or “-n”) are specified. With no flags, it matches the output of du. With “-a”, it matches the output of du -a. With “-b”, it matches the output of du –b, and with “-h”, it matches the output of du –h. The “-n” flag inserts a column between the size of the file and the filename that gives the number of files and subdirectories within a directory, but is otherwise blank.
If, however, the “-v” flag is specified, the print_verbose() function is called. This function takes no arguments, and iterates through all the files in the files vector. It first prints a comma separated header specifying which field is which. It then prints a line of comma separated values for each file. The format of the output is as follows:

“Name, Size (Bytes), Blocks (512B), File Type, Num Children, Last Access, Last Modification, Last Status Change"

If the standard output is redirected from the console to a .csv file, this output can be directly opened by Microsoft Excel or any other similar spreadsheet program.
Once the files have all been printed to standard output, the main function calls free_all(), which simply iterates through the vector of files and deallocates all the previously allocated heap space.
