/*
 * This program implements a file system utility similar
 * to du. Various flags are supported:
 *     -h   human readable
 *     -s   sort descending
 *     -n   show number of files in directory
 *     -b   apparent size in bytes
 *     -a   all files, not just directories
 *     -v   verbose, csv output
 * Takes a directory as an argument. If no directory is provided,
 * the current directory is used.
 *
 * @author: Mark Jannenga
 */

#include <iostream>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <cstring>
#include <algorithm>

using namespace std;

/*
 * Private class for keeping track of information about
 * each file accessed.
 */
class FileNode {
public:
    struct stat statBuf;
    vector<FileNode *> children;
    long size;
    long blocks;
    long num_files;
    string name;
};

//Custom comparator for sorting by size.
struct comp {
    bool operator()( const FileNode * one, const FileNode * two) {
        return one->size > two->size;
    }
};

//Primary data structure
vector<FileNode *> files;

//Constants
const int BLOCK_SIZE = 512;
const int KB = 1024;
const int MB = KB * 1024;
const int GB = MB * 1024;

//Output flags
bool human_readable = false;
bool sort_by_size = false;
bool num_files = false;
bool verbose = false;
bool bytes = false;
bool all = false;

//Function prototypes
string parse_input ( int argc, char * argv[] );
FileNode *read_file ( string path, string filename );
void print_files ();
void print_verbose ();
void free_all();

int main ( int argc, char *argv[] ) {
    string path;

    //Parse the arguments
    path = parse_input(argc, argv);

    //Open the target directory, use current directory if none provided
    if (path.compare("") == 0)
        path = ".";
    read_file(path, "");

    //Sort in descending order if -s flag passed
    if (sort_by_size)
        sort(files.begin(), files.end(), comp());

    //Print verbose output if -v flag passed
    if (verbose)
        print_verbose();

    //Otherwise print output normally
    else
        print_files ();

    //Release all allocated heap space
    free_all ();

    return 0;
}

/*
 * Reads the arguments passed from the command line. Sets appropriate flags
 * and returns the path of the passed directory. Interprets any parameters
 * not preceded by - as the target directory
 */
string parse_input ( int argc, char * argv[] ) {
    string result = "";

    for ( int i = 1; i < argc; ++i ) {
        /*Check for flags*/
        if ( strcmp( argv[i], "-h" ) == 0 ) {
            human_readable = true;
        }
        else if ( strcmp( argv[i], "-s" ) == 0 ) {
            sort_by_size = true;
        }
        else if ( strcmp( argv[i], "-n" ) == 0 ) {
            num_files = true;
        }
        else if ( strcmp( argv[i], "-v" ) == 0 ) {
            verbose = true;
        }
        else if ( strcmp( argv[i], "-b" ) == 0 ) {
            bytes = true;
        }
        else if ( strcmp( argv[i], "-a" ) == 0 ) {
            all = true;
        }
        else if ( argv[i][0] != '-' ) {
            result = string( argv[i] );
        }
    }

    return result;
}

/*
 * Calls lstat() for the passed filename, recursively steps
 * through directories, and creates a FileNode for each file.
 * Adds each FileNode to the files vector. Returns pointer to
 * the FileNode created for the passed file.
 */
FileNode *read_file ( string path, string filename ) {
    struct stat _statBuf;
    DIR *dirPtr;
    struct dirent *entryPtr;
    FileNode *file = new FileNode();
    FileNode *child;
    string _path(path);

    //If filename is not blank, append to the path
    if ( filename.compare ( "" ) != 0 ) {
        _path += std::string("/");
        _path += filename;
    }

    //Call lstat() instead of stat() to handle symbolic links
    if ( lstat ( _path.c_str(), &_statBuf ) < 0 ) {
        cerr << "Error: Failed to read file " << _path << endl; ;
        exit(1);
    }

    //Initialise FileNode object
    file->statBuf = _statBuf;
    file->name = _path;
    file->size = (long) _statBuf.st_size;
    file->blocks = (long) _statBuf.st_blocks;
    file->num_files = 0;

    //Recursively step through directory
    if ( S_ISDIR ( _statBuf.st_mode ) ) {
        dirPtr = opendir ( _path.c_str() );
        while ( ( entryPtr = readdir ( dirPtr ) ) ) {
            if ( strcmp( entryPtr->d_name, "." ) != 0 && strcmp( entryPtr->d_name, ".." ) != 0 ) {
                child = read_file ( _path, entryPtr->d_name );
                file->children.push_back ( child );
                file->size += child->size;
                file->blocks += child->blocks;
                file->num_files += 1 + child->num_files;
            }
        }
        closedir(dirPtr);
    }

    //Add pointer to newly allocated FileNode to the files vector
    files.push_back(file);

    //Return a pointer to the newly allocated FileNode
    return file;
}

/*
 * Iterates through the vector of FileNodes and prints to standard output.
 * Formats output based on flags passed from command line;
 */
void print_files () {
    double size;
    string suffix;

    //Steps through every FileNode in the files vector
    for (auto it = files.begin(); it != files.end(); ++it) {
        //Prints only directories unless -a flag is passed
        if ( S_ISDIR ( ( ( FileNode * ) *it )->statBuf.st_mode) || all ) {
            //Formats human readable if -h flag passed
            if (human_readable) {
                size = ((FileNode *) *it)->blocks;
                size *= BLOCK_SIZE;
                if (size > GB) {
                    size = size / GB;
                    suffix = "G";
                }
                else if (size > MB) {
                    size = size / MB;
                    suffix = "M";
                }
                else {
                    size = size / KB;
                    suffix = "K";
                }
                if (size < 10)
                    printf("%.1f%s\t", size, suffix.c_str());
                else
                    printf("%.0f%s\t", size, suffix.c_str());
            }

            //Formats as actual size in bytes if -b passed
            else if (bytes) {
                cout << ((FileNode *) *it)->size << "\t";
            }

            //Otherwise outputs as number of 1KB blocks used
            else {
                cout << ((FileNode *) *it)->blocks * BLOCK_SIZE / KB << "\t";
            }

            //Prints number of files in directory if -n is passed
            if (num_files) {
                if (S_ISDIR(((FileNode *) *it)->statBuf.st_mode)) {
                    cout << ((FileNode *) *it)->num_files << "\t";
                }
                else {
                    cout << "\t";
                }
            }

            //Finally outputs the name of the file
            cout << ((FileNode *) *it)->name << endl;
        }
    }
}

/*
 * Prints comma separated detailed information about each file.
 * Can be saved as .csv for direct import into Microsoft Excel
 * or other similar programs
 */
void print_verbose() {
    //Print header
    cout << "Name, Size (Bytes), Blocks (512B), File Type, Num Children, Last Access, "
                    "Last Modification, Last Status Change" << endl;

    //Iterate through files vector
    for (auto it = files.begin(); it != files.end(); ++it) {
        //Output name, size in bytes, number of 512 byte blocks
        cout << ((FileNode *) *it)->name << ", ";
        cout << ((FileNode *) *it)->size << ", ";
        cout << ((FileNode *) *it)->blocks << ", ";

        //Output the type of file
        if ( S_ISREG ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "regular, ";
        else if ( S_ISREG ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "regular, ";
        else if ( S_ISDIR ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "directory, ";
        else if ( S_ISCHR ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "character device, ";
        else if ( S_ISBLK ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "block device, ";
        else if ( S_ISFIFO ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "FIFO, (named pipe) ";
        else if ( S_ISLNK ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "symbolic link, ";
        else if ( S_ISSOCK ( ( (FileNode *) *it)->statBuf.st_mode ) )
            cout << "socket, ";
        else
            cout << " ,";

        //Output the number of files in a directory
        cout << ((FileNode *) *it)->num_files << ", ";

        //Output last access time
        cout << ((FileNode *) *it)->statBuf.st_atim.tv_sec << ", ";

        //Output last modification time
        cout << ((FileNode *) *it)->statBuf.st_mtim.tv_sec << ", ";

        //Output last status change time
        cout << ((FileNode *) *it)->statBuf.st_ctim.tv_sec << endl;
    }
}

/*
 * Frees all space allocated for FileNode objects
 */
void free_all() {
    for (auto it = files.begin(); it != files.end(); ++it) {
        delete(*it);
    }
}